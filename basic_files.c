#include "basic_files.h"

/**
 * Divide dos números redondeando hacia arriba.
 *
 * @param dividend Dividendo.
 * @param divisor  Divisor.
 *
 * @return División de los dos números redondeada hacia arriba.
 */
unsigned int div_ceil(unsigned int dividend, unsigned int divisor) {
    return dividend / divisor + (dividend % divisor > 0);
}

unsigned int block_size(unsigned int bytes) {
    return div_ceil(bytes, BLOCK_SIZE);
}

unsigned int bitmap_size(unsigned int total_blocks) {
    return block_size(div_ceil((int) total_blocks, BYTE_LENGTH));
}

unsigned int inodes_size(unsigned int total_inodes) {
    return block_size((int) (total_inodes * INODE_SIZE));
}

int init_super_block(unsigned int total_blocks, unsigned int total_inodes) {
    const super_block_t sb = {
            .bitmap_first_block = SUPER_BLOCK_POSITION + SUPER_BLOCKS_PER_BLOCK,
            .bitmap_last_block = sb.bitmap_first_block + bitmap_size(total_blocks) - 1,
            .inodes_first_block = sb.bitmap_last_block + 1,
            .inodes_last_block = sb.inodes_first_block + inodes_size(total_inodes) - 1,
            .data_first_block = sb.inodes_last_block + 1,
            .data_last_block = total_blocks - 1,
            .root_inode = 0,
            .next_free_inode_position = 0,
            .free_blocks_count = total_blocks,
            .free_inodes_count = total_inodes,
            .blocks_count = total_blocks,
            .inodes_count = total_inodes
    };

    return write_block(SUPER_BLOCK_POSITION, &sb);
}

int init_bitmap() {
    super_block_t sb;
    if (read_block(SUPER_BLOCK_POSITION, &sb) < 0) return FAILURE;

    const unsigned int metadata_block_size = bitmap_size(sb.blocks_count)
                                             + inodes_size(sb.inodes_count)
                                             + SUPER_BLOCKS_PER_BLOCK;

    const unsigned int metadata_bytes = metadata_block_size / BYTE_LENGTH;
    const unsigned int metadata_extra_bits = metadata_block_size % BYTE_LENGTH;

    const unsigned int bitmap_block_size = block_size(metadata_bytes + (metadata_extra_bits > 0));
    const unsigned int bitmap_byte_size = bitmap_block_size * BLOCK_SIZE;

    unsigned char bitmap[bitmap_byte_size];

    memset(bitmap, FULL_BYTE, metadata_bytes);
    memset(bitmap + metadata_bytes + 1, EMPTY_BYTE, bitmap_byte_size - metadata_bytes);
    bitmap[metadata_bytes] = FULL_BYTE << (BYTE_LENGTH - metadata_extra_bits);

    for (int i = 0; i < bitmap_block_size; ++i) {
        if (write_block(sb.bitmap_first_block + i, &bitmap[i * BLOCK_SIZE]) < 0) return FAILURE;
    }

    sb.free_blocks_count -= metadata_block_size;

    if (write_block(SUPER_BLOCK_POSITION, &sb) < 0) return FAILURE;

    return SUCCESS;
}

int init_inodes() {
    super_block_t sb;
    if (read_block(SUPER_BLOCK_POSITION, &sb) < 0) return FAILURE;

    inode_t inodes[INODES_PER_BLOCK];

    unsigned int next_inode = sb.next_free_inode_position + 1;

    for (unsigned int i = sb.inodes_first_block; i <= sb.inodes_last_block; ++i) {
        for (int j = 0; j < INODES_PER_BLOCK; ++j) {
            inodes[j].direct[0] = next_inode++;
            inodes[j].metadata.type = INODE_FREE;
        }

        if (i == sb.inodes_last_block) inodes[INODES_PER_BLOCK - 1].direct[0] = UINT_MAX;

        if (write_block(i, inodes) < 0) return FAILURE;
    }

    return SUCCESS;
}

