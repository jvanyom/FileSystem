#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "errors.h"

#define BLOCK_SIZE 1024

/**
 * Montar dispositivo.
 *
 * @param path Nombre o ruta del dispositivo.
 *
 * @return Descriptor del fichero abierto. Puede devolver error.
 */
int mount(const char *path);

/**
 * Desmontar dispositivo.
 *
 * @return 1 si se ha cerrado correctament. Puede devolver error.
 */
int umount();

/**
 * Escribir bloque físico en el dispositivo.
 *
 * @param physical_block Número del bloque físico que se quiere escribir.
 * @param buffer Contenido que se quiere escribir.
 *
 * @return Número de bytes escritos. Puede devolver error.
 */
int write_block(unsigned int physical_block, const void *buffer);

/**
 * Leer bloque físico del dispositivo.
 *
 * @param physical_block Número del bloque físico que se quiere leer.
 * @param buffer Buffer donde se quiere guardar el contenido leído.
 *
 * @return Número de bytes leídos. Puede devolver error.
 */
int read_block(unsigned int physical_block, void *buffer);