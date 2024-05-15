#include "files.h"

int my_write_file(unsigned int inode_position, const void *buffer, unsigned int offset, unsigned int count) {
    inode_t inode;
    if (read_inode(inode_position, &inode) < 0) return FAILURE;

    if ((inode.metadata.permissions & WRITE) == 0) return NOT_WRITE_PERMISSIONS;

    unsigned int wrote_bytes = 0;

    const unsigned int first_logical_block = offset / BLOCK_SIZE;
    const unsigned int first_logical_block_offset = offset % BLOCK_SIZE;
    const unsigned int first_block_remainder_size = BLOCK_SIZE - first_logical_block_offset;

    const unsigned int last_logical_block = (offset + count - 1) / BLOCK_SIZE;
    const unsigned int last_logical_block_size = (offset + count - 1) % BLOCK_SIZE;

    const unsigned int data_block_size = last_logical_block - first_logical_block;

    unsigned char block[BLOCK_SIZE];

    {
        const int first_physical_block = get_physical_block(&inode, first_logical_block, RESERVE);
        if (first_physical_block < 0) return FAILURE;
        if (read_block(first_physical_block, block) < 0) return FAILURE;

        const unsigned int bytes_to_write = data_block_size == 0 ? count : first_block_remainder_size;

        memcpy(
                block + first_logical_block_offset,
                buffer,
                bytes_to_write
        );

        if (write_block(first_physical_block, block) < 0) return FAILURE;

        wrote_bytes += bytes_to_write;
    }

    {
        for (int i = 1; i < data_block_size; ++i) {
            const int physical_block = get_physical_block(&inode, first_logical_block + i, RESERVE);
            if (physical_block < 0) return FAILURE;

            const void *data = buffer + first_block_remainder_size + (i - 1) * BLOCK_SIZE;
            if (write_block(physical_block, data) < 0) return FAILURE;

            wrote_bytes += BLOCK_SIZE;
        }
    }

    if (data_block_size > 0) {
        const int last_physical_block = get_physical_block(&inode, last_logical_block, RESERVE);
        if (last_physical_block < 0) return FAILURE;
        if (read_block(last_physical_block, block) < 0) return FAILURE;

        const unsigned int bytes_to_write = last_logical_block_size + 1;

        memcpy(
                block,
                buffer + count - bytes_to_write,
                bytes_to_write
        );

        if (write_block(last_physical_block, block) < 0) return FAILURE;

        wrote_bytes += bytes_to_write;
    }

    const unsigned int total_bytes = wrote_bytes + offset;

    inode.metadata.data_modified_at = time(NULL);

    if (inode.metadata.size < total_bytes) {
        inode.metadata.size = total_bytes;
        inode.metadata.modified_at = inode.metadata.data_modified_at;
    }

    if (write_inode(inode_position, &inode) < 0) return FAILURE;

    return (int) wrote_bytes;
}

int my_read_file(unsigned int inode_position, void *buffer, unsigned int offset, unsigned int count) {
    inode_t inode;
    if (read_inode(inode_position, &inode) < 0) return FAILURE;

    if ((inode.metadata.permissions & READ) == 0) return NOT_READ_PERMISSIONS;

    if (offset > inode.metadata.size) return 0;
    if (offset + count > inode.metadata.size) count = inode.metadata.size - offset;

    unsigned int read_bytes = 0;

    const unsigned int first_logical_block = offset / BLOCK_SIZE;
    const unsigned int first_logical_block_offset = offset % BLOCK_SIZE;
    const unsigned int first_block_remainder_size = BLOCK_SIZE - first_logical_block_offset;

    const unsigned int last_logical_block = (offset + count - 1) / BLOCK_SIZE;
    const unsigned int last_logical_block_size = (offset + count - 1) % BLOCK_SIZE;

    const unsigned int data_block_size = last_logical_block - first_logical_block;

    unsigned char block[BLOCK_SIZE];

    {
        const unsigned int bytes_to_read = data_block_size == 0 ? count : first_block_remainder_size;

        const int first_physical_block = get_physical_block(&inode, first_logical_block, NO_RESERVE);

        if (first_physical_block >= 0) {
            if (read_block(first_physical_block, block) < 0) return FAILURE;

            memcpy(
                    buffer,
                    block + first_logical_block_offset,
                    bytes_to_read
            );
        }

        read_bytes += bytes_to_read;
    }

    {
        for (int i = 1; i < data_block_size; ++i, read_bytes += BLOCK_SIZE) {
            const int physical_block = get_physical_block(&inode, first_logical_block + i, NO_RESERVE);
            if (physical_block < 0) continue;

            void *container = buffer + first_block_remainder_size + (i - 1) * BLOCK_SIZE;
            if (read_block(physical_block, container) < 0) return FAILURE;
        }
    }

    if (data_block_size > 0) {
        const unsigned int bytes_to_read = last_logical_block_size + 1;

        const int last_physical_block = get_physical_block(&inode, last_logical_block, NO_RESERVE);

        if (last_physical_block >= 0) {
            if (read_block(last_physical_block, block) < 0) return FAILURE;

            memcpy(
                    buffer + count - bytes_to_read,
                    block,
                    bytes_to_read
            );
        }

        read_bytes += bytes_to_read;
    }

    inode.metadata.data_accessed_at = time(NULL);

    return write_inode(inode_position, &inode) < 0 ? FAILURE : (int) read_bytes;
}

int my_stat_file(unsigned int inode_position, metadata_t *metadata) {
    inode_t inode;
    if (read_inode(inode_position, &inode) < 0) return FAILURE;

    *metadata = inode.metadata;

    return SUCCESS;
}

int my_chmod_file(unsigned int inode_position, unsigned char permissions) {
    inode_t inode;
    if (read_inode(inode_position, &inode) < 0) return FAILURE;

    inode.metadata.permissions = permissions;
    inode.metadata.modified_at = time(NULL);

    if (write_inode(inode_position, &inode) < 0) return FAILURE;

    return SUCCESS;
}

int my_trunc_file(unsigned int inode_position, unsigned int count) {
    inode_t inode;
    if (read_inode(inode_position, &inode) < 0) return FAILURE;

    if ((inode.metadata.permissions & WRITE) == 0) return NOT_WRITE_PERMISSIONS;
    if (inode.metadata.size < count) return FAILURE;

    const unsigned int first_logical_block = count / BLOCK_SIZE + (count % BLOCK_SIZE > 0);

    const int freed_blocks = free_inode_blocks(first_logical_block, &inode);
    if (freed_blocks < 0) return FAILURE;

    inode.metadata.modified_at = time(NULL);
    inode.metadata.data_modified_at = inode.metadata.modified_at;
    inode.metadata.size = count;
    inode.metadata.busy_blocks_count -= freed_blocks;

    if (write_inode(inode_position, &inode) < 0) return FAILURE;

    return freed_blocks;
}