int set_block_state(unsigned int physical_block, unsigned int state) {
    super_block_t sb;
    if (read_block(SUPER_BLOCK_POSITION, &sb) < 0) return FAILURE;

    const unsigned int block_byte = physical_block / BYTE_LENGTH;
    const unsigned int absolute_block = sb.bitmap_first_block + (block_byte / BLOCK_SIZE);

    unsigned char bitmap[BLOCK_SIZE];
    if (read_block(absolute_block, bitmap) < 0) return FAILURE;

    const unsigned char mask = BIT7 >> (physical_block % BYTE_LENGTH);

    if (state) {
        bitmap[block_byte] |= mask;
    } else {
        bitmap[block_byte] &= ~mask;
    }

    if (write_block(absolute_block, &bitmap) < 0) return FAILURE;

    return SUCCESS;
}

char get_block_state(unsigned int physical_block) {
    super_block_t sb;
    if (read_block(SUPER_BLOCK_POSITION, &sb) < 0) return FAILURE;

    const unsigned int block_byte = physical_block / BYTE_LENGTH;
    const unsigned int absolute_block = sb.bitmap_first_block + (block_byte / BLOCK_SIZE);

    unsigned char bitmap[BLOCK_SIZE];
    if (read_block(absolute_block, bitmap) < 0) return FAILURE;

    return (char) ((bitmap[block_byte % BLOCK_SIZE] & (BIT7 >> physical_block % BYTE_LENGTH)) > 0);
}

int reserve_block() {
    super_block_t sb;
    if (read_block(SUPER_BLOCK_POSITION, &sb) < 0) return FAILURE;

    if (sb.free_blocks_count == 0) return FAILURE;

    unsigned int bitmap_block_length = sb.bitmap_last_block - sb.bitmap_first_block;

    int bitmap_block = -1;

    unsigned char bitmap[BLOCK_SIZE];

    unsigned char full_block[BLOCK_SIZE];
    memset(full_block, FULL_BYTE, BLOCK_SIZE);

    const unsigned char empty_block[BLOCK_SIZE] = {0};

    do {
        if (read_block(sb.bitmap_first_block + ++bitmap_block, bitmap) < 0) return FAILURE;

    } while (memcmp(bitmap, full_block, BLOCK_SIZE) == 0 && bitmap_block < bitmap_block_length);

    for (unsigned int byte = 0; byte < BLOCK_SIZE; ++byte) {
        if (bitmap[byte] == FULL_BYTE) continue;

        unsigned char bit = 0;

        for (unsigned char mask = BIT7; bitmap[byte] & mask; mask >>= 1) bit++;

        const unsigned int block_position = (bitmap_block * BLOCK_SIZE + byte) * BYTE_LENGTH + bit;

        if (set_block_state(block_position, BUSY_BLOCK) < 0) return FAILURE;

        sb.free_blocks_count--;

        if (write_block(SUPER_BLOCK_POSITION, &sb) < 0) return FAILURE;
        if (write_block(block_position, empty_block) < 0) return FAILURE;

        return (int) block_position;
    }

    return FAILURE;
}

int free_block(unsigned int physical_block) {
    super_block_t sb;
    if (read_block(SUPER_BLOCK_POSITION, &sb) < 0) return FAILURE;

    if (sb.blocks_count == sb.free_blocks_count) return (int) physical_block;

    if (set_block_state(physical_block, FREE_BLOCK) < 0) return FAILURE;

    sb.free_blocks_count++;

    if (write_block(SUPER_BLOCK_POSITION, &sb) < 0) return FAILURE;

    return (int) physical_block;
}

