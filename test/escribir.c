#include "../files.h"

const static int offsets[] = {9000, 209000, 30725000, 409605000, 480000000};

int main(int argc, char **argv) {
    if (argc != 4) return print_error(SYNTAX, argv[0], "<dispositivo> <datos> <número de i-nodos diferentes>");
    if (dev_mount(argv[1]) < 0) return print_error(MOUNT, argv[1]);

    const unsigned int inodes_num = strtol(argv[3], NULL, 10);
    if (errno == EINVAL) return print_error(NAN, "número de i-nodos diferentes");

    const unsigned int buffer_length = strlen(argv[2]);

    printf("Longitud del texto: %d\n", buffer_length);

    int inode_position = inodes_num == 0 ? reserve_inode(INODE_FILE, RW) : 0;
    if (inode_position < 0) return print_cerror("No ha sido posible reservar un nuevo i-nodo");

    metadata_t metadata;

    for (int i = 0; i < sizeof(offsets) / sizeof(offsets[0]); ++i) {
        if (inodes_num == 1) {
            inode_position = reserve_inode(INODE_FILE, RW);
            if (inode_position < 0) return print_cerror("No ha sido posible reservar un nuevo i-nodo");
        }

        const int wrote_bytes = my_write_file(inode_position, argv[2], offsets[i], buffer_length);
        if (wrote_bytes < 0) {
            return print_cerror("No ha sido posible escribir el contenido en el i-nodo %d", inode_position);
        }

        if (my_stat_file(inode_position, &metadata) < 0) {
            return print_cerror("No ha sido posible leer correctamente los metadatos del i-nodo %d", inode_position);
        }

        printf("I-nodo reservado: %d\n", inode_position);
        printf("Offset: %d\n", offsets[i]);
        printf("Bytes escritos: %d\n", wrote_bytes);
        printf("Tamaño en bytes lógicos: %d\n", metadata.size);
        printf("Bloques ocupados: %d\n", metadata.busy_blocks_count);
        printf("--------------------------------------------------------\n");
    }

    return dev_umount() < 0 ? print_error(MOUNT, argv[1]) : EXIT_SUCCESS;
}