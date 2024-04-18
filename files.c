#include "files.h"

int my_write(unsigned int inode_position, const void *buffer, unsigned int file_offset, unsigned int count) {
    struct INode inode;

    if (read_inode(inode_position, &inode) == FAILURE) return FAILURE;

    if ((inode.permissions & WRITE) == 0) return FAILURE;

    unsigned int wrote_bytes = 0;

    const unsigned int first_logical_block = file_offset / BLOCK_SIZE;
    const unsigned int first_logical_block_offset = file_offset % BLOCK_SIZE;
    const unsigned int first_block_remainder_size = BLOCK_SIZE - first_logical_block_offset;

    const unsigned int last_logical_block = (file_offset + count - 1) / BLOCK_SIZE;
    const unsigned int last_logical_block_size = (file_offset + count - 1) % BLOCK_SIZE;

    const unsigned int data_block_size = last_logical_block - first_logical_block;

    unsigned char block[BLOCK_SIZE];

    { // FIRST BLOCK
        const signed int first_physical_block = get_physical_by_logical_block(&inode, first_logical_block, RESERVE);
        if (first_physical_block == FAILURE) return FAILURE;
        if (read_block(first_physical_block, &block) == FAILURE) return FAILURE;

        const unsigned int bytes_to_write = data_block_size == 0 ? count : first_block_remainder_size;

        memcpy(
                block + first_logical_block_offset,
                buffer,
                bytes_to_write
        );

        if (write_block(first_physical_block, &block) == FAILURE) return FAILURE;

        wrote_bytes += bytes_to_write;
    }

    { // INTERMEDIATE BLOCKS
        for (int i = 1; i < data_block_size; ++i) {
            const signed int physical_block = get_physical_by_logical_block(&inode, first_logical_block + i, RESERVE);
            if (physical_block == FAILURE) return FAILURE;

            const void *data = buffer + first_block_remainder_size + (i - 1) * BLOCK_SIZE;

            if (write_block(physical_block, data) == FAILURE) return FAILURE;

            wrote_bytes += BLOCK_SIZE;
        }
    }

    if (data_block_size > 0) { // LAST BLOCK
        const signed int last_physical_block = get_physical_by_logical_block(&inode, last_logical_block, RESERVE);
        if (last_physical_block == FAILURE) return FAILURE;
        if (read_block(last_physical_block, block) == FAILURE) return FAILURE;

        const unsigned int bytes_to_write = last_logical_block_size + 1;

        memcpy(
                block,
                buffer + count - bytes_to_write,
                bytes_to_write
        );

        if (write_block(last_physical_block, block) == FAILURE) return FAILURE;
        wrote_bytes += bytes_to_write;
    }

    const unsigned int total_bytes = wrote_bytes + file_offset;

    inode.dataModifiedAt = time(NULL);

    if (inode.logicalBytesSize < total_bytes) {
        inode.logicalBytesSize = total_bytes;
        inode.modifiedAt = inode.dataModifiedAt;
    }

    if (write_inode(inode_position, &inode) == FAILURE) return FAILURE;

    return (int) wrote_bytes;
}

