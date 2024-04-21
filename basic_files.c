#include "basic_files.h"

/**
 * Divide dos números redondeando hacia arriba.
 *
 * @param dividend Dividendo.
 * @param divisor  Divisor.
 *
 * @return División de los dos números redondeada hacia arriba.
 */
int div_ceil(int dividend, int divisor) {
    return dividend / divisor + (dividend % divisor > 0);
}

/**
 * Calcula el tamaño en bloques en función del número de bytes.
 *
 * @param bytes Bytes.

 * @return Tamaño en bloques.
 */
int block_size(int bytes) {
    return div_ceil(bytes, BLOCK_SIZE);
}

int bitmap_size(unsigned int total_blocks) {
    return block_size(div_ceil((int) total_blocks, BYTE_LENGTH));
}

int inodes_size(unsigned int total_inodes) {
    return block_size((int) (total_inodes * INODE_SIZE));
}

int init_super_block(unsigned int total_blocks, unsigned int total_inodes) {
    const struct SuperBlock sb = {
            .bitMapFirstBlock = SUPER_BLOCK_POSITION + SUPER_BLOCKS_PER_BLOCK,
            .bitMapLastBlock = sb.bitMapFirstBlock + bitmap_size(total_blocks) - 1,
            .iNodesFirstBlock = sb.bitMapLastBlock + 1,
            .iNodesLastBlock = sb.iNodesFirstBlock + inodes_size(total_inodes) - 1,
            .dataFirstBlock = sb.iNodesLastBlock + 1,
            .dataLastBlock = total_blocks - 1,
            .rootINode = 0,
            .firstFreeINode = 0,
            .totalFreeBlocks = total_blocks,
            .totalFreeINodes = total_inodes,
            .totalBlocks = total_blocks,
            .totalINodes = total_inodes
    };

    return write_block(SUPER_BLOCK_POSITION, &sb);
}

int init_bitmap() {
    struct SuperBlock sb;
    if (read_block(SUPER_BLOCK_POSITION, &sb) < 0) return FAILURE;

    const int metadata_block_size = bitmap_size(sb.totalBlocks)
                                    + inodes_size(sb.totalINodes)
                                    + SUPER_BLOCKS_PER_BLOCK;

    const int metadata_bytes = metadata_block_size / BYTE_LENGTH;
    const int metadata_extra_bits = metadata_block_size % BYTE_LENGTH;

    const int bitmap_block_size = block_size(metadata_bytes + (metadata_extra_bits > 0));
    const int bitmap_byte_size = bitmap_block_size * BLOCK_SIZE;

    unsigned char bitmap[bitmap_byte_size];

    memset(bitmap, FULL_BYTE, metadata_bytes);
    memset(bitmap + metadata_bytes + 1, EMPTY_BYTE, bitmap_byte_size - metadata_bytes);
    bitmap[metadata_bytes] = FULL_BYTE << (BYTE_LENGTH - metadata_extra_bits);

    for (int i = 0; i < bitmap_block_size; ++i) {
        if (write_block(sb.bitMapFirstBlock + i, &bitmap[i * BLOCK_SIZE]) < 0) return FAILURE;
    }

    sb.totalFreeBlocks -= metadata_block_size;

    if (write_block(SUPER_BLOCK_POSITION, &sb) < 0) return FAILURE;

    return SUCCESS;
}

int init_inodes() {
    struct SuperBlock sb;
    if (read_block(SUPER_BLOCK_POSITION, &sb) < 0) return FAILURE;

    struct INode inodes[INODES_PER_BLOCK];

    unsigned int next_inode = sb.firstFreeINode + 1;

    for (unsigned int i = sb.iNodesFirstBlock; i <= sb.iNodesLastBlock; ++i) {
        for (int j = 0; j < INODES_PER_BLOCK; ++j) {
            inodes[j].directPointers[0] = next_inode++;
            inodes[j].metadata.type = FREE_INODE;
        }

        if (i == sb.iNodesLastBlock) inodes[INODES_PER_BLOCK - 1].directPointers[0] = UINT_MAX;

        if (write_block(i, inodes) < 0) return FAILURE;
    }

    return SUCCESS;
}

