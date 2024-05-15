#include "directories.h"

#define BUFFER_SIZE (BLOCK_SIZE * 4)

int main(int argc, char **argv) {
    if (argc != 3) return print_error(SYNTAX, argv[0], "<dispositivo> <ruta>");

    if (access(argv[1], F_OK) != 0) return print_error(DEV_NOT_EXISTS, argv[1]);
    if (mount(argv[1]) < 0) return print_error(MOUNT, argv[1]);

    unsigned char buffer[BUFFER_SIZE] = {0};

    unsigned int read_bytes = 0;
    unsigned int offset = 0;

    while (1) {
        const int current_read_bytes = my_read(argv[2], buffer, offset, BUFFER_SIZE);
        if (current_read_bytes < 0) return print_error(current_read_bytes);
        if (current_read_bytes == 0) break;

        if (write(STDOUT_FILENO, buffer, current_read_bytes) < 0) return print_unexpected();

        read_bytes += current_read_bytes;
        offset += BUFFER_SIZE;
        memset(buffer, EMPTY_BYTE, BUFFER_SIZE);
    }

    fprintf(stderr, "\n%d bytes leídos\n", read_bytes);

    return umount() < 0 ? print_unexpected() : EXIT_SUCCESS;
}