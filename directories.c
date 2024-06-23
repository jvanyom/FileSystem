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
 * @param create 0 permite consultar y 1 permite crear nueva nueva entrada.
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

    debug(DEBUG_ENTRIES, "Inicial: %s | Final: %s | Crear: %d", initial_path, final_path, create);

    inode_t parent_inode;
    if (read_inode(*parent_inode_position, &parent_inode) < 0) return FAILURE;

    dentry_t entries[ENTRIES_PER_BLOCK];

    for (int block = 0; block < parent_inode.metadata.busy_blocks_count; ++block) {
        int read_bytes = my_read_file(
                *parent_inode_position,
                entries,
                block * BLOCK_SIZE,
                BLOCK_SIZE
        );

        if (read_bytes < 0) return read_bytes;

        for (int entry = 0; entry < read_bytes / ENTRY_SIZE; ++entry) {
            dentry_t *e = entries + entry;

            if (strcmp(e->filename, initial_path) != 0) continue;

            if (strcmp(final_path, EMPTY_STR) == 0 || strcmp(final_path, SLASH_STR) == 0) {
                if (create) return FILE_ALREADY_EXISTS;
                if (entry_position) *entry_position = entry;

                return (int) e->inode_position;
            }

            *parent_inode_position = e->inode_position;

            return find_entry(final_path, parent_inode_position, entry_position, create, permissions, type);
        }
    }

    const int bad_path = create == NO_CREATE
                         || (type == INODE_FILE && strcmp(final_path, SLASH_STR) == 0)
                         || (strcmp(final_path, EMPTY_STR) != 0 && strcmp(final_path, SLASH_STR) != 0);

    if (bad_path) return FILE_NOT_EXISTS;

    if (parent_inode.metadata.type == INODE_FILE) return IS_FILE;
    if ((parent_inode.metadata.permissions & READ) == 0) return NOT_READ_PERMISSIONS;
    if ((parent_inode.metadata.permissions & WRITE) == 0) return NOT_WRITE_PERMISSIONS;

    int new_inode_position = reserve_inode(type, permissions);
    if (new_inode_position < 0) return new_inode_position;

    debug(DEBUG_ENTRIES,
          "Se ha reservado el i-nodo %d tipo '%c' con permisos %d para %s",
          new_inode_position, type, RW, initial_path
    );

    dentry_t new_entry = {0};
    strcpy(new_entry.filename, initial_path);
    new_entry.inode_position = new_inode_position;

    int wrote_bytes = my_write_file(
            *parent_inode_position,
            &new_entry,
            parent_inode.metadata.size,
            ENTRY_SIZE
    );

    if (wrote_bytes < 0 && free_inode(new_entry.inode_position) < 0) return FAILURE;

    debug(DEBUG_ENTRIES, "Se ha creado la entrada: '%s' | %d", new_entry.filename, new_entry.inode_position);
    return new_inode_position;
}

int get_inode(const char *path, unsigned int *parent_inode_position, unsigned int *entry_position) {
#ifdef CACHE
    const int dcache_inode_position = dcache_get(path);
    if (dcache_inode_position >= 0) return dcache_inode_position;
#endif

    super_block_t sb;
    if (read_block(SUPER_BLOCK_POSITION, &sb) < 0) return FAILURE;

    if (parent_inode_position) *parent_inode_position = sb.root_inode;

    const int inode_position = find_entry(
            path,
            parent_inode_position ?: &sb.root_inode,
            entry_position,
            NO_CREATE,
            0, 0
    );

#ifdef CACHE
    if (inode_position >= 0 && dcache_set(path, inode_position) < 0) return FAILURE;
#endif

    return inode_position;
}

int create_entry(const char *path, unsigned int *parent_inode_position, unsigned char permissions, unsigned char type) {
    if (parent_inode_position) {
        return find_entry(path, parent_inode_position, NULL, CREATE, permissions, type);
    }

    super_block_t sb;
    if (read_block(SUPER_BLOCK_POSITION, &sb) < 0) return FAILURE;

    return find_entry(path, &sb.root_inode, NULL, CREATE, permissions, type);
}

int my_chmod(const char *path, unsigned char new_permissions) {
    int inode_position = get_inode(path, NULL, NULL);
    if (inode_position < 0) return inode_position;

    return my_chmod_file(inode_position, new_permissions) < 0 ? FAILURE : inode_position;
}

int my_stat(const char *path, metadata_t *metadata) {
    int inode_position = get_inode(path, NULL, NULL);
    if (inode_position < 0) return inode_position;

    return my_stat_file(inode_position, metadata) < 0 ? FAILURE : inode_position;
}

