#include "directories.h"

int main(int argc, char **argv) {
    if (argc != 4) return print_error(SYNTAX, argv[0], "<dispositivo> <permisos> <ruta>");

    if (access(argv[1], F_OK) != 0) return print_error(DEV_NOT_EXISTS, argv[1]);
    if (mount(argv[1]) < 0) return print_error(MOUNT, argv[1]);

    const char permissions = (char) strtol(argv[2], NULL, 10);
    if (errno == EINVAL) return print_error(NAN, "permisos");
    if (!are_valid(permissions)) return print_error(NOT_VALID_PERMISSIONS);

    const int inode_position = my_chmod(argv[3], permissions);

    return inode_position < 0 ? print_error(inode_position) : EXIT_SUCCESS;
}