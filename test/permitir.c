#include "../files.h"

int main(int argc, char **argv) {
    if (argc != 4) return print_error(SYNTAX, argv[0], "<dispositivo> <número de i-nodo> <permisos>");
    if (mount(argv[1]) < 0) return print_error(MOUNT, argv[1]);

    const unsigned int inode_position = strtol(argv[2], NULL, 10);
    if (errno == EINVAL) return print_error(NAN, "número de i-nodo");

    const unsigned int permissions = strtol(argv[3], NULL, 10);
    if (errno == EINVAL) return print_error(NAN, "permisos");

    if (my_chmod_file(inode_position, permissions) < 0) {
        return print_cerror("No ha sido posible modificar correctamente los permisos del i-nodo");
    }

    return umount() < 0 ? print_error(MOUNT, argv[1]) : EXIT_SUCCESS;
}