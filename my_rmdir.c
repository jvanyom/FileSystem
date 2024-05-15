#include "directories.h"

int main(int argc, char **argv) {
    if (argc != 3) return print_error(SYNTAX, argv[0], "<dispositivo> <ruta>");

    if (access(argv[1], F_OK) != 0) return print_error(DEV_NOT_EXISTS, argv[1]);
    if (mount(argv[1]) < 0) return print_error(SYNTAX, argv[1]);

    const int error = my_unlink(argv[2], INODE_DIR);
    if (error < 0) return print_error(error);

    return umount() < 0 ? print_unexpected() : EXIT_SUCCESS;
}