int write_inode(unsigned int inode_position, inode_t *inode) {
    super_block_t sb;
    if (read_block(SUPER_BLOCK_POSITION, &sb) < 0) return FAILURE;

    const unsigned int block = sb.inodes_first_block + inode_position / INODES_PER_BLOCK;

    inode_t inodes[INODES_PER_BLOCK];
    if (read_block(block, inodes) < 0) return FAILURE;

    inodes[inode_position % INODES_PER_BLOCK] = *inode;

    if (write_block(block, inodes) < 0) return FAILURE;

    return SUCCESS;
}

int read_inode(unsigned int inode_position, inode_t *inode) {
    super_block_t sb;
    if (read_block(SUPER_BLOCK_POSITION, &sb) < 0) return FAILURE;

    const unsigned int block = sb.inodes_first_block + inode_position / INODES_PER_BLOCK;

    inode_t inodes[INODES_PER_BLOCK];
    if (read_block(block, inodes) < 0) return FAILURE;

    *inode = inodes[inode_position % INODES_PER_BLOCK];

    return SUCCESS;
}

int reserve_inode(unsigned char type, unsigned char permissions) {
    super_block_t sb;
    if (read_block(SUPER_BLOCK_POSITION, &sb) < 0) return FAILURE;

    if (sb.free_inodes_count == 0) return FAILURE;

    const unsigned int inode_position = sb.next_free_inode_position;

    inode_t inode;
    if (read_inode(inode_position, &inode) < 0) return FAILURE;

    sb.next_free_inode_position = inode.direct[0];

    inode.metadata.type = type;
    inode.metadata.permissions = permissions;
    inode.metadata.links_count = 1;
    inode.metadata.size = 0;
    inode.metadata.data_accessed_at = time(NULL);
    inode.metadata.data_modified_at = inode.metadata.data_accessed_at;
    inode.metadata.modified_at = inode.metadata.data_accessed_at;
    inode.metadata.busy_blocks_count = 0;

    memset(inode.direct, 0, DIRECT_POINTERS * sizeof(unsigned int));
    memset(inode.indirect, 0, INDIRECT_POINTERS * sizeof(unsigned int));

    if (write_inode(inode_position, &inode) < 0) return FAILURE;

    sb.free_inodes_count--;

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
int get_block_level(unsigned int logical_block, inode_t *inode, unsigned int *block_ptr) {
    for (int lvl = 0; lvl < NUM_LEVELS; ++lvl) {
        if (logical_block < ranges[lvl]) {
            *block_ptr = lvl > 0 ? inode->indirect[lvl - 1] : inode->direct[logical_block];
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
int reserve_inode_block(inode_t *inode, unsigned int logical_block, unsigned int logical_block_level,
                        unsigned int reserve_block_level, unsigned int *intermediate_pointer_block,
                        unsigned int intermediate_pointer_block_position, unsigned int block_ptr_position) {

    const int block_ptr = reserve_block();
    if (block_ptr < 0) return FAILURE;

    inode->metadata.busy_blocks_count++;
    inode->metadata.modified_at = time(NULL);

    if (logical_block_level == 0) {
        debug(DEBUG_INODES, "directPointers[%d] = %d", logical_block, block_ptr);
        inode->direct[logical_block] = block_ptr;
        return block_ptr;
    }

    if (logical_block_level == reserve_block_level) {
        debug(DEBUG_INODES, "indirectPointers[%d] = %d", reserve_block_level - 1, block_ptr);
        inode->indirect[reserve_block_level - 1] = block_ptr;
        return block_ptr;
    }

    debug(DEBUG_INODES, "nivel%d[%d] = %d", reserve_block_level + 1, block_ptr_position, block_ptr);
    intermediate_pointer_block[block_ptr_position] = block_ptr;

    if (write_block(intermediate_pointer_block_position, intermediate_pointer_block) < 0) return FAILURE;

    return block_ptr;
}

int get_physical_block(inode_t *inode, unsigned int logical_block, unsigned char reserve) {
    unsigned int prev_block_ptr;
    unsigned int block_ptr;
    int block_ptr_position;

    const int logical_block_level = get_block_level(logical_block, inode, &block_ptr);
    if (logical_block_level < 0) return FAILURE;

    unsigned int pointer_block[POINTERS_PER_BLOCK];

    for (int current_level = logical_block_level; current_level > 0; --current_level) {
        if (block_ptr) {
            if (read_block(block_ptr, pointer_block) < 0) return FAILURE;
        } else {
            if (!reserve) return FAILURE;

            const int reserved_block_ptr = reserve_inode_block(
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
        debug(DEBUG_INODES, "Se ha liberado el bloque de datos %d", *physical_block);
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
        debug(DEBUG_INODES, "Se ha liberado el bloque de punteros %d", *physical_block);
        *physical_block = 0;
    } else if (modified) {
        write_block(*physical_block, pointer_block);
        debug(DEBUG_INODES, "Se ha escrito el bloque de punteros %d", *physical_block);
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

int free_inode_blocks(unsigned int first_logical_block, inode_t *inode) {
    const int position = get_block_pointer_position((int) first_logical_block);
    unsigned int freed_blocks = 0;

    for (int i = position; i < POINTERS; ++i) {
        unsigned int *block_ptr = inode->direct + i;

        if (*block_ptr) {
            freed_blocks += free_inode_tree(
                    block_ptr,
                    i == position ? first_logical_block : 0,
                    i < DIRECT_POINTERS ? 0 : i - DIRECT_POINTERS + 1
            );
        }
    }
    debug(DEBUG_INODES, "Se han liberado %d bloques", freed_blocks);
    return (int) freed_blocks;
}

int free_inode(unsigned int inode_position) {
    inode_t inode;
    if (read_inode(inode_position, &inode) < 0) return FAILURE;

    const int freed_blocks = free_inode_blocks(0, &inode);
    if (freed_blocks < 0) return FAILURE;

    inode.metadata.modified_at = time(NULL);
    inode.metadata.busy_blocks_count -= freed_blocks;
    inode.metadata.size = 0;
    inode.metadata.type = INODE_FREE;

    super_block_t sb;
    if (read_block(SUPER_BLOCK_POSITION, &sb) < 0) return FAILURE;

    inode.direct[0] = sb.next_free_inode_position;
    sb.next_free_inode_position = inode_position;
    sb.free_inodes_count++;

    if (write_block(SUPER_BLOCK_POSITION, &sb) < 0) return FAILURE;
    if (write_inode(inode_position, &inode) < 0) return FAILURE;

    return (int) inode_position;
}

void format_datetime(char *str, const time_t *time) {
    strftime(str, DATETIME_LENGTH, DATETIME_FORMAT, localtime(time));
}

int print_inode(metadata_t *metadata, char *name) {
    if (name) printf("\n *** %s ***\n", name);

    char data_accessed_at[DATETIME_LENGTH];
    format_datetime(data_accessed_at, &metadata->data_accessed_at);

    char data_modified_at[DATETIME_LENGTH];
    format_datetime(data_modified_at, &metadata->data_modified_at);

    char modified_at[DATETIME_LENGTH];
    format_datetime(modified_at, &metadata->modified_at);

    printf(
            "Tipo: %c\n"
            "Permisos: %d\n"
            "-------------------------------------\n"
            "Último acceso de datos: %s\n"
            "Última modificación de datos: %s\n"
            "Última modificación del i-nodo: %s\n"
            "-------------------------------------\n"
            "Número de enlaces: %d\n"
            "Tamaño en bytes lógicos: %d\n"
            "Número de bloques ocupados: %d\n",
            metadata->type,
            metadata->permissions,
            data_accessed_at,
            data_modified_at,
            modified_at,
            metadata->links_count,
            metadata->size,
            metadata->busy_blocks_count
    );

    return SUCCESS;
}

int are_valid(char permissions) {
    return permissions >= 0 && permissions <= RWX;
}