int set_block_state(unsigned int physical_block, unsigned int state) {
    struct SuperBlock sb;
    if (read_block(SUPER_BLOCK_POSITION, &sb) < 0) return FAILURE;

    const unsigned int block_byte = physical_block / BYTE_LENGTH;
    const unsigned int absolute_block = sb.bitMapFirstBlock + (block_byte / BLOCK_SIZE);

    unsigned char bitmap[BLOCK_SIZE];
    if (read_block(absolute_block, bitmap) < 0) return FAILURE;

    const unsigned char mask = BIT7 >> (physical_block % BYTE_LENGTH);

    unsigned char *byte_to_modify = &bitmap[block_byte];

    *byte_to_modify = state ? *byte_to_modify | mask : *byte_to_modify & ~mask;

    if (write_block(absolute_block, &bitmap) < 0) return FAILURE;

    return SUCCESS;
}

char get_block_state(unsigned int physical_block) {
    struct SuperBlock sb;
    if (read_block(SUPER_BLOCK_POSITION, &sb) < 0) return FAILURE;

    const unsigned int block_byte = physical_block / BYTE_LENGTH;
    const unsigned int absolute_block = sb.bitMapFirstBlock + (block_byte / BLOCK_SIZE);

    unsigned char bitmap[BLOCK_SIZE];
    if (read_block(absolute_block, bitmap) < 0) return FAILURE;

    return (char) ((bitmap[block_byte % BLOCK_SIZE] & (BIT7 >> physical_block % BYTE_LENGTH)) > 0);
}

int reserve_block() {
    struct SuperBlock sb;
    if (read_block(SUPER_BLOCK_POSITION, &sb) < 0) return FAILURE;

    if (sb.totalFreeBlocks == 0) return FAILURE;

    unsigned int bitmap_block_length = sb.bitMapLastBlock - sb.bitMapFirstBlock;

    int bitmap_block = -1;

    unsigned char bitmap[BLOCK_SIZE];

    unsigned char full_block[BLOCK_SIZE];
    memset(full_block, FULL_BYTE, BLOCK_SIZE);

    const unsigned char empty_block[BLOCK_SIZE] = {0};

    do {
        if (read_block(sb.bitMapFirstBlock + ++bitmap_block, bitmap) < 0) return FAILURE;

    } while (memcmp(bitmap, full_block, BLOCK_SIZE) == 0 && bitmap_block < bitmap_block_length);

    for (unsigned int byte = 0; byte < BLOCK_SIZE; ++byte) {
        if (bitmap[byte] == FULL_BYTE) continue;

        unsigned char bit = 0;

        for (unsigned char mask = BIT7; bitmap[byte] & mask; mask >>= 1) bit++;

        const unsigned int block_position = (bitmap_block * BLOCK_SIZE + byte) * BYTE_LENGTH + bit;

        if (set_block_state(block_position, BUSY_BLOCK) < 0) return FAILURE;

        sb.totalFreeBlocks--;

        if (write_block(SUPER_BLOCK_POSITION, &sb) < 0) return FAILURE;
        if (write_block(block_position, empty_block) < 0) return FAILURE;

        return (int) block_position;
    }

    return FAILURE;
}

int free_block(unsigned int physical_block) {
    struct SuperBlock sb;
    if (read_block(SUPER_BLOCK_POSITION, &sb) < 0) return FAILURE;

    if (sb.totalBlocks == sb.totalFreeBlocks) return (int) physical_block;

    if (set_block_state(physical_block, FREE_BLOCK) < 0) return FAILURE;

    sb.totalFreeBlocks++;

    if (write_block(SUPER_BLOCK_POSITION, &sb) < 0) return FAILURE;

    return (int) physical_block;
}

int write_inode(unsigned int inode_position, struct INode *inode) {
    struct SuperBlock sb;
    if (read_block(SUPER_BLOCK_POSITION, &sb) < 0) return FAILURE;

    const unsigned int block = sb.iNodesFirstBlock + inode_position / INODES_PER_BLOCK;

    struct INode inodes[INODES_PER_BLOCK];
    if (read_block(block, inodes) < 0) return FAILURE;

    inodes[inode_position % INODES_PER_BLOCK] = *inode;

    if (write_block(block, inodes) < 0) return FAILURE;

    return SUCCESS;
}

