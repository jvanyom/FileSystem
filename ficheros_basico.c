#include "ficheros_basico.h"

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

/**
 * Calcula el tamaño en bloques del mapa de bits (MB).
 *
 * @param blocks_num Número total de bloques del dispositivo.
 *
 * @return Tamaño calculado.
 */
int tamMB(unsigned int blocks_num) {
    return block_size(div_ceil((int) blocks_num, BYTE_LENGTH));
}

/**
 * Calcula el tamaño en bloques del array de i-nodos (AI).
 *
 * @param inodes_num Número de i-nodos.
 *
 * @return Tamaño calculado.
 */
int tamAI(unsigned int inodes_num) {
    return block_size((int) (inodes_num * INODE_SIZE));
}

/**
 * Inicializa el súper bloque (SB).
 *
 * @param blocks_num Número de bloques del dispositivo.
 * @param inodes_num Número de i-nodos.
 *
 * @return Número de bytes escritos o -1 si ha habido algún error.
 */
int initSB(unsigned int blocks_num, unsigned int inodes_num) {
    const struct SuperBlock sb = {
            .bitMapFirstBlock = SUPER_BLOCK_POSITION + SUPER_BLOCKS_PER_BLOCK,
            .bitMapLastBlock = sb.bitMapFirstBlock + tamMB(blocks_num) - 1,
            .iNodesFirstBlock = sb.bitMapLastBlock + 1,
            .iNodesLastBlock = sb.iNodesFirstBlock + tamAI(inodes_num) - 1,
            .dataFirstBlock = sb.iNodesLastBlock + 1,
            .dataLastBlock = blocks_num - 1,
            .rootINode = 0,
            .firstFreeINode = 0,
            .totalFreeBlocks = blocks_num,
            .totalFreeINodes = inodes_num,
            .totalBlocks = blocks_num,
            .totalINodes = inodes_num
    };

    return bwrite(SUPER_BLOCK_POSITION, &sb);
}

/**
 * Inicializa el mapa de bits (MB) con un 1 en los bits que representan los metadatos.
 *
 * @return Número positivo si se ha inicializado correctamente o un -1 en caso contrario.
 */
int initMB() {
    struct SuperBlock sb;

    if (bread(SUPER_BLOCK_POSITION, &sb) == FAILURE) return FAILURE;

    const int metadata_block_size = tamMB(sb.totalBlocks) + tamAI(sb.totalINodes) + SUPER_BLOCKS_PER_BLOCK;

    const int metadata_bytes = metadata_block_size / BYTE_LENGTH;
    const int metadata_extra_bits = metadata_block_size % BYTE_LENGTH;

    const int bitmap_block_size = block_size(metadata_bytes + (metadata_extra_bits > 0));
    const int bitmap_byte_size = bitmap_block_size * BLOCK_SIZE;

    unsigned char bitmap[bitmap_byte_size];

    memset(bitmap, FULL_BYTE, metadata_bytes);
    memset(bitmap + metadata_bytes + 1, EMPTY_BYTE, bitmap_byte_size - metadata_bytes);

    bitmap[metadata_bytes] = FULL_BYTE << (BYTE_LENGTH - metadata_extra_bits);

    for (int i = 0; i < bitmap_block_size; ++i) {
        if (bwrite(sb.bitMapFirstBlock + i, &bitmap[i * BLOCK_SIZE]) == FAILURE) return FAILURE;
    }

    sb.totalFreeBlocks -= metadata_block_size;

    if (bwrite(SUPER_BLOCK_POSITION, &sb) == FAILURE) return FAILURE;

    return SUCCESS;
}

/**
 * Inicializa la lista de i-nodos libres (AI). Todos los i-nodos se enlazan con el siguiente a través del primer puntero
 * directo 'directPointers[0]'.
 *
 * @return Número positivo si se ha inicializado correctamente o un -1 en caso contrario.
 */
