#include "../ficheros.h"

#define READ_BUFFER_LENGTH (BLOCK_SIZE * 4)

int main(int argc, char **argv) {
    if (argc != 3) return failure("Sintaxi: leer <dispositivo> <número i-nodo>");

    if (bmount(argv[1]) == FAILURE) {
        return failure("No ha sido posible montar correctamente el dispositivo");
    }

    const unsigned int inode_position = strtol(argv[2], NULL, 10);

    struct INode inode;

    if (leer_inodo(inode_position, &inode) == FAILURE) {
        return failure("No ha sido posible leer correctamente el i-nodo especificado");
    }

    unsigned char buffer[READ_BUFFER_LENGTH] = {0};

    unsigned int read_bytes = 0;
    unsigned int offset = 0;

    while (1) {
        const signed int current_read_bytes = mi_read_f(inode_position, buffer, offset, READ_BUFFER_LENGTH);

        if (current_read_bytes == 0) break;
        if (current_read_bytes == FAILURE) {
            return failure("No ha sido posible leer correctamente el contenido de datos del i-nodo");
        }

        write(STDOUT_FILENO, buffer, current_read_bytes);

        read_bytes += current_read_bytes;
        offset += READ_BUFFER_LENGTH;
        memset(buffer, 0, READ_BUFFER_LENGTH);
    }

    fprintf(stderr, "\nBytes leídos: %d\n", read_bytes);
    fprintf(stderr, "Tamaño en bytes lógicos: %d\n", inode.logicalBytesSize);

    if (bumount() == FAILURE) return failure("No ha sido posible desmontar correctamente el dispositivo");

    return EXIT_SUCCESS;
}