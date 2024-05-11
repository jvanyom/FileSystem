#include "../directories.h"

#define BATCH_SIZE 10

int main(int argc, char **argv) {
    if (argc != 5) return print_error(SYNTAX, argv[0], "<dispositivo> <ruta> <datos> <offset>");

    const int offset = (int) strtol(argv[4], NULL, 10);
    if (errno == EINVAL) return print_error(NAN, "offset");

    if (access(argv[1], F_OK) != 0) return print_error(DEV_NOT_EXISTS, argv[1]);
    if (mount(argv[1]) < 0) return print_error(MOUNT, argv[1]);

    const size_t count = strlen(argv[3]);

    //obtenemos la ruta y comprobamos que no se refiera a un directorio
//    if (argv[2][strlen(argv[2]) - 1] == '/') {
//        fprintf(stderr, RED "Error: la ruta se corresponde a un directorio.\n" RESET);
//        exit(-1);
//    }

    int wrote_bytes = 0;

    debug(1, "Longitud del texto: %zu", count);

    for (int i = 0; i < BATCH_SIZE; i++) {
        wrote_bytes += my_write(argv[2], argv[3], offset + BLOCK_SIZE * i, count);
    }

    debug(1, "Bytes escritos: %d", wrote_bytes);

    /* Visualización del stat
    mi_stat_f(ninodo, &stat);
    printf("stat.tamEnBytesLog=%d\n",stat.tamEnBytesLog);
    printf("stat.numBloquesOcupados=%d\n",stat.numBloquesOcupados);
    */

    return umount() < 0 ? print_unexpected() : EXIT_SUCCESS;
}
