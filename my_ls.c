#include "directories.h"

void strfperms(unsigned char permissions, char *buffer) {
    buffer[0] = permissions & READ ? 'r' : HYPHEN;
    buffer[1] = permissions & WRITE ? 'w' : HYPHEN;
    buffer[2] = permissions & EXEC ? 'x' : HYPHEN;
    buffer[3] = EOL;
}

int print_entry(const struct Metadata *m, const char *filename, const int is_extended) {
    if (!is_extended) return printf("%s%s "RESET, m->type == INODE_DIR ? LIGHT_BLUE : EMPTY_STR, filename);

    const static char *entry_format = "%-4c %-8s %-19s %-8d %s%-s\n"RESET;
    const static char *date_format = "%Y-%m-%d %H:%M:%S";

    char permissions[4];
    strfperms(m->permissions, permissions);

    char time[20];
    strftime(time, 20, date_format, localtime(&m->modifiedAt));

    return printf(
            entry_format,
            m->type, permissions, time, m->size, m->type == INODE_DIR ? LIGHT_BLUE : EMPTY_STR, filename
    );
}

void print_total_entries(unsigned long total_entries) {
    printf("Entradas totales: %lu\n", total_entries);
}

void print_header() {
    printf("%-4s %-8s %-19s %-8s %-s\n", "Tipo", "Permisos", "Última modificación", "Bytes", "Nombre");
}

int main(int argc, char **argv) {
    if (argc < 3 || argc > 4) return print_error(SYNTAX, argv[0], "<dispositivo> <ruta>");

    const int is_extended = strcmp(argv[1], "-l") == 0;

    if (argc == 4 && !is_extended) return print_error(SYNTAX, argv[0], "<dispositivo> <ruta>");

    const char *dev = argv[is_extended ? 2 : 1];

    if (access(dev, F_OK) != 0) return print_error(DEV_NOT_EXISTS, dev);
    if (mount(dev) < 0) return print_error(MOUNT, dev);

    unsigned int parent_inode_position;
    unsigned int entry_position;

    const int path_inode = get_inode(argv[is_extended ? 3 : 2], &parent_inode_position, &entry_position);
    if (path_inode < 0) return print_error(path_inode);

    struct INode inode;
    if (read_inode(path_inode, &inode) < 0) return print_unexpected();

    if ((inode.metadata.permissions & READ) == 0) return print_error(NO_READ_PERMISSIONS);

    const unsigned int is_dir = inode.metadata.type == INODE_DIR;

    if (is_dir) print_total_entries(inode.metadata.size / ENTRY_SIZE);
    if (is_extended) print_header();

    if (!is_dir) {
        struct Entry entry;

        const int read_bytes = my_read_file(
                parent_inode_position,
                &entry,
                entry_position * ENTRY_SIZE,
                ENTRY_SIZE
        );

        if (read_bytes < 0) return print_error(read_bytes);
        if (read_bytes == 0) return EXIT_SUCCESS;

        print_entry(&inode.metadata, entry.filename, is_extended);
        return EXIT_SUCCESS;
    }

    struct Entry entries[ENTRIES_PER_BLOCK];

    for (int block = 0; block < inode.metadata.busyBlocksCount; ++block) {
        const int read_bytes = my_read_file(
                path_inode,
                entries,
                block * BLOCK_SIZE,
                BLOCK_SIZE
        );

        if (read_bytes < 0) return print_error(read_bytes);
        if (read_bytes == 0) return EXIT_SUCCESS;

        for (int entry = 0; entry < read_bytes / ENTRY_SIZE; ++entry) {
            if (read_inode(entries[entry].inodePosition, &inode) < 0) return print_unexpected();

            print_entry(&inode.metadata, entries[entry].filename, is_extended);
        }
    }

    return EXIT_SUCCESS;
}