#include "directories.h"

int main(int argc, char **argv) {
    if (argc < 3 || argc > 4) return print_error(SYNTAX, argv[0], "<dispositivo> <ruta>");

    const int is_recursive = strcmp(argv[1], "-r") == 0;

    if (argc == 4 && !is_recursive) return print_error(SYNTAX, argv[0], "<dispositivo> <ruta>");

    const char *dev = argv[is_recursive ? 2 : 1];

    if (access(dev, F_OK) != 0) return print_error(DEV_NOT_EXISTS, dev);
    if (dev_mount(dev) < 0) return print_error(MOUNT, dev);

    const int error = my_unlink(argv[is_recursive ? 3 : 2], is_recursive ? INODE_DIR : INODE_FILE, is_recursive);
    if (error < 0) return print_error(error);

    return dev_umount() < 0 ? print_unexpected() : EXIT_SUCCESS;
}