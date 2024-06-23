#include "../directories.h"

#define PATHS_SIZE 5
#define PATHS_ACCESSES_SIZE 12

const static unsigned char paths_access[PATHS_ACCESSES_SIZE] = {1, 2, 1, 0, 4, 1, 3, 4, 2, 1, 4, 1};

const static char *paths[PATHS_SIZE] = {
        "/fic1",
        "/fic2",
        "/fic3",
        "/fic4",
        "/fic5"
};

int main(int argc, char **argv) {
    if (argc != 3) return print_error(SYNTAX, argv[0], "<dispositivo> <texto>");

    if (access(argv[1], F_OK) != 0) return print_error(DEV_NOT_EXISTS, argv[1]);
    if (dev_mount(argv[1]) < 0) return print_error(MOUNT, argv[1]);

    for (int i = 0; i < PATHS_SIZE; i++) {
        const char *path = paths[i];

        debug(1, "Se ha creado '%s'", path);
        create_entry(path, NULL, RWX, INODE_FILE);
    }

    for (int i = 0; i < PATHS_ACCESSES_SIZE; i++) {
        const char *path = paths[paths_access[i]];

        debug(1, "Se ha accedido a '%s'", path);
        my_write(path, argv[2], 0, strlen(argv[2]));
    }

    return dev_umount() < 0 ? print_unexpected() : EXIT_SUCCESS;
}