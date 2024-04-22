#include "files.h"

#define ENTRY_SIZE sizeof(struct Entry)
#define ENTRIES_PER_BLOCK (BLOCK_SIZE / ENTRY_SIZE)
#define FILENAME_SIZE 60

#define EMPTY_STR ""
#define SLASH_STR "/"

#define SLASH '/'
#define EOL '\0'

struct Entry {
    char filename[FILENAME_SIZE];
    unsigned int inodePosition;
};

/**
 * Separar el 'path' en dos: 'initial' y 'final'. Donde 'initial' es la parte del 'path' comprendida entre las dos
 * primeras barras (/) y 'final' es el resto del 'path'.
 *
 *
 * @param path Ruta del fichero.
 * @param initial Buffer para almacenar la parte inicial.
 * @param final Buffer para almacenar la parte final.
 * @param type Tipo de fichero.
 *
 * @return 0. Puede devolver error.
 */
int split_path(const char *path, char *initial, char *final, char *type);

/**
 * Buscar una determinada entrada entre todas las entradas del i-nodo correspondiente a su directorio padre.
 *
 * @param partial_path Ruta parcial al fichero. Siempre empieza con la ruta absoluta.
 * @param parent_inode_position Posición del i-nodo del directorio padre.
 * @param entry_inode_position Puntero donde se almacena la posición del i-nodo de la entrada.
 * @param entry_position Puntero donde se almacena la posición de la entrada.
 * @param reserve 0 permite consultar y 1 permite consultar y crear nueva nueva entrada.
 * @param permissions Permisos.
 *
 * @return 0. Puede devolver error.
 */
int find_entry(const char *partial_path, unsigned int parent_inode_position, char reserve, unsigned char permissions);