int my_write(const char *path, const void *buffer, unsigned int offset, unsigned int count) {
    const int inode_position = get_inode(path, NULL, NULL);
    if (inode_position < 0) return inode_position;

    inode_t inode;
    if (read_inode(inode_position, &inode) < 0) return FAILURE;
    if (inode.metadata.type != INODE_FILE) return IS_NOT_FILE;

    return my_write_file(inode_position, buffer, offset, count);
}

int my_read(const char *path, void *buffer, unsigned int offset, unsigned int count) {
    const int inode_position = get_inode(path, NULL, NULL);
    if (inode_position < 0) return inode_position;

    inode_t inode;
    if (read_inode(inode_position, &inode) < 0) return FAILURE;
    if (inode.metadata.type != INODE_FILE) return IS_NOT_FILE;

    return my_read_file(inode_position, buffer, offset, count);
}

int read_entry(unsigned int parent_inode_position, unsigned int entry_position, dentry_t *buffer) {
    return my_read_file(
            parent_inode_position,
            buffer,
            ENTRY_SIZE * entry_position,
            ENTRY_SIZE
    );
}

int write_entry(unsigned int parent_inode_position, unsigned int entry_position, const dentry_t *buffer) {
    return my_write_file(
            parent_inode_position,
            buffer,
            ENTRY_SIZE * entry_position,
            ENTRY_SIZE
    );
}

int my_link(const char *target, const char *link) {
    const int target_inode_position = get_inode(target, NULL, NULL);
    if (target_inode_position < 0) return target_inode_position;

    inode_t target_inode;
    if (read_inode(target_inode_position, &target_inode) < 0) return FAILURE;
    if ((target_inode.metadata.permissions & READ) == 0) return NOT_READ_PERMISSIONS;
    if (target_inode.metadata.type != INODE_FILE) return IS_NOT_FILE;

    const int link_inode_position = create_entry(link, NULL, RW, INODE_FILE);
    if (link_inode_position < 0) return link_inode_position;

    unsigned int parent_inode_position;
    unsigned int entry_position;

    const int error = get_inode(link, &parent_inode_position, &entry_position);
    if (error < 0) return error;

    dentry_t dentry;
    const int read_bytes = read_entry(parent_inode_position, entry_position, &dentry);
    if (read_bytes < 0) return read_bytes;

    dentry.inode_position = target_inode_position;

    const int wrote_bytes = write_entry(parent_inode_position, entry_position, &dentry);
    if (wrote_bytes < 0) return wrote_bytes;

    const int freed_inode = free_inode(link_inode_position);
    if (freed_inode < 0) return freed_inode;

    target_inode.metadata.links_count++;
    target_inode.metadata.modified_at = time(NULL);

    if (write_inode(target_inode_position, &target_inode) < 0) return FAILURE;

    return SUCCESS;
}

int my_recursive_unlink(unsigned int inode_position, unsigned int parent_inode_position,
                        unsigned int entry_position, unsigned char type, unsigned char recursive) {
    inode_t inode;
    if (read_inode(inode_position, &inode) < 0) return FAILURE;

    if (inode.metadata.type != type) return type == INODE_DIR ? IS_FILE : IS_NOT_FILE;
    if (!recursive && inode.metadata.type == INODE_DIR && inode.metadata.size > 0) return NOT_EMPTY_DIR;

    if (type == INODE_DIR && recursive) {
        dentry_t entries[ENTRIES_PER_BLOCK];

        for (int i = 0; i < inode.metadata.busy_blocks_count; ++i) {
            int read_bytes = my_read_file(inode_position, entries, i * BLOCK_SIZE, BLOCK_SIZE);
            if (read_bytes < 0) return read_bytes;

            const unsigned int current_block_entries_count = read_bytes / ENTRY_SIZE;

            for (int e = 0; e < current_block_entries_count; ++e) {
                dentry_t *dentry = entries + e;

                inode_t recursive_dentry_inode;
                if (read_inode(dentry->inode_position, &recursive_dentry_inode) < 0) return FAILURE;

                const int error = my_recursive_unlink(
                        dentry->inode_position,
                        inode_position,
                        e,
                        recursive_dentry_inode.metadata.type,
                        RECURSIVE
                );

                if (error < 0) return error;
            }
        }
    }

    inode_t parent_inode;
    if (read_inode(parent_inode_position, &parent_inode) < 0) return FAILURE;

    const unsigned int last_entry_position = (int) parent_inode.metadata.size / ENTRY_SIZE - 1;

    if (entry_position < last_entry_position) {
        dentry_t last_entry;
        const int read_bytes = read_entry(parent_inode_position, last_entry_position, &last_entry);
        if (read_bytes < 0) return read_bytes;

        const int wrote_bytes = write_entry(parent_inode_position, entry_position, &last_entry);
        if (wrote_bytes < 0) return wrote_bytes;
    }

    const int freed_blocks = my_trunc_file(parent_inode_position, parent_inode.metadata.size - ENTRY_SIZE);
    if (freed_blocks < 0) return freed_blocks;

    inode.metadata.links_count--;

    if (inode.metadata.links_count == 0) {
        const int freed_inode = free_inode(inode_position);
        if (freed_inode < 0) return freed_inode;
    } else {
        inode.metadata.modified_at = time(NULL);
        if (write_inode(inode_position, &inode) < 0) return FAILURE;
    }

    return SUCCESS;
}

