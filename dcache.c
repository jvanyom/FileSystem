#include "dcache.h"
#include "basic_files.h"

static dcache_t cache = {0};

int inline dcache_set(const char *path, int inode_position) {
    if (dcache_get(path) >= 0) return SUCCESS;

    dentry_node_t *new_node = (dentry_node_t *) malloc(sizeof(dentry_node_t));
    if (!new_node) return FAILURE;

    const size_t filename_length = strlen(path) + 1;

    new_node->entry.filename = (char *) malloc(filename_length);
    if (!new_node->entry.filename) {
        free(new_node);
        return FAILURE;
    }

    strncpy(new_node->entry.filename, path, filename_length);
    new_node->entry.inode_position = inode_position;
    new_node->prev = NULL;
    new_node->next = NULL;

    if (cache.head) {
#if CACHE == LRU
        new_node->next = cache.head;
        cache.head->prev = new_node;
        cache.head = new_node;

        debug(DEBUG_CACHE, LRU_PREFIX"Se ha añadido un nuevo registro en la cima '%s'", cache.head->entry.filename);
#else
        cache.tail->next = new_node;
        new_node->prev = cache.tail;
        cache.tail = new_node;

        debug(DEBUG_CACHE, "Se ha añadido un nuevo registro en la cola '%s'", new_node->entry.filename);
#endif
    } else {
        new_node->next = NULL;
        cache.head = new_node;
        cache.tail = new_node;

        debug(DEBUG_CACHE, "Añadimos el primer elemento '%s'", new_node->entry.filename);
    }

    if (cache.size == MAX_CACHE_SIZE) {
#if CACHE == LRU
        dentry_node_t *tail = cache.tail;
        cache.tail = cache.tail->prev;
        cache.tail->next = NULL;

        debug(DEBUG_CACHE, LRU_PREFIX"Se ha eliminado de la caché '%s'", tail->entry.filename);

        free(tail->entry.filename);
        free(tail);
#else
        dentry_node_t *head = cache.head;
        cache.head = cache.head->next;
        cache.head->prev = NULL;

        debug(DEBUG_CACHE, "Se ha eliminado de la caché '%s'", head->entry.filename);

        free(head->entry.filename);
        free(head);
#endif
    } else {
        cache.size++;
        debug(DEBUG_CACHE, "Tamaño caché: %d", cache.size);
    }

    return SUCCESS;
}


int inline dcache_get(const char *path) {
    dentry_node_t *current = cache.head;

    while (current != NULL) {
        if (strcmp(current->entry.filename, path) == 0) {
            debug(DEBUG_CACHE, "Usamos la entrada de '%s'", path);

#if CACHE == LRU
            if (current != cache.head) {
                if (current == cache.tail) {
                    cache.tail = current->prev;
                    cache.tail->next = NULL;
                } else {
                    current->prev->next = current->next;
                }

                current->prev = NULL;
                current->next = cache.head;
                cache.head->prev = current;

                cache.head = current;
            }

            debug(DEBUG_CACHE, LRU_PREFIX"Se ha movido a la cima '%s'", cache.head->entry.filename);
#endif
            return (int) current->entry.inode_position;
        }

        current = current->next;
    }

    return FAILURE;
}
