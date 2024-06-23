#include "../files.h"

#define READ_BUFFER_LENGTH BLOCK_SIZE

int main(int argc, char **argv) {
    if (argc != 3) return print_error(SYNTAX, argv[0], "<dispositivo> <número de i-nodo>");
    if (dev_mount(argv[1]) < 0) return print_error(MOUNT, argv[1]);

    const unsigned int inode_position = strtol(argv[2], NULL, 10);
    if (errno == EINVAL) return print_error(NAN, "número de i-nodo");

    inode_t inode;
    if (read_inode(inode_position, &inode) < 0) {
        return print_cerror("No ha sido posible leer correctamente el i-nodo %d", inode_position);
    }

    unsigned int read_bytes = 0;
    unsigned int offset = 0;

    unsigned char buffer[READ_BUFFER_LENGTH] = {0};

    while (1) {
        const int current_read_bytes = my_read_file(inode_position, buffer, offset, READ_BUFFER_LENGTH);
        if (current_read_bytes == 0) break;
        if (current_read_bytes < 0) {
            return print_cerror("No ha sido posible leer correctamente el contenido del i-nodo %d", inode_position);
        }

        if (write(STDOUT_FILENO, buffer, current_read_bytes) < 0) {
            return print_cerror("No ha sido posible mostrar el contenido del i-nodo %d", inode_position);
        }

        read_bytes += current_read_bytes;
        offset += READ_BUFFER_LENGTH;
        memset(buffer, EMPTY_BYTE, READ_BUFFER_LENGTH);
    }

    fprintf(stderr, "\nBytes leídos: %d\n", read_bytes);
    fprintf(stderr, "Tamaño en bytes lógicos: %d\n", inode.metadata.size);

    return dev_umount() < 0 ? print_error(MOUNT, argv[1]) : EXIT_SUCCESS;
}