int my_unlink(const char *path, unsigned char type, unsigned char recursive) {
    if (strcmp(path, ROOT) == 0) return NOT_WRITE_PERMISSIONS;

    unsigned int parent_inode_position;
    unsigned int entry_position;

    const int inode_position = get_inode(path, &parent_inode_position, &entry_position);
    if (inode_position < 0) return inode_position;

    return my_recursive_unlink(inode_position, parent_inode_position, entry_position, type, recursive);
}

int my_rename(const char *path, const char *new_name) {
    unsigned int parent_inode_position;
    unsigned int entry_position;

    const int inode_position = get_inode(path, &parent_inode_position, &entry_position);
    if (inode_position < 0) return inode_position;

    const size_t length = strlen(new_name);
    char new_name_path[length + 2];
    new_name_path[0] = SLASH;
    strcpy(new_name_path + 1, new_name);
    new_name_path[length + 1] = EOL;

    if (find_entry(new_name_path, &parent_inode_position, NULL, NO_CREATE, 0, 0) != FILE_NOT_EXISTS) {
        return FILE_ALREADY_EXISTS;
    }

    inode_t inode;
    if (read_inode(inode_position, &inode) < 0) return FAILURE;

    const int last_char_is_slash = new_name[length - 1] == SLASH;

    if (last_char_is_slash && inode.metadata.type != INODE_DIR) return IS_FILE;

    if ((inode.metadata.permissions & WRITE) == 0) return NOT_WRITE_PERMISSIONS;
    if ((inode.metadata.permissions & READ) == 0) return NOT_READ_PERMISSIONS;

    dentry_t dentry = {0};
    dentry.inode_position = inode_position;
    strncpy(dentry.filename, new_name, last_char_is_slash ? length - 1 : length);

    return write_entry(parent_inode_position, entry_position, &dentry);
}

int my_move(const char *src_path, const char *dest_dir_path) {
    unsigned int src_parent_inode_position;
    unsigned int src_entry_position;

    const int src_inode_position = get_inode(
            src_path,
            &src_parent_inode_position, &src_entry_position
    );

    if (src_inode_position < 0) return src_inode_position;

    const int dest_inode_position = get_inode(dest_dir_path, NULL, NULL);
    if (dest_inode_position < 0) return dest_inode_position;

    inode_t dest_inode;
    if (read_inode(dest_inode_position, &dest_inode) < 0) return FAILURE;
    if (dest_inode.metadata.type != INODE_DIR) return IS_FILE;

    dentry_t src_dentry;
    const int read_bytes = read_entry(src_parent_inode_position, src_entry_position, &src_dentry);
    if (read_bytes < 0) return read_bytes;

    inode_t src_parent_inode;
    if (read_inode(src_parent_inode_position, &src_parent_inode)) return FAILURE;

    const unsigned int last_entry_position = (int) src_parent_inode.metadata.size / ENTRY_SIZE - 1;

    if (src_entry_position < last_entry_position) {
        dentry_t last_entry;
        const int err = read_entry(src_parent_inode_position, last_entry_position, &last_entry);
        if (err < 0) return err;

        const int wrote_bytes = write_entry(src_parent_inode_position, src_entry_position, &last_entry);
        if (wrote_bytes < 0) return wrote_bytes;
    }

    const int freed_blocks = my_trunc_file(src_parent_inode_position, src_parent_inode.metadata.size - ENTRY_SIZE);
    if (freed_blocks < 0) return freed_blocks;

    return my_write_file(
            dest_inode_position,
            &src_dentry,
            dest_inode.metadata.size,
            ENTRY_SIZE
    );
}

