#include "../directories.h"

#define DEBUG_SUPER_BLOCK 1
#define DEBUG_ALL_INODES 0
#define DEBUG_BITMAP 0
#define DEBUG_BLOCKS 0
#define DEBUG_ROOT_INODE 0
#define DEBUG_BLOCK_TRANSLATION 0
#define DEBUG_FIND_DIR 0

int print_super_block(super_block_t *sb) {
    if (read_block(SUPER_BLOCK_POSITION, sb) < 0) {
        return print_cerror("No ha sido posible leer correctamente el súper bloque");
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
            sb->bitmap_first_block, sb->bitmap_last_block,
            sb->inodes_first_block, sb->inodes_last_block,
            sb->data_first_block, sb->data_last_block,
            sb->root_inode, sb->next_free_inode_position,
            sb->free_blocks_count, sb->free_inodes_count,
            sb->blocks_count, sb->inodes_count
    );

    printf("Tamaño súper bloque: %lu\n", sizeof(super_block_t));
    printf("Tamaño i-nodo: %lu\n", sizeof(inode_t));

    return SUCCESS;
}

int print_inodes(super_block_t *sb) {
    printf("\n*** I-NODOS ***\n");

    inode_t inodes[INODES_PER_BLOCK];

    for (unsigned int i = sb->inodes_first_block; i <= sb->inodes_last_block; ++i) {
        if (read_block(i, &inodes) < 0) {
            return print_cerror("No ha sido posible leer correctamente el bloque físico %d", i);
        }

        for (unsigned int j = 0; j < INODES_PER_BLOCK; ++j) {
            printf("%d, ", inodes[j].direct[0]);
        }

        printf("\n");
    }

    return SUCCESS;
}

int print_block_state(unsigned int block) {
    const unsigned char state = get_block_state(block);
    if (state < 0) return print_cerror("No ha sido posible leer correctamente el estado del bloque %d", block);

    printf("Bloque: %d | Estado: %d\n", block, state);

    return SUCCESS;
}

int print_bitmap(super_block_t *sb) {
    printf("\n*** BITMAP ***\n");

    return (
            print_block_state(SUPER_BLOCK_POSITION) < 0 ||
            print_block_state(sb->bitmap_first_block) < 0 ||
            print_block_state(sb->bitmap_last_block) < 0 ||
            print_block_state(sb->inodes_first_block) < 0 ||
            print_block_state(sb->inodes_last_block) < 0 ||
            print_block_state(sb->data_first_block) < 0 ||
            print_block_state(sb->data_last_block) < 0
    );
}

int print_blocks(super_block_t *sb) {
    printf("\n*** RESERVA Y LIBERACIÓN DE BLOQUES ***\n");

    const int block = reserve_block();
    if (block < 0) return print_cerror("No ha sido posible reservar correctamente un bloque nuevo");

    printf("Se ha reservado el bloque físico %d.\n", block);

    if (read_block(SUPER_BLOCK_POSITION, sb) < 0) {
        return print_cerror("No ha sido posible leer correctamente el súper bloque");
    }

    printf("Bloques libres: %d\n", sb->free_blocks_count);

    if (free_block(block) < 0) {
        return print_cerror("No ha sido posible liberar correctamente el bloque %d", block);
    }

    printf("Se ha liberado el bloque físico %d.\n", block);

    if (read_block(SUPER_BLOCK_POSITION, sb) < 0) {
        return print_cerror("No ha sido posible leer correctamente el súper bloque");
    }

    printf("Bloques libres: %d\n", sb->free_blocks_count);

    return SUCCESS;
}

int print_root_inode(super_block_t *sb) {
    inode_t root;

    if (read_inode(sb->root_inode, &root) < 0) {
        return print_cerror("No ha sido posible leer correctamente el i-nodo raíz (%d)", sb->root_inode);
    }

    return print_inode(&root.metadata, "RAÍZ");
}

int print_logical_blocks_translation() {
    printf("\n*** TRADUCCIÓN DE BLOQUES LÓGICOS ***\n");

    const static unsigned int logical_blocks[] = {8, 204, 30004, 400004, 468750};

    const int inode_position = reserve_inode(INODE_FILE, READ | WRITE);
    if (inode_position < 0) return print_cerror("No ha sido posible reservar un nuevo i-nodo");

    inode_t inode;
    if (read_inode(inode_position, &inode) < 0) {
        return print_cerror("No ha sido posible leer correctamente el i-nodo %d", inode_position);
    }

    for (int i = 0; i < sizeof(logical_blocks) / sizeof(unsigned int); ++i) {
        if (get_physical_block(&inode, logical_blocks[i], RESERVE) < 0) {
            return print_cerror(
                    "No ha sido posible obtener correctamente el número bloque físico del bloque lógico %d",
                    logical_blocks[i]
            );
        }
    }

    return print_inode(&inode.metadata, "I-NODO RESERVADO");
}

int print_find_entry(char *path, unsigned char create, unsigned char type) {
    printf("\nRuta: %s | Crear: %d\n", path, create);
    printf("\n********************************************************************\n");

    const int inode_position = create
                               ? create_entry(path, RW, type)
                               : get_inode(path, NULL, NULL);

    if (inode_position < 0) return print_error(inode_position);

    return SUCCESS;
}

int print_find_entries() {
    print_find_entry("pruebas/", CREATE, INODE_DIR);                    // BAD_PATH
    print_find_entry("/pruebas/", NO_CREATE, INODE_DIR);                // BAD_PATH
    print_find_entry("/pruebas/docs/", CREATE, INODE_DIR);              // BAD_PATH
    print_find_entry("/pruebas/", CREATE, INODE_DIR);                   // Se crea /pruebas/
    print_find_entry("/pruebas/docs/", CREATE, INODE_DIR);              // Se crea /pruebas/docs/
    print_find_entry("/pruebas/docs/doc1", CREATE, INODE_FILE);         // Se crea /pruebas/docs/doc1
    print_find_entry("/pruebas/docs/doc1/doc11", CREATE, INODE_FILE);   // IS_FILE
    print_find_entry("/pruebas/", CREATE, INODE_DIR);                   // FILE_ALREADY_EXISTS
    print_find_entry("/pruebas/docs/doc1", NO_CREATE, INODE_FILE);      // Se consulta /pruebas/docs/doc1
    print_find_entry("/pruebas/docs/doc1", CREATE, INODE_FILE);         // FILE_ALREADY_EXISTS
    print_find_entry("/pruebas/casos/", CREATE, INODE_DIR);             // Se crea /pruebas/casos/
    print_find_entry("/pruebas/docs/doc2", CREATE, INODE_FILE);         // Se crea /pruebas/docs/doc2

    return SUCCESS;
}

int main(int argc, char **argv) {
    if (argc != 2) return print_error(SYNTAX, argv[0], "<dispositivo>");
    if (mount(argv[1]) < 0) return print_error(MOUNT, argv[1]);

    super_block_t sb;

#if DEBUG_SUPER_BLOCK
    if (print_super_block(&sb) < 0) return EXIT_FAILURE;
#endif
#if DEBUG_ALL_INODES
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

    return umount() < 0 ? print_error(MOUNT, argv[1]) : EXIT_SUCCESS;
}