int read_inode(unsigned int inode_position, struct INode *inode) {
    struct SuperBlock sb;
    if (read_block(SUPER_BLOCK_POSITION, &sb) < 0) return FAILURE;

    const unsigned int block = sb.iNodesFirstBlock + inode_position / INODES_PER_BLOCK;

    struct INode inodes[INODES_PER_BLOCK];
    if (read_block(block, inodes) < 0) return FAILURE;

    *inode = inodes[inode_position % INODES_PER_BLOCK];

    return SUCCESS;
}

int reserve_inode(unsigned char type, unsigned char permissions) {
    struct SuperBlock sb;
    if (read_block(SUPER_BLOCK_POSITION, &sb) < 0) return FAILURE;

    if (sb.totalFreeINodes == 0) return FAILURE;

    const unsigned int inode_position = sb.firstFreeINode;

    struct INode inode;
    if (read_inode(inode_position, &inode) < 0) return FAILURE;

    sb.firstFreeINode = inode.directPointers[0];

    inode.metadata.type = type;
    inode.metadata.permissions = permissions;
    inode.metadata.linksCount = 1;
    inode.metadata.size = 0;
    inode.metadata.dataAccessedAt = time(NULL);
    inode.metadata.dataModifiedAt = inode.metadata.dataAccessedAt;
    inode.metadata.modifiedAt = inode.metadata.dataAccessedAt;
    inode.metadata.busyBlocksCount = 0;

    memset(inode.directPointers, 0, DIRECT_POINTERS * sizeof(unsigned int));
    memset(inode.indirectPointers, 0, INDIRECT_POINTERS * sizeof(unsigned int));

    if (write_inode(inode_position, &inode) < 0) return FAILURE;

    sb.totalFreeINodes--;

    if (write_block(SUPER_BLOCK_POSITION, &sb) < 0) return FAILURE;

    return (int) inode_position;
}

const static unsigned int ranges[] = {LEVEL0_MAX, LEVEL1_MAX, LEVEL2_MAX, LEVEL3_MAX};

/**
 * Obtener el nivel donde se encuentra el bloque lógico.
 *
 * @param inode I-Nodo del qual se quiere consultar el bloque lógico.
 * @param logical_block Bloque lógico a consultar.
 * @param block_ptr Puntero al bloque. 0 significa que no apunta a nada.
 *
 * @return Nivel del bloque lógico.
 */
int get_block_level(unsigned int logical_block, struct INode *inode, unsigned int *block_ptr) {
    for (int lvl = 0; lvl < NUM_LEVELS; ++lvl) {
        if (logical_block < ranges[lvl]) {
            *block_ptr = lvl > 0 ? inode->indirectPointers[lvl - 1] : inode->directPointers[logical_block];
            return lvl;
        }
    }

    return FAILURE;
}

/**
 * Obtener potencia de un número entero.
 * Simplificación del método math.pow por la no necesidad de usar números decimales.
 *
 * @param base Número que se multiplica tantas veces como el exponente.
 * @param exp Exponente al que se eleva la base.
 *
 * @return Base elevada al exponente. Si el exponente es negativo, el resultado será 1 (por conveniencia del programa
 * 'block_pointer_position_by_level').
 */
unsigned int power(unsigned int base, int exp) {
    unsigned int result = 1;

    for (unsigned int i = 0; i < exp; ++i) {
        result *= base;
    }

    return result;
}

/**
 * Obtener índice del bloque de punteros.
 *
 * @param logical_block Bloque lógico.
 * @param level Nivel de punteros. Nivel 0 se refiere a los punteros directos.
 *
 * @return Índice del bloque lógico según el nivel.
 */
int block_pointer_position_by_level(unsigned int logical_block, int level) {
    for (int lvl = 0; lvl < NUM_LEVELS; ++lvl) {
        if (logical_block < ranges[lvl]) {
            const unsigned int relative_block = lvl > 0 ? logical_block - ranges[lvl - 1] : logical_block;

            const unsigned int total_lvl_pointers = power(POINTERS_PER_BLOCK, lvl - 1);

            if (level == lvl) return (int) (relative_block / total_lvl_pointers);

            const unsigned int extra_lvl_pointers = power(POINTERS_PER_BLOCK, lvl - 2);

            if (level == lvl - 1) return (int) ((relative_block % total_lvl_pointers) / extra_lvl_pointers);
            if (level == lvl - 2) return (int) ((relative_block % total_lvl_pointers) % POINTERS_PER_BLOCK);

            return (int) relative_block;
        }
    }

    return FAILURE;
}

