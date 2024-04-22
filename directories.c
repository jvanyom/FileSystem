#include "directories.h"

int split_path(const char *path, char *initial, char *final, char *type) {
    if (path[0] != SLASH) return FAILURE;

    const char *no_slash_path = path + 1;
    const char *slash = strchr(no_slash_path, SLASH);

    if (slash) {
        const long length = slash - no_slash_path;

        strncpy(initial, no_slash_path, length);
        initial[length] = EOL;

        strcpy(final, slash);
        *type = DIR_INODE;
    } else {
        strcpy(initial, no_slash_path);
        strcpy(final, EMPTY_STR);
        *type = FILE_INODE;
    }

    return SUCCESS;
}

int find_entry(const char *partial_path, unsigned int parent_inode_position, char reserve, unsigned char permissions) {
    if (strcmp(partial_path, SLASH_STR) == 0) return SUCCESS;

    char initial_path[FILENAME_SIZE] = {0};
    char final_path[strlen(partial_path)];
    char type;

    if (split_path(partial_path, initial_path, final_path, &type) < 0) return BAD_PATH;

    debug("Inicial: %s | Final: %s", initial_path, final_path);

    struct INode parent_inode;
    if (read_inode(parent_inode_position, &parent_inode) < 0) return FAILURE;

    struct Entry entries[ENTRIES_PER_BLOCK] = {0};
    unsigned int entry_position = 0;

    for (unsigned int block = 0; block < parent_inode.metadata.busyBlocksCount; ++block) {
        const signed read_bytes = my_read(parent_inode_position, entries, block * BLOCK_SIZE, BLOCK_SIZE);
        if (read_bytes < 0) return FAILURE;
        entry_position = 0;

        for (int entry = 0; entry < read_bytes / ENTRY_SIZE; ++entry, entry_position++) {
            const struct Entry *e = entries + entry;

            if (strcmp(e->filename, initial_path) == 0) {
                if (strcmp(final_path, EMPTY_STR) == 0 || strcmp(final_path, SLASH_STR) == 0) {
                    return reserve ? FILE_ALREADY_EXISTS : SUCCESS;
                }

                return find_entry(final_path, e->inodePosition, reserve, permissions);
            }
        }
    }

    if (reserve == NO_RESERVE) return BAD_PATH;
    if (parent_inode.metadata.type == FILE_INODE) return IS_FILE;
    if ((parent_inode.metadata.permissions & WRITE) == 0) return NO_WRITE_PERMISSIONS;
    if (type == DIR_INODE && strcmp(final_path, SLASH_STR) != 0) return BAD_PATH;

    const int new_inode_position = reserve_inode(*final_path == SLASH ? DIR_INODE : FILE_INODE, RW);
    if (new_inode_position < 0) return FAILURE;

    struct Entry new_entry = {.filename = {0}};
    strcpy(new_entry.filename, initial_path);
    new_entry.inodePosition = new_inode_position;

    const signed wrote_bytes = my_write(
            parent_inode_position,
            &new_entry,
            entry_position * ENTRY_SIZE,
            ENTRY_SIZE
    );

    if (wrote_bytes < 0 && free_inode(new_entry.inodePosition) < 0) return FAILURE;

    debug("Se ha creado la entrada '%s' en la posición %d", new_entry.filename, entry_position);
    return SUCCESS;
}