#include "../files.h"

int main(int argc, char **argv) {
    if (argc != 4) return failure(SYNTAX, argv[0], "<dispositivo> <número de i-nodo> <cantidad de bytes>");
    if (mount(argv[1]) < 0) return failure(MOUNT, argv[1]);

    const unsigned int inode_position = strtol(argv[2], NULL, 10);
    if (errno == EINVAL) return failure(NAN, "número de i-nodo");

    const unsigned int count = strtol(argv[3], NULL, 10);
    if (errno == EINVAL) return failure(NAN, "cantidad de bytes");

    if (count == 0) {
        if (free_inode(inode_position) < 0) {
            return f("No ha sido posible liberar correctamente el i-nodo %d", inode_position);
        }
    } else {
        if (my_trunc(inode_position, count) < 0) {
            return f("No ha sido posible truncar correctamente el i-nodo %d", inode_position);
        }
    }

    struct Metadata metadata;
    if (my_stat(inode_position, &metadata) < 0) {
        return f("No ha sido posible leer correctamente el i-nodo %d", inode_position);
    }

    char string[64];
    sprintf(string, "Metadatos i-nodo %d", inode_position);
    print_inode(&metadata, string);

    return umount() < 0 ? failure(MOUNT, argv[1]) : EXIT_SUCCESS;
}