#include "../files.h"

int main(int argc, char **argv) {
    if (argc != 4) return failure("Sintaxi: permitir <dispositivo> <número i-nodo> <permisos>");

    if (mount(argv[1]) == FAILURE) {
        return failure("No ha sido posible montar correctamente el dispositivo");
    }

    const unsigned int inode_position = strtol(argv[2], NULL, 10);
    const unsigned int permissions = strtol(argv[3], NULL, 10);

    if (my_chmod(inode_position, permissions) == FAILURE) {
        return failure("No ha sido posible modificar correctamente los permisos del i-nodo");
    }

    if (umount() == FAILURE) return failure("No ha sido posible desmontar correctamente el dispositivo");

    return EXIT_SUCCESS;
}