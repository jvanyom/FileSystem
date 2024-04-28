#include "directories.h"

int main(int argc, char **argv) {
    if (argc != 3) return print_error(SYNTAX, argv[0], "<disco> <ruta>");

    if (access(argv[1], F_OK) != 0) return print_error(DEV_NOT_EXISTS, argv[1]);
    if (mount(argv[1]) < 0) return print_error(SYNTAX, argv[1]);

    struct Metadata metadata;
    const int inode_position = my_stat(argv[2], &metadata);
    if (inode_position < 0) return print_error(inode_position);

    printf("I-Nodo: %d\n", inode_position);
    print_inode(&metadata, NULL);

    return EXIT_SUCCESS;
}