/**
 * Reserva un bloque de datos para un i-nodo específico.
 *
 * @param inode I-Nodo del cual se quiere reservar un bloque de datos.
 * @param logical_block Bloque lógico al que corresponde el bloque físico que se va a reservar.
 * @param logical_block_level Nivel de punteros donde se encuentra el bloque lógico especificado.
 * @param reserve_block_level Nivel donde se quiere reservar el bloque lógico.
 * @param intermediate_pointer_block Bloque de punteros intermedio, el cual no cuelga directamente del i-nodo.
 * @param intermediate_pointer_block_position Posición física donde tiene que guardarse el puntero al bloque intermedio.
 * @param block_ptr_position Posición dentro del bloque intermedio donde tiene que guardarse el puntero del bloque reservado.
 *
 * @return Puntero del bloque físico reservado.
 */
int reserve_inode_block(struct INode *inode, unsigned int logical_block, unsigned int logical_block_level,
                        unsigned int reserve_block_level, unsigned int *intermediate_pointer_block,
                        unsigned int intermediate_pointer_block_position, unsigned int block_ptr_position) {

    const int block_ptr = reserve_block();
    if (block_ptr < 0) return FAILURE;

    inode->metadata.busyBlocksCount++;
    inode->metadata.modifiedAt = time(NULL);

    if (logical_block_level == 0) {
        debug("directPointers[%d] = %d", logical_block, block_ptr);
        inode->directPointers[logical_block] = block_ptr;
        return block_ptr;
    }

    if (logical_block_level == reserve_block_level) {
        debug("indirectPointers[%d] = %d", reserve_block_level - 1, block_ptr);
        inode->indirectPointers[reserve_block_level - 1] = block_ptr;
        return block_ptr;
    }

    debug("nivel%d[%d] = %d", reserve_block_level + 1, block_ptr_position, block_ptr);
    intermediate_pointer_block[block_ptr_position] = block_ptr;

    if (write_block(intermediate_pointer_block_position, intermediate_pointer_block) < 0) return FAILURE;

    return block_ptr;
}

int get_physical_block(struct INode *inode, unsigned int logical_block, unsigned char reserve) {
    unsigned int prev_block_ptr;
    unsigned int block_ptr;
    int block_ptr_position;

    const signed int logical_block_level = get_block_level(logical_block, inode, &block_ptr);
    if (logical_block_level < 0) return FAILURE;

    unsigned int pointer_block[POINTERS_PER_BLOCK];

    for (int current_level = logical_block_level; current_level > 0; --current_level) {
        if (block_ptr) {
            if (read_block(block_ptr, pointer_block) < 0) return FAILURE;
        } else {
            if (!reserve) return FAILURE;

            const signed reserved_block_ptr = reserve_inode_block(
                    inode,
                    logical_block,
                    logical_block_level,
                    current_level,
                    pointer_block,
                    prev_block_ptr,
                    block_ptr_position
            );

            if (reserved_block_ptr < 0) return FAILURE;

            memset(pointer_block, EMPTY_BYTE, BLOCK_SIZE);

            block_ptr = reserved_block_ptr;
        }

        block_ptr_position = block_pointer_position_by_level(logical_block, current_level);
        if (block_ptr_position < 0) return FAILURE;

        prev_block_ptr = block_ptr;
        block_ptr = pointer_block[block_ptr_position];
    }

    if (block_ptr) return (int) block_ptr;
    if (!reserve) return FAILURE;

    return reserve_inode_block(
            inode,
            logical_block,
            logical_block_level,
            0,
            pointer_block,
            prev_block_ptr,
            block_ptr_position
    );
}

/**
 * Libera recursivamente todos los bloques de una rama de punteros. Tanto los bloques de punteros como de datos.
 *
 * @param physical_block Bloque físico del cual empieza la liberación.
 * @param from_logical_block Bloque lógico desde donde se tienen que liberar los bloques físicos.
 * @param level Nivel del bloque de punteros a liberar.
 *
 * @return Número de bloques liberados.
 */
