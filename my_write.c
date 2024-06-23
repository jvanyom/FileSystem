#include "directories.h"

int main(int argc, char **argv) {
    if (argc != 5) return print_error(SYNTAX, argv[0], "<dispositivo> <ruta> <datos> <offset>");

    const int offset = (int) strtol(argv[4], NULL, 10);
    if (errno == EINVAL) return print_error(NAN, "offset");

    if (access(argv[1], F_OK) != 0) return print_error(DEV_NOT_EXISTS, argv[1]);
    if (dev_mount(argv[1]) < 0) return print_error(MOUNT, argv[1]);

    const int wrote_bytes = my_write(argv[2], argv[3], offset, strlen(argv[3]));

    if (wrote_bytes < 0) return print_error(wrote_bytes);

    fprintf(stderr, "\n%d bytes escritos\n", wrote_bytes);

    return dev_umount() < 0 ? print_unexpected() : EXIT_SUCCESS;
}