int initAI() {
    struct SuperBlock sb;

    if (bread(SUPER_BLOCK_POSITION, &sb) == FAILURE) return FAILURE;

    struct INode inodes[INODES_PER_BLOCK];

    unsigned int next_inode = sb.firstFreeINode + 1;

    for (unsigned int i = sb.iNodesFirstBlock; i <= sb.iNodesLastBlock; ++i) {
        for (int j = 0; j < INODES_PER_BLOCK; ++j) {
            inodes[j].directPointers[0] = next_inode++;
            inodes[j].type = INODE_FREE;
        }

        if (i == sb.iNodesLastBlock) inodes[LAST_INODE].directPointers[0] = UINT_MAX;

        if (bwrite(i, inodes) == FAILURE) return FAILURE;
    }

    return SUCCESS;
}

/**
 * Modifica el estado de un bloque en el mapa de bits. Si tiene que estar libre, se escribe un 0 y un 1 en caso contrario.
 *
 * @param block Bloque físico del que se quiere modificar su estado.
 * @param bit 0 o 1. Indica que el bloque se queda libre o ocupado, respectivamente.
 *
 * @return 1 si se ha escrito el bit correctamente o -1 en otro caso.
 */
int escribir_bit(unsigned int block, unsigned int bit) {
    struct SuperBlock sb;

    if (bread(SUPER_BLOCK_POSITION, &sb) == FAILURE) return FAILURE;

    const unsigned int block_byte = block / BYTE_LENGTH;
    const unsigned int absolute_block = sb.bitMapFirstBlock + (block_byte / BLOCK_SIZE);

    unsigned char bitmap[BLOCK_SIZE];

    if (bread(absolute_block, bitmap) == FAILURE) return FAILURE;

    const unsigned char mask = BIT7 >> (block % BYTE_LENGTH);

    unsigned char *byte_to_modify = &bitmap[block_byte];

    *byte_to_modify = bit ? *byte_to_modify | mask : *byte_to_modify & ~mask;

    if (bwrite(absolute_block, &bitmap) == FAILURE) return FAILURE;

    return SUCCESS;
}

/**
 * Lee el valor del bit (estado) en el mapa de bits de un bloque físico.
 *
 * @param block Bloque del que se quiere conocer el estado en el mapa de bits.
 *
 * @return Valor del bit leído.
 */
char leer_bit(unsigned int block) {
    struct SuperBlock sb;

    if (bread(SUPER_BLOCK_POSITION, &sb) == FAILURE) return FAILURE;

    const unsigned int block_byte = block / BYTE_LENGTH;
    const unsigned int absolute_block = sb.bitMapFirstBlock + (block_byte / BLOCK_SIZE);

    unsigned char bitmap[BLOCK_SIZE];

    if (bread(absolute_block, bitmap) == FAILURE) return FAILURE;

    return (char) ((bitmap[block_byte % BLOCK_SIZE] & (BIT7 >> block % BYTE_LENGTH)) > 0);
}

/**
 * Encuentra el primer bloque libre y lo ocupa.
 *
 * @return Posición del bloque reservado.
 */
int reservar_bloque() {
    struct SuperBlock sb;

    if (bread(SUPER_BLOCK_POSITION, &sb) == FAILURE) return FAILURE;

    if (sb.totalFreeBlocks == 0) return FAILURE;

    unsigned int bitmap_block_length = sb.bitMapLastBlock - sb.bitMapFirstBlock;

    int bitmap_block = -1;

    unsigned char bitmap[BLOCK_SIZE];
    unsigned char aux_bitmap[BLOCK_SIZE];

    memset(aux_bitmap, FULL_BYTE, BLOCK_SIZE);

    do {
        if (bread(sb.bitMapFirstBlock + ++bitmap_block, bitmap) == FAILURE) return FAILURE;

    } while (memcmp(bitmap, aux_bitmap, BLOCK_SIZE) == 0 && bitmap_block < bitmap_block_length);

    for (unsigned int byte = 0; byte < BLOCK_SIZE; ++byte) {
        if (bitmap[byte] == FULL_BYTE) continue;

        unsigned char bit = 0;

        for (unsigned char mask = BIT7; bitmap[byte] & mask; mask >>= 1) bit++;

        const unsigned int block_position = (bitmap_block * BLOCK_SIZE + byte) * BYTE_LENGTH + bit;

        if (escribir_bit(block_position, BUSY_BLOCK) == FAILURE) return FAILURE;

        sb.totalFreeBlocks--;

        if (bwrite(SUPER_BLOCK_POSITION, &sb) == FAILURE) return FAILURE;

        unsigned char empty[BLOCK_SIZE];
        memset(empty, EMPTY_BYTE, BLOCK_SIZE);

        if (bwrite(block_position, empty) == FAILURE) return FAILURE;

        return (int) block_position;
    }

    return FAILURE;
}

