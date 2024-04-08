#include "ficheros.h"

/**
 * Escribir contenido de 'buffer', de tamaño 'count', en un fichero/directorio referido por el i-nodo.
 *
 * @param inode_position Posición del i-nodo que se quiere escribir.
 * @param buffer Contenido que se quiere escribir.
 * @param file_offset Posición inicial de escritura en bytes.
 * @param count Número de bytes a escribir.
 *
 * @return Bytes escritos.
 */
int mi_write_f(unsigned int inode_position, const void *buffer, unsigned int file_offset, unsigned int count) {
    struct INode inode;

    if (leer_inodo(inode_position, &inode) == FAILURE) return FAILURE;

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
        const signed int first_physical_block = traducir_bloque_inodo(&inode, first_logical_block, RESERVE);
        if (first_physical_block == FAILURE) return FAILURE;
        if (bread(first_physical_block, &block) == FAILURE) return FAILURE;

        const unsigned int bytes_to_write = data_block_size == 0 ? count : first_block_remainder_size;

        memcpy(
                block + first_logical_block_offset,
                buffer,
                bytes_to_write
        );

        if (bwrite(first_physical_block, &block) == FAILURE) return FAILURE;

        wrote_bytes += bytes_to_write;
    }

    { // INTERMEDIATE BLOCKS
        for (int i = 1; i < data_block_size; ++i) {
            const signed int physical_block = traducir_bloque_inodo(&inode, first_logical_block + i, RESERVE);
            if (physical_block == FAILURE) return FAILURE;

            const void *data = buffer + first_block_remainder_size + (i - 1) * BLOCK_SIZE;

            if (bwrite(physical_block, data) == FAILURE) return FAILURE;

            wrote_bytes += BLOCK_SIZE;
        }
    }

    if (data_block_size > 0) { // LAST BLOCK
        const signed int last_physical_block = traducir_bloque_inodo(&inode, last_logical_block, RESERVE);
        if (last_physical_block == FAILURE) return FAILURE;
        if (bread(last_physical_block, block) == FAILURE) return FAILURE;

        const unsigned int bytes_to_write = last_logical_block_size + 1;

        memcpy(
                block,
                buffer + count - bytes_to_write,
                bytes_to_write
        );

        if (bwrite(last_physical_block, block) == FAILURE) return FAILURE;
        wrote_bytes += bytes_to_write;
    }

    const unsigned int total_bytes = wrote_bytes + file_offset;

    inode.dataModifiedAt = time(NULL);

    if (inode.logicalBytesSize < total_bytes) {
        inode.logicalBytesSize = total_bytes;
        inode.iNodeModifiedAt = inode.dataModifiedAt;
    }

    if (escribir_inodo(inode_position, &inode) == FAILURE) return FAILURE;

    return (int) wrote_bytes;
}

/**
 * Leer contenido de un fichero/directorio dentro de 'buffer'.
 *
 * @param inode_position Posición del i-nodo que se quiere leer.
 * @param buffer Contenedor donde se almacena la información leída.
 * @param file_offset Posición inicial de escritura en bytes.
 * @param count Cantidad a leer en bytes.
 *
 * @return Bytes leídos.
 */
int mi_read_f(unsigned int inode_position, void *buffer, unsigned int file_offset, unsigned int count) {
    struct INode inode;

    if (leer_inodo(inode_position, &inode) == FAILURE) return FAILURE;

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

        const signed int first_physical_block = traducir_bloque_inodo(&inode, first_logical_block, NO_RESERVE);
        if (first_physical_block == FAILURE) goto intermediate_blocks;

        if (bread(first_physical_block, block) == FAILURE) return FAILURE;

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
            const signed int physical_block = traducir_bloque_inodo(&inode, first_logical_block + i, NO_RESERVE);
            if (physical_block == FAILURE) continue;

            void *container = buffer + first_block_remainder_size + (i - 1) * BLOCK_SIZE;

            if (bread(physical_block, container) == FAILURE) return FAILURE;
        }
    }

    if (data_block_size > 0) { // LAST BLOCK
        const unsigned int bytes_to_read = last_logical_block_size + 1;

        const signed int last_physical_block = traducir_bloque_inodo(&inode, last_logical_block, NO_RESERVE);
        if (last_physical_block == FAILURE) goto sum_bytes;

        if (bread(last_physical_block, block) == FAILURE) return FAILURE;

        memcpy(
                buffer + count - bytes_to_read,
                block,
                bytes_to_read
        );

        sum_bytes:
        read_bytes += bytes_to_read;
    }


    inode.dataAccessedAt = time(NULL);
    if (escribir_inodo(inode_position, &inode) == FAILURE) return FAILURE;

    return (int) read_bytes;
}

/**
 * Leer metadatos de un i-nodo.
 *
 * @param inode_position Posición del i-nodo del que se quieren consultar los metadatos.
 * @param metadata Puntero al contenedor de los metadatos.
 *
 * @return 1 si ha sido posible leer correctamente los metadatos del i-nodo, -1 en cualquier otro caso.
 */
int mi_stat_f(unsigned int inode_position, struct Metadata *metadata) {
    struct INode inode;

    if (leer_inodo(inode_position, &inode) == FAILURE) return FAILURE;

    metadata->type = inode.type;
    metadata->permissions = inode.permissions;

    metadata->dataAccessedAt = inode.dataAccessedAt;
    metadata->dataModifiedAt = inode.dataModifiedAt;
    metadata->iNodeModifiedAt = inode.iNodeModifiedAt;

    metadata->totalLinks = inode.totalLinks;
    metadata->logicalBytesSize = inode.logicalBytesSize;
    metadata->totalBusyBlocks = inode.totalBusyBlocks;

    return SUCCESS;
}

/**
 * Modificar permisos de un i-nodo.
 *
 * @param inode_position Posición del i-nodo del que se quieren modificar los permisos.
 * @param permissions Nuevos permisos del i-nodo.
 *
 * @return 1 si ha sido posible modificar los permisos correctamente, -1 en cualquier otro caso.
 */
int mi_chmod_f(unsigned int inode_position, unsigned char permissions) {
    struct INode inode;

    if (leer_inodo(inode_position, &inode) == FAILURE) return FAILURE;

    inode.permissions = permissions;
    inode.iNodeModifiedAt = time(NULL);

    if (escribir_inodo(inode_position, &inode) == FAILURE) return FAILURE;

    return SUCCESS;
}