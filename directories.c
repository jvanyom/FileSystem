#include "directories.h"

/**
 * Separar el 'path' en dos: 'initial' y 'final'. Donde 'initial' es la parte del 'path' comprendida entre las dos
 * primeras barras (/) y 'final' es el resto del 'path'.
 *
 *
 * @param path Ruta del fichero.
 * @param initial Buffer para almacenar la parte inicial.
 * @param final Buffer para almacenar la parte final.
 *
 * @return 0. Puede devolver error.
 */
int split_path(const char *path, char *initial, char *final) {
    if (*path != SLASH) return FILE_NOT_EXISTS;

    const char *no_slash_path = path + 1;
    const char *slash = strchr(no_slash_path, SLASH);

    if (slash) {
        int length = (int) (slash - no_slash_path);

        strncpy(initial, no_slash_path, length);
        initial[length] = EOL;

        strcpy(final, slash);
    } else {
        strcpy(initial, no_slash_path);
        strcpy(final, EMPTY_STR);
    }

    return SUCCESS;
}

/**
 * Buscar una determinada entrada entre todas las entradas del i-nodo correspondiente a su directorio padre.
 *
 * @param path Ruta parcial al fichero. Es una ruta absoluta.
 * @param parent_inode_position Posición del i-nodo del directorio padre.
 * @param entry_position Posición de la entrada consultada.
 * @param create 0 permite consultar y 1 permite consultar y crear nueva nueva entrada.
 * @param permissions Permisos que se le asignan al fichero en el caso que 'reserve = 1'.
 * @param type Tipo de entrada que tiene que crear. INODE_DIR o INODE_FILE.
 *
 * @return Posición del i-nodo encontrado o creado. Puede devolver error.
 */
int find_entry(const char *path, unsigned int *parent_inode_position, unsigned int *entry_position,
               unsigned char create, unsigned char permissions, unsigned char type) {
    if (strcmp(path, SLASH_STR) == 0) return create ? FILE_ALREADY_EXISTS : (int) *parent_inode_position;

    char initial_path[FILENAME_SIZE] = {0};
    char final_path[strlen(path)];

    if (split_path(path, initial_path, final_path) < 0) return FILE_NOT_EXISTS;

    debug("Inicial: %s | Final: %s | Crear: %d", initial_path, final_path, create);

    struct INode parent_inode;
    if (read_inode(*parent_inode_position, &parent_inode) < 0) return FAILURE;

    struct Entry entries[ENTRIES_PER_BLOCK];

    for (int block = 0; block < parent_inode.metadata.busyBlocksCount; ++block) {
        int read_bytes = my_read_file(*parent_inode_position, entries, block * BLOCK_SIZE, BLOCK_SIZE);
        if (read_bytes < 0) return read_bytes;

        for (int entry = 0; entry < read_bytes / ENTRY_SIZE; ++entry) {
            struct Entry *e = entries + entry;

            if (strcmp(e->filename, initial_path) == 0) {
                if (strcmp(final_path, EMPTY_STR) == 0 || strcmp(final_path, SLASH_STR) == 0) {
                    if (entry_position) *entry_position = entry;

                    return create ? FILE_ALREADY_EXISTS : (int) e->inodePosition;
                }

                *parent_inode_position = e->inodePosition;

                return find_entry(final_path, parent_inode_position, entry_position, create, permissions, type);
            }
        }
    }

    const unsigned int bad_path = create == NO_CREATE ||
                                  (type == INODE_FILE && strcmp(final_path, SLASH_STR) == 0) ||
                                  (strcmp(final_path, EMPTY_STR) != 0 && strcmp(final_path, SLASH_STR) != 0);

    if (bad_path) return FILE_NOT_EXISTS;

    if (parent_inode.metadata.type == INODE_FILE) return IS_FILE;
    if ((parent_inode.metadata.permissions & READ) == 0) return NO_READ_PERMISSIONS;
    if ((parent_inode.metadata.permissions & WRITE) == 0) return NO_WRITE_PERMISSIONS;

    int new_inode_position = reserve_inode(type, permissions);
    if (new_inode_position < 0) return new_inode_position;

    debug("Se ha reservado el i-nodo %d tipo '%c' con permisos %d para %s", new_inode_position, type, RW, initial_path);

    struct Entry new_entry = {0};
    strcpy(new_entry.filename, initial_path);
    new_entry.inodePosition = new_inode_position;

    int wrote_bytes = my_write_file(
            *parent_inode_position,
            &new_entry,
            parent_inode.metadata.size,
            ENTRY_SIZE
    );

    if (wrote_bytes < 0 && free_inode(new_entry.inodePosition) < 0) return FAILURE;

    debug("Se ha creado la entrada: '%s' | %d", new_entry.filename, new_entry.inodePosition);
    return new_inode_position;
}

int get_inode(const char *path, unsigned int *parent_inode_position, unsigned int *entry_position) {
    struct SuperBlock sb;
    if (read_block(SUPER_BLOCK_POSITION, &sb) < 0) return FAILURE;

    if (parent_inode_position) *parent_inode_position = sb.rootINode;

    return find_entry(
            path,
            parent_inode_position ?: &sb.rootINode,
            entry_position,
            NO_CREATE,
            0, 0
    );
}

int create_entry(const char *path, unsigned char permissions, unsigned char type) {
    struct SuperBlock sb;
    if (read_block(SUPER_BLOCK_POSITION, &sb) < 0) return FAILURE;

    return find_entry(path, &sb.rootINode, NULL, CREATE, permissions, type);
}

int my_chmod(const char *path, unsigned char new_permissions) {
    int inode_position = get_inode(path, NULL, NULL);
    if (inode_position < 0) return inode_position;

    return my_chmod_file(inode_position, new_permissions) < 0 ? FAILURE : inode_position;
}

int my_stat(const char *path, struct Metadata *metadata) {
    int inode_position = get_inode(path, NULL, NULL);
    if (inode_position < 0) return inode_position;

    return my_stat_file(inode_position, metadata) < 0 ? FAILURE : inode_position;
}