#include "../directories.h"

#define DEBUG_SUPER_BLOCK 1
#define DEBUG_INODES 0
#define DEBUG_BITMAP 0
#define DEBUG_BLOCKS 0
#define DEBUG_ROOT_INODE 0
#define DEBUG_BLOCK_TRANSLATION 0
#define DEBUG_FIND_DIR 1

int print_super_block(struct SuperBlock *sb) {
    if (read_block(SUPER_BLOCK_POSITION, sb) < 0) {
        return f("No ha sido posible leer correctamente el súper bloque");
    }

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
            "Bloques libres: "LIGHT_BLUE"%d\n"RESET
            "I-Nodos libres: "LIGHT_BLUE"%d\n"RESET
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
    printf("Tamaño i-nodo: %lu\n", sizeof(struct INode));

    return SUCCESS;
}

int print_inodes(struct SuperBlock *sb) {
    printf("\n*** I-NODOS ***\n");

    struct INode inodes[INODES_PER_BLOCK];

    for (unsigned int i = sb->iNodesFirstBlock; i <= sb->iNodesLastBlock; ++i) {
        if (read_block(i, &inodes) < 0) {
            return f("No ha sido posible leer correctamente el bloque físico %d", i);
        }

        for (unsigned int j = 0; j < INODES_PER_BLOCK; ++j) {
            printf("%d, ", inodes[j].directPointers[0]);
        }

        printf("\n");
    }

    return SUCCESS;
}

int print_block_state(unsigned int block) {
    const unsigned char state = get_block_state(block);
    if (state < 0) return f("No ha sido posible leer correctamente el estado del bloque %d", block);

    printf("Bloque: %d | Estado: %d\n", block, state);

    return SUCCESS;
}

int print_bitmap(struct SuperBlock *sb) {
    printf("\n*** BITMAP ***\n");

    return (
            print_block_state(SUPER_BLOCK_POSITION) < 0 ||
            print_block_state(sb->bitMapFirstBlock) < 0 ||
            print_block_state(sb->bitMapLastBlock) < 0 ||
            print_block_state(sb->iNodesFirstBlock) < 0 ||
            print_block_state(sb->iNodesLastBlock) < 0 ||
            print_block_state(sb->dataFirstBlock) < 0 ||
            print_block_state(sb->dataLastBlock) < 0
    );
}

int print_blocks(struct SuperBlock *sb) {
    printf("\n*** RESERVA Y LIBERACIÓN DE BLOQUES ***\n");

    const int block = reserve_block();
    if (block < 0) return f("No ha sido posible reservar correctamente un bloque nuevo");

    printf("Se ha reservado el bloque físico %d.\n", block);

    if (read_block(SUPER_BLOCK_POSITION, sb) < 0) {
        return f("No ha sido posible leer correctamente el súper bloque");
    }

    printf("Bloques libres: %d\n", sb->totalFreeBlocks);

    if (free_block(block) < 0) {
        return f("No ha sido posible liberar correctamente el bloque %d", block);
    }

    printf("Se ha liberado el bloque físico %d.\n", block);

    if (read_block(SUPER_BLOCK_POSITION, sb) < 0) {
        return f("No ha sido posible leer correctamente el súper bloque");
    }

    printf("Bloques libres: %d\n", sb->totalFreeBlocks);

    return SUCCESS;
}

int print_root_inode(struct SuperBlock *sb) {
    struct INode root;

    if (read_inode(sb->rootINode, &root) < 0) {
        return f("No ha sido posible leer correctamente el i-nodo raíz (%d)", sb->rootINode);
    }

    return print_inode(&root.metadata, "RAÍZ");
}

int print_logical_blocks_translation() {
    printf("\n*** TRADUCCIÓN DE BLOQUES LÓGICOS ***\n");

    const static unsigned int logical_blocks[] = {8, 204, 30004, 400004, 468750};

    const int inode_position = reserve_inode(FILE_INODE, READ | WRITE);
    if (inode_position < 0) return f("No ha sido posible reservar un nuevo i-nodo");

    struct INode inode;
    if (read_inode(inode_position, &inode) < 0) {
        return f("No ha sido posible leer correctamente el i-nodo %d", inode_position);
    }

    for (int i = 0; i < sizeof(logical_blocks) / sizeof(unsigned int); ++i) {
        if (get_physical_block(&inode, logical_blocks[i], RESERVE) < 0) {
            return f(
                    "No ha sido posible obtener correctamente el número bloque físico del bloque lógico %d",
                    logical_blocks[i]
            );
        }
    }

    return print_inode(&inode.metadata, "I-NODO RESERVADO");
}

int print_find_entry(char *path, char reserve) {
    printf("\nRuta: %s | Reservar: %d\n", path, reserve);
    printf("\n********************************************************************\n");

    const signed error = find_entry(path, 0, reserve, RW);
    if (error < 0) return failure(error);

    return SUCCESS;
}

int print_find_entries() {
    print_find_entry("pruebas/", RESERVE);                    // BAD_PATH
    print_find_entry("/pruebas/", NO_RESERVE);                // BAD_PATH
    print_find_entry("/pruebas/docs/", RESERVE);              // BAD_PATH
    print_find_entry("/pruebas/", RESERVE);                   // Se crea /pruebas/
    print_find_entry("/pruebas/docs/", RESERVE);              // Se crea /pruebas/docs/
    print_find_entry("/pruebas/docs/doc1", RESERVE);          // Se crea /pruebas/docs/doc1
    print_find_entry("/pruebas/docs/doc1/doc11", RESERVE);    // IS_FILE
    print_find_entry("/pruebas/", RESERVE);                   // FILE_ALREADY_EXISTS
    print_find_entry("/pruebas/docs/doc1", NO_RESERVE);       // Se consulta /pruebas/docs/doc1
    print_find_entry("/pruebas/docs/doc1", RESERVE);          // FILE_ALREADY_EXISTS
    print_find_entry("/pruebas/casos/", RESERVE);             // Se crea /pruebas/casos/
    print_find_entry("/pruebas/docs/doc2", RESERVE);          // Se crea /pruebas/docs/doc2

    return SUCCESS;
}

int main(int argc, char **argv) {
    if (argc != 2) return failure(SYNTAX, argv[0], "<dispositivo>");
    if (mount(argv[1]) < 0) return failure(MOUNT, argv[1]);

    struct SuperBlock sb;

#if DEBUG_SUPER_BLOCK
    if (print_super_block(&sb) < 0) return EXIT_FAILURE;
#endif
#if DEBUG_INODES
    if (print_inodes(&sb) < 0) return EXIT_FAILURE;
#endif
#if DEBUG_BITMAP
    if (print_bitmap(&sb) < 0) return EXIT_FAILURE;
#endif
#if DEBUG_BLOCKS
    if (print_blocks(&sb) < 0) return EXIT_FAILURE;
#endif
#if DEBUG_ROOT_INODE
    if (print_root_inode(&sb) < 0) return EXIT_FAILURE;
#endif
#if DEBUG_BLOCK_TRANSLATION
    if (print_logical_blocks_translation() < 0) return EXIT_FAILURE;
#endif
#if DEBUG_FIND_DIR
    print_find_entries();
#endif

    return umount() < 0 ? failure(MOUNT, argv[1]) : EXIT_SUCCESS;
}