int free_inode_tree(unsigned int *physical_block, unsigned int from_logical_block, int level) {
    if (level == 0) {
        const int freed = free_block(*physical_block) > 0;
        debug("Se ha liberado el bloque de datos %d", *physical_block);
        *physical_block = 0;
        return freed;
    }

    unsigned int pointer_block[POINTERS_PER_BLOCK];
    read_block(*physical_block, pointer_block);

    int freed_blocks = 0;

    const int position = from_logical_block ? block_pointer_position_by_level(from_logical_block, level) : 0;

    unsigned char modified = 0;

    for (int i = position; i < POINTERS_PER_BLOCK; ++i) {
        if (pointer_block[i]) {
            freed_blocks += free_inode_tree(
                    pointer_block + i,
                    i == position ? from_logical_block : 0,
                    level - 1
            );

            if (!modified) modified = !*(pointer_block + i);
        }
    }

    const static unsigned char empty_block[BLOCK_SIZE] = {0};

    if (memcmp(pointer_block, empty_block, BLOCK_SIZE) == 0) {
        freed_blocks += free_block(*physical_block) > 0;
        debug("Se ha liberado el bloque de punteros %d", *physical_block);
        *physical_block = 0;
    } else if (modified) {
        write_block(*physical_block, pointer_block);
        debug("Se ha escrito el bloque de punteros %d", *physical_block);
    }

    return freed_blocks;
}

/**
 * Obtener la posición relativa al array de punteros directos del puntero del primer bloque a consultar para el bloque
 * lógico.
 *
 * @param logical_block Bloque lógico del cual se quiere obtener el primer puntero.
 *
 * @return Posición relativa al array de punteros directos.
 *         0-11 = DIRECTOS
 *           12 = INDIRECTOS 1
 *           13 = INDIRECTOS 2
 *           14 = INDIRECTOS 3
 */
int get_block_pointer_position(int logical_block) {
    for (int lvl = 0; lvl < NUM_LEVELS; ++lvl) {
        if (logical_block < ranges[lvl]) return lvl > 0 ? DIRECT_POINTERS + lvl - 1 : logical_block;
    }

    return FAILURE;
}

int free_inode_blocks(unsigned int first_logical_block, struct INode *inode) {
    const int position = get_block_pointer_position((int) first_logical_block);
    unsigned int freed_blocks = 0;

    for (int i = position; i < POINTERS; ++i) {
        unsigned int *block_ptr = inode->directPointers + i;

        if (*block_ptr) {
            freed_blocks += free_inode_tree(
                    block_ptr,
                    i == position ? first_logical_block : 0,
                    i < DIRECT_POINTERS ? 0 : i - DIRECT_POINTERS + 1
            );
        }
    }
    debug("Se han liberado %d bloques", freed_blocks);
    return (int) freed_blocks;
}

int free_inode(unsigned int inode_position) {
    struct INode inode;
    if (read_inode(inode_position, &inode) < 0) return FAILURE;

    const signed int freed_blocks = free_inode_blocks(0, &inode);
    if (freed_blocks < 0) return FAILURE;

    inode.metadata.modifiedAt = time(NULL);
    inode.metadata.busyBlocksCount -= freed_blocks;
    inode.metadata.size = 0;
    inode.metadata.type = FREE_INODE;

    struct SuperBlock sb;
    if (read_block(SUPER_BLOCK_POSITION, &sb) < 0) return FAILURE;

    inode.directPointers[0] = sb.firstFreeINode;
    sb.firstFreeINode = inode_position;
    sb.totalFreeINodes++;

    if (write_block(SUPER_BLOCK_POSITION, &sb) < 0) return FAILURE;
    if (write_inode(inode_position, &inode) < 0) return FAILURE;

    return (int) inode_position;
}

int print_inode(struct Metadata *metadata, char *name) {
    printf("\n *** %s ***\n", name);

    printf(
            "Tipo: %c\n"
            "Permisos: %d\n"
            "-------------------------------------\n"
            "Fecha del último acceso a los datos: %s"
            "Fecha de la última modificación de los datos: %s"
            "Fecha de la última modificación del i-nodo: %s"
            "-------------------------------------\n"
            "Número de enlaces: %d\n"
            "Tamaño en bytes lógicos: %d\n"
            "Número de bloques ocupados: %d\n",
            metadata->type,
            metadata->permissions,
            ctime(&metadata->dataAccessedAt),
            ctime(&metadata->dataModifiedAt),
            ctime(&metadata->modifiedAt),
            metadata->linksCount,
            metadata->size,
            metadata->busyBlocksCount
    );

    return SUCCESS;
}