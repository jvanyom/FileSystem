#include "bloques.h"

signed int failure(char *message) {
    fprintf(stderr, RED"Error: "RESET"%s\n", message);
    return EXIT_FAILURE;
}

static int descriptor = 0;

/**
 * Monta el dispositivo (abre el fichero).
 *
 * @param path Nombre o ruta del dispositivo virtual (disco).
 *
 * @return Descriptor del fichero abierto o -1 si hay error.
 */
int bmount(const char *path) {
    if ((descriptor = open(path, O_RDWR | O_CREAT)) == FAILURE) return FAILURE;

    chmod(path, 0666);

    return descriptor;
}

/**
 * Desmonta el dispositivo (cierra el fichero).
 *
 * @return 1 si se ha cerrado correctament y -1 en caso contrario.
 */
int bumount() {
    return close(descriptor) ? FAILURE : SUCCESS;
}

/**
 * Escribe un bloque físico en el dispositivo (fichero).
 *
 * @param physical_block Número del bloque físico que se quiere escribir.
 * @param buffer Contenido que se quiere escribir.
 *
 * @return Número de bytes escritos o -1 si ha habido algún error.
 */
int bwrite(unsigned int physical_block, const void *buffer) {
    if (lseek(descriptor, physical_block * BLOCK_SIZE, SEEK_SET) == FAILURE) return FAILURE;

    return (int) write(descriptor, buffer, BLOCK_SIZE);
}

/**
 * Lee un bloque físico del dispositivo (fichero).
 *
 * @param physical_block Número del bloque físico que se quiere leer.
 * @param buffer Buffer donde se quiere guardar el contenido leído.
 *
 * @return Número de bytes leídos o -1 si ha habido algún error.
 */
int bread(unsigned int physical_block, void *buffer) {
    if (lseek(descriptor, physical_block * BLOCK_SIZE, SEEK_SET) == FAILURE) return FAILURE;

    return (int) read(descriptor, buffer, BLOCK_SIZE);
}