/**
 * Obtener el último segmento de una ruta. El último segmento incluye la barra del principio y la del final si tiene.
 *
 * @param path Ruta.
 *
 * @return Último segmento. (strdup)
 */
const char *get_last_segment(const char *path) {
    const char *last_slash = strrchr(path, SLASH);

    if (!last_slash) return path;
    if (*(last_slash + 1) != EOL) return last_slash;

    char *src_path_without_slash = strdup(path);
    const size_t length = strlen(path);

    src_path_without_slash[length - 1] = EOL;
    const char *segment = strchr(src_path_without_slash, SLASH);
    src_path_without_slash[length - 1] = SLASH;

    return segment;
}

int my_copy_file(const char *filename, unsigned int src_inode_position, inode_t *src_inode,
                 unsigned int dest_inode_position) {

    const int copy_inode_position = create_entry(
            filename,
            &dest_inode_position,
            src_inode->metadata.permissions, src_inode->metadata.type
    );

    const char empty_buffer[BLOCK_SIZE] = {0};
    char block[BLOCK_SIZE] = {0};

    for (int i = 0, blocks_count = 0; blocks_count < src_inode->metadata.busy_blocks_count; ++i) {
        memset(block, EMPTY_BYTE, BLOCK_SIZE);

        int read_bytes = my_read_file(src_inode_position, block, i * BLOCK_SIZE, BLOCK_SIZE);
        if (read_bytes < 0) return read_bytes;

        if (read_bytes == 0 || memcmp(block, empty_buffer, BLOCK_SIZE) == 0) continue;

        int wrote_bytes = my_write_file(copy_inode_position, block, i * BLOCK_SIZE, read_bytes);
        if (wrote_bytes < 0) return wrote_bytes;
        if (wrote_bytes > 0) blocks_count++;
    }

    return SUCCESS;
}

int my_copy_dir(const char *filename, unsigned int src_inode_position, inode_t *src_inode,
                unsigned int dest_inode_position) {

    const int copy_dir_inode_position = create_entry(
            filename,
            &dest_inode_position,
            src_inode->metadata.permissions, src_inode->metadata.type
    );

    dentry_t entries[ENTRIES_PER_BLOCK] = {0};

    for (int i = 0; i < src_inode->metadata.busy_blocks_count; ++i) {
        int read_bytes = my_read_file(src_inode_position, entries, i * BLOCK_SIZE, BLOCK_SIZE);
        if (read_bytes < 0) return read_bytes;

        const unsigned int current_block_entries_count = read_bytes / ENTRY_SIZE;

        for (int e = 0; e < current_block_entries_count; ++e) {
            dentry_t *dentry = entries + e;

            inode_t dentry_inode;
            if (read_inode(dentry->inode_position, &dentry_inode) < 0) return FAILURE;

            const size_t length = strlen(dentry->filename);
            char dentry_filename[length + 2];
            dentry_filename[0] = SLASH;
            strcpy(dentry_filename + 1, dentry->filename);
            dentry_filename[length + 1] = EOL;

            const int error = dentry_inode.metadata.type == INODE_FILE ?
                              my_copy_file(
                                      dentry_filename,
                                      dentry->inode_position,
                                      &dentry_inode,
                                      copy_dir_inode_position
                              ) :
                              my_copy_dir(
                                      dentry_filename,
                                      dentry->inode_position,
                                      &dentry_inode,
                                      copy_dir_inode_position
                              );

            if (error < 0) return error;
        }
    }

    return SUCCESS;
}

int my_copy(const char *src_path, const char *dest_dir_path) {
    const int src_inode_position = get_inode(src_path, NULL, NULL);
    if (src_inode_position < 0) return src_inode_position;

    const int dest_inode_position = get_inode(dest_dir_path, NULL, NULL);
    if (dest_inode_position < 0) return dest_inode_position;

    inode_t dest_inode;
    if (read_inode(dest_inode_position, &dest_inode) < 0) return FAILURE;
    if (dest_inode.metadata.type != INODE_DIR) return IS_FILE;

    inode_t src_inode;
    if (read_inode(src_inode_position, &src_inode) < 0) return FAILURE;

    const char *filename = get_last_segment(src_path);

    return src_inode.metadata.type == INODE_FILE
           ? my_copy_file(filename, src_inode_position, &src_inode, dest_inode_position)
           : my_copy_dir(filename, src_inode_position, &src_inode, dest_inode_position);
}