#include "../files.h"

#define OFFSETS 5

const static int offsets[OFFSETS] = {9000, 209000, 30725000, 409605000, 480000000};

int main(int argc, char **argv) {
    if (argc != 4) return failure("Sintaxi: escribir <dispositivo> <datos> <i-nodos diferentes>");

    if (mount(argv[1]) == FAILURE) {
        return failure("No ha sido posible montar correctamente el dispositivo");
    }

    struct Metadata metadata;

    const unsigned int inodes_num = strtol(argv[3], NULL, 10);

    const unsigned int buffer_length = strlen(argv[2]);

    printf("Longitud del texto: %d\n", buffer_length);

    unsigned int inode_position = inodes_num == 0 ? reserve_inode(FILE_INODE, RW) : 0;

    for (int i = 0; i < OFFSETS; ++i) {
        if (inodes_num == 1) inode_position = reserve_inode(FILE_INODE, RW);

        const signed int wrote_bytes = my_write(inode_position, argv[2], offsets[i], buffer_length);

        if (my_stat(inode_position, &metadata) == FAILURE) {
            return failure("No ha sido posible leer correctamente los metadatos del i-nodo");
        }

        printf("--------------------------------------------------------\n");
        printf("I-nodo reservado: %d\n", inode_position);
        printf("Offset: %d\n", offsets[i]);
        printf("Bytes escritos: %d\n", wrote_bytes);
        printf("Tamaño en bytes lógicos: %d\n", metadata.logicalBytesSize);
        printf("Bloques ocupados: %d\n", metadata.totalBusyBlocks);
        printf("--------------------------------------------------------\n");
    }

    if (umount() == FAILURE) return failure("No ha sido posible desmontar correctamente el dispositivo");

    return EXIT_SUCCESS;
}