#include "../ficheros_basico.h"

int print_super_block(struct SuperBlock *sb) {
    if (bread(SUPER_BLOCK_POSITION, sb) == FAILURE) return FAILURE;

    printf("\n*** SUPER BLOQUE ***\n");

    printf(
            "-------------------------------------\n"
            "Primer bloque Bitmap: %d\n"
            "Último bloque Bitmap: %d\n"
            "-------------------------------------\n"
            "Primer bloque array i-nodos: %d\n"
            "Último bloque array i-nodos: %d\n"
            "-------------------------------------\n"
            "Primer bloque datos: %d\n"
            "Último bloque datos: %d\n"
            "-------------------------------------\n"
            "I-Nodo raíz: %d\n"
            "Primer i-nodo libre: %d\n"
            "-------------------------------------\n"
            "Bloques libres: %d\n"
            "I-Nodos libres: %d\n"
            "-------------------------------------\n"
            "Cantidad de bloques: %d\n"
            "Cantidad de i-nodos: %d\n"
            "-------------------------------------\n",
            sb->bitMapFirstBlock, sb->bitMapLastBlock,
            sb->iNodesFirstBlock, sb->iNodesLastBlock,
            sb->dataFirstBlock, sb->dataLastBlock,
            sb->rootINode, sb->firstFreeINode,
            sb->totalFreeBlocks, sb->totalFreeINodes,
            sb->totalBlocks, sb->totalINodes
    );

    printf("Tamaño súper bloque: %lu\n", sizeof(struct SuperBlock));

    return SUCCESS;
}


int print_inodes(struct SuperBlock *sb) {
    printf("\n*** I-NODOS ***\n");

    printf("Tamaño i-nodo: %lu\n", sizeof(struct INode));

    struct INode inodes[INODES_PER_BLOCK];

    for (unsigned int i = sb->iNodesFirstBlock; i <= sb->iNodesLastBlock; ++i) {
        if (bread(i, &inodes) == FAILURE) return FAILURE;

        for (unsigned int j = 0; j < INODES_PER_BLOCK; ++j) {
            printf("%d, ", inodes[j].directPointers[0]);
        }

        printf("\n");
    }

    return SUCCESS;
}

int print_bit(unsigned int block) {
    const unsigned char bit = leer_bit(block);

    printf("Bloque: %d | Bit: %d\n", block, bit);

    return SUCCESS;
}

int print_bitmap(struct SuperBlock *sb) {
    printf("\n*** BITMAP ***\n");

    print_bit(SUPER_BLOCK_POSITION);

    print_bit(sb->bitMapFirstBlock);
    print_bit(sb->bitMapLastBlock);

    print_bit(sb->iNodesFirstBlock);
    print_bit(sb->iNodesLastBlock);

    print_bit(sb->dataFirstBlock);
    print_bit(sb->dataLastBlock);

    return SUCCESS;
}

int print_blocks(struct SuperBlock *sb) {
    printf("\n*** RESERVA Y LIBERACIÓN DE BLOQUES ***\n");

    const unsigned int block = reservar_bloque();

    printf("Se ha reservado el bloque físico %d.\n", block);

    if (bread(SUPER_BLOCK_POSITION, sb) == FAILURE) return FAILURE;

    printf("Bloques libres: %d\n", sb->totalFreeBlocks);

    if (liberar_bloque(block) == FAILURE) return FAILURE;

    printf("Se ha liberado el bloque físico %d.\n", block);

    if (bread(SUPER_BLOCK_POSITION, sb) == FAILURE) return FAILURE;

    printf("Bloques libres: %d\n", sb->totalFreeBlocks);

    return SUCCESS;
}

int print_inode(struct INode *inode, char *inode_name) {
    printf("\n *** %s ***\n", inode_name);

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
            inode->type,
            inode->permissions,
            ctime(&inode->dataAccessedAt),
            ctime(&inode->dataModifiedAt),
            ctime(&inode->iNodeModifiedAt),
            inode->totalLinks,
            inode->logicalBytesSize,
            inode->totalBusyBlocks
    );

    return SUCCESS;
}

int print_root_inode(struct SuperBlock *sb) {
    struct INode root;

    if (leer_inodo(sb->rootINode, &root) == FAILURE) return FAILURE;

    return print_inode(&root, "RAÍZ");
}

int print_logical_blocks_translation() {
    printf("\n*** TRADUCCIÓN DE BLOQUES LÓGICOS ***\n");

    const static unsigned int logical_blocks[] = {8, 204, 30004, 400004, 468750};

    const int inode_position = reservar_inodo(INODE_FILE, READ | WRITE);

    struct INode inode;

    if (leer_inodo(inode_position, &inode) == FAILURE) return FAILURE;

    if (inode_position == FAILURE) return FAILURE;

    for (int i = 0; i < sizeof(logical_blocks) / sizeof(unsigned int); ++i) {
        traducir_bloque_inodo(&inode, logical_blocks[i], RESERVE);
    }

    return print_inode(&inode, "I-NODO RESERVADO");
}

int main(__attribute__((unused)) int argc, char **argv) {

    if (bmount(argv[1]) == FAILURE) return FAILURE;

    struct SuperBlock sb;

    print_super_block(&sb);
//    print_blocks(&sb);
//    print_bitmap(&sb);
//    print_logical_blocks_translation();

    printf("Posición primer i-nodo libre: %d", sb.firstFreeINode);

    if (bumount() == FAILURE) return FAILURE;

    return EXIT_SUCCESS;
}