/**
 * Librar un bloque físico.
 *
 * @param block Índice del bloque físico a eliminar.
 *
 * @return Índice del bloque.
 */
int liberar_bloque(unsigned int block) {
    struct SuperBlock sb;

    if (bread(SUPER_BLOCK_POSITION, &sb) == FAILURE) return FAILURE;

    if (sb.totalBlocks == sb.totalFreeBlocks) return SUCCESS;

    if (escribir_bit(block, FREE_BLOCK) == FAILURE) return FAILURE;

    sb.totalFreeBlocks++;

    if (bwrite(SUPER_BLOCK_POSITION, &sb) == FAILURE) return FAILURE;

    return (int) block;
}

/**
 * Escribe el contenido de un struct INode dentro del espacio reservado para el i-nodo.
 *
 * @param inode_position Índice del i-nodo en el que se quiere escribir.
 * @param inode Contenido del i-nodo.
 *
 * @return 1 si se ha podido guardar la información o -1 si ha habido algún error.
 */
int escribir_inodo(unsigned int inode_position, struct INode *inode) {
    struct SuperBlock sb;

    if (bread(SUPER_BLOCK_POSITION, &sb) == FAILURE) return FAILURE;

    const unsigned int block = sb.iNodesFirstBlock + inode_position / INODES_PER_BLOCK;

    struct INode inodes[INODES_PER_BLOCK];

    if (bread(block, inodes) == FAILURE) return FAILURE;

    inodes[inode_position % INODES_PER_BLOCK] = *inode;

    if (bwrite(block, inodes) == FAILURE) return FAILURE;

    return SUCCESS;
}

/**
 * Leer un i-nodo dada su posición.
 *
 * @param inode_position Índice del i-nodo a leer.
 * @param inode I-nodo donde se va a guardar la información.
 *
 * @return 1 si se ha leído correctamente o -1 si ha habido algún error.
 */
int leer_inodo(unsigned int inode_position, struct INode *inode) {
    struct SuperBlock sb;

    if (bread(SUPER_BLOCK_POSITION, &sb) == FAILURE) return FAILURE;

    const unsigned int block = sb.iNodesFirstBlock + inode_position / INODES_PER_BLOCK;

    struct INode inodes[INODES_PER_BLOCK];

    if (bread(block, inodes) == FAILURE) return FAILURE;

    *inode = inodes[inode_position % INODES_PER_BLOCK];

    return SUCCESS;
}

/**
 * Reservar el primer i-nodo libre.
 *
 * @param type Tipo de i-nodo.
 * @param permissions Permisos del i-nodo.
 *
 * @return Posición del i-nodo reservado.
 */
int reservar_inodo(unsigned char type, unsigned char permissions) {
    struct SuperBlock sb;

    if (bread(SUPER_BLOCK_POSITION, &sb) == FAILURE) return FAILURE;

    if (sb.totalFreeINodes == 0) return FAILURE;

    const unsigned int inode_position = sb.firstFreeINode;

    struct INode inode;

    if (leer_inodo(inode_position, &inode) == FAILURE) return FAILURE;

    sb.firstFreeINode = inode.directPointers[0];

    inode.type = type;
    inode.permissions = permissions;
    inode.totalLinks = 1;
    inode.logicalBytesSize = 0;
    inode.dataAccessedAt = time(NULL);
    inode.dataModifiedAt = inode.dataAccessedAt;
    inode.iNodeModifiedAt = inode.dataAccessedAt;
    inode.totalBusyBlocks = 0;

    memset(inode.directPointers, 0, DIRECT_POINTERS * sizeof(unsigned int));
    memset(inode.indirectPointers, 0, INDIRECT_POINTERS * sizeof(unsigned int));

    if (escribir_inodo(inode_position, &inode) == FAILURE) return FAILURE;

    sb.totalFreeINodes--;

    if (bwrite(SUPER_BLOCK_POSITION, &sb) == FAILURE) return FAILURE;

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
unsigned int get_block_level(unsigned int logical_block, struct INode *inode, unsigned int *block_ptr) {
    for (unsigned int lvl = 0; lvl < NUM_LEVELS; ++lvl) {
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
unsigned int block_pointer_position_by_level(unsigned int logical_block, unsigned int level) {
    for (int lvl = 0; lvl < NUM_LEVELS; ++lvl) {
        if (logical_block < ranges[lvl]) {
            const unsigned int relative_block = lvl > 0 ? logical_block - ranges[lvl - 1] : logical_block;

            const unsigned int total_lvl_pointers = power(POINTERS_PER_BLOCK, lvl - 1);

            if (level == lvl) return relative_block / total_lvl_pointers;

            const unsigned int extra_lvl_pointers = power(POINTERS_PER_BLOCK, lvl - 2);

            if (level == lvl - 1) return (relative_block % total_lvl_pointers) / extra_lvl_pointers;
            if (level == lvl - 2) return (relative_block % total_lvl_pointers) % POINTERS_PER_BLOCK;

            return relative_block;
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

    const int block_ptr = reservar_bloque();

    if (block_ptr == FAILURE) return FAILURE;

    inode->totalBusyBlocks++;
    inode->iNodeModifiedAt = time(NULL);

    if (logical_block_level == 0) {
#if DEBUG
        printf(LIGHT_GRAY"[reserve_inode_block → directPointers[%d] = %d]\n"RESET, logical_block, block_ptr);
#endif
        inode->directPointers[logical_block] = block_ptr;
        return block_ptr;
    }

    if (logical_block_level == reserve_block_level) {
#if DEBUG
        printf(LIGHT_GRAY"[reserve_inode_block → indirectPointers[%d] = %d]\n"RESET, reserve_block_level - 1, block_ptr);
#endif
        inode->indirectPointers[reserve_block_level - 1] = block_ptr;
        return block_ptr;
    }
#if DEBUG
    printf(LIGHT_GRAY"[reserve_inode_block → nivel%d[%d] = %d]\n"RESET, reserve_block_level + 1, block_ptr_position,block_ptr);
#endif
    intermediate_pointer_block[block_ptr_position] = block_ptr;

    if (bwrite(intermediate_pointer_block_position, intermediate_pointer_block) == FAILURE) return FAILURE;

    return block_ptr;
}

/**
 * Obtener el número de bloque físico correspondiente a un bloque lógico del i-nodo indicado.
 *
 * @param inode I-Nodo
 * @param logical_block Número de bloque lógico
 * @param reserve Si tiene que reservarse o no.
 *
 * @return Puntero al bloque físico correspondiente al bloque lógico especificado.
 */
int traducir_bloque_inodo(struct INode *inode, unsigned int logical_block, unsigned char reserve) {
    unsigned int block_ptr;
    unsigned int prev_block_ptr;
    unsigned int block_ptr_position;

    const unsigned int logical_block_level = get_block_level(logical_block, inode, &block_ptr);

    unsigned int pointer_block[POINTERS_PER_BLOCK] = {0};

    for (unsigned int current_level = logical_block_level; current_level > 0; --current_level) {
        if (block_ptr) {
            if (bread(block_ptr, pointer_block) == FAILURE) return FAILURE;
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

            if (reserved_block_ptr == FAILURE) return FAILURE;

            memset(pointer_block, EMPTY_BYTE, BLOCK_SIZE);

            block_ptr = reserved_block_ptr;
        }

        block_ptr_position = block_pointer_position_by_level(logical_block, current_level);

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