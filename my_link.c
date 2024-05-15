#include "directories.h"

int main(int argc, char **argv) {
    if (argc != 4) return print_error(SYNTAX, argv[0], "<dispositivo> <ruta objetivo> <nombre enlace>");

    if (access(argv[1], F_OK) != 0) return print_error(DEV_NOT_EXISTS, argv[1]);
    if (mount(argv[1]) < 0) return print_error(MOUNT, argv[1]);

    const int error = my_link(argv[2], argv[3]);
    if (error < 0) return print_error(error);

    return umount() < 0 ? print_unexpected() : EXIT_SUCCESS;
}