int my_read(unsigned int inode_position, void *buffer, unsigned int file_offset, unsigned int count) {
    struct INode inode;

    if (read_inode(inode_position, &inode) == FAILURE) return FAILURE;

    if ((inode.permissions & READ) == 0) return FAILURE;

    if (file_offset > inode.logicalBytesSize) return 0;
    if (file_offset + count > inode.logicalBytesSize) count = inode.logicalBytesSize - file_offset;

    unsigned int read_bytes = 0;

    const unsigned int first_logical_block = file_offset / BLOCK_SIZE;
    const unsigned int first_logical_block_offset = file_offset % BLOCK_SIZE;
    const unsigned int first_block_remainder_size = BLOCK_SIZE - first_logical_block_offset;

    const unsigned int last_logical_block = (file_offset + count - 1) / BLOCK_SIZE;
    const unsigned int last_logical_block_size = (file_offset + count - 1) % BLOCK_SIZE;

    const unsigned int data_block_size = last_logical_block - first_logical_block;

    unsigned char block[BLOCK_SIZE];

    { // FIRST BLOCK
        const unsigned int bytes_to_read = data_block_size == 0 ? count : first_block_remainder_size;

        const signed int first_physical_block = get_physical_by_logical_block(&inode, first_logical_block, NO_RESERVE);
        if (first_physical_block == FAILURE) goto intermediate_blocks;

        if (read_block(first_physical_block, block) == FAILURE) return FAILURE;

        memcpy(
                buffer,
                block + first_logical_block_offset,
                bytes_to_read
        );

        intermediate_blocks:
        read_bytes += bytes_to_read;
    }

    { // INTERMEDIATE BLOCKS
        for (int i = 1; i < data_block_size; ++i, read_bytes += BLOCK_SIZE) {
            const signed int physical_block = get_physical_by_logical_block(&inode, first_logical_block + i, NO_RESERVE);
            if (physical_block == FAILURE) continue;

            void *container = buffer + first_block_remainder_size + (i - 1) * BLOCK_SIZE;

            if (read_block(physical_block, container) == FAILURE) return FAILURE;
        }
    }

    if (data_block_size > 0) { // LAST BLOCK
        const unsigned int bytes_to_read = last_logical_block_size + 1;

        const signed int last_physical_block = get_physical_by_logical_block(&inode, last_logical_block, NO_RESERVE);
        if (last_physical_block == FAILURE) goto sum_bytes;

        if (read_block(last_physical_block, block) == FAILURE) return FAILURE;

        memcpy(
                buffer + count - bytes_to_read,
                block,
                bytes_to_read
        );

        sum_bytes:
        read_bytes += bytes_to_read;
    }


    inode.dataAccessedAt = time(NULL);
    if (write_inode(inode_position, &inode) == FAILURE) return FAILURE;

    return (int) read_bytes;
}

int my_stat(unsigned int inode_position, struct Metadata *metadata) {
    struct INode inode;

    if (read_inode(inode_position, &inode) == FAILURE) return FAILURE;

    metadata->type = inode.type;
    metadata->permissions = inode.permissions;

    metadata->dataAccessedAt = inode.dataAccessedAt;
    metadata->dataModifiedAt = inode.dataModifiedAt;
    metadata->modifiedAt = inode.modifiedAt;

    metadata->totalLinks = inode.totalLinks;
    metadata->logicalBytesSize = inode.logicalBytesSize;
    metadata->totalBusyBlocks = inode.totalBusyBlocks;

    return SUCCESS;
}

int my_chmod(unsigned int inode_position, unsigned char permissions) {
    struct INode inode;

    if (read_inode(inode_position, &inode) == FAILURE) return FAILURE;

    inode.permissions = permissions;
    inode.modifiedAt = time(NULL);

    if (write_inode(inode_position, &inode) == FAILURE) return FAILURE;

    return SUCCESS;
}

int my_trunc(unsigned int inode_position, unsigned int count) {
    struct INode inode;

    if (read_inode(inode_position, &inode) == FAILURE) return FAILURE;

    if ((inode.permissions & WRITE) == 0) return FAILURE;
    if (inode.logicalBytesSize < count) return FAILURE;

    const unsigned int first_logical_block = count / BLOCK_SIZE + (count % BLOCK_SIZE > 0);

    const signed int freed_blocks = free_inode_blocks(first_logical_block, &inode);
    if (freed_blocks == FAILURE) return FAILURE;

    inode.modifiedAt = time(NULL);
    inode.dataModifiedAt = inode.modifiedAt;
    inode.logicalBytesSize = count;
    inode.totalBusyBlocks -= freed_blocks;

    if (write_inode(inode_position, &inode) == FAILURE) return FAILURE;

    return freed_blocks;
}