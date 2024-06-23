#include <time.h>
#include <string.h>

#include "errors.h"

#define MAX_CACHE_SIZE 3

#define SLASH '/'
#define EOL '\0'

#define FIFO 1
#define LRU 2

#define LRU_PREFIX MAGENTA"(LRU): "LIGHT_GRAY

#define CACHE LRU

#define DEBUG_CACHE 0

typedef struct {
    int inode_position;
    char *filename;
} dcache_dentry_t;

typedef struct dentry_node_t {
    struct dentry_node_t *prev;
    struct dentry_node_t *next;

    dcache_dentry_t entry;
} dentry_node_t;

typedef struct {
    dentry_node_t *head;
    dentry_node_t *tail;

    unsigned int size;
} dcache_t;

/**
 * Insertar una entrada de fichero en la caché si no existe.
 * Si el algoritmo usado es LRU, se mueve al frente de la caché si existe.
 * Si el algoritmo es FIFO, se inserta por la cola.
 *
 * @param path Ruta del archivo que se quiere guardar en caché.
 * @param inode_position Posición del i-nodo del fichero que se quiere guardar en caché.
 *
 * @return 0. Puede devolver error.
 */
int dcache_set(const char *path, int inode_position);

/**
 * Obtener de la cache el número de i-nodo en base al nombre del fichero.
 *
 * @param path Nombre de fichero.
 *
 * @return Posición del i-nodo.
 */
int dcache_get(const char *path);