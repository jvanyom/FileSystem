#include "../files.h"

int print_inode(struct Metadata *metadata, char *name) {
    printf("\n *** %s ***\n", name);

    printf(
            "Tipo: %c\n"
            "Permisos: %d\n"
            "-------------------------------------\n"
            "Fecha del último acceso a los datos: %s"
            "Fecha de la última modificación de los datos: %s"
            "Fecha de la última modificación del i-nodo: %s"
            "-------------------------------------\n"
            "Número de enlaces: %d\n"
            "Tamaño en bytes lógicos: %d\n"
            "Número de bloques ocupados: %d\n",
            metadata->type,
            metadata->permissions,
            ctime(&metadata->dataAccessedAt),
            ctime(&metadata->dataModifiedAt),
            ctime(&metadata->modifiedAt),
            metadata->totalLinks,
            metadata->logicalBytesSize,
            metadata->totalBusyBlocks
    );

    return SUCCESS;
}

int main(int argc, char **argv) {
    if (argc != 4) return failure("Sintaxi: truncar <dispositivo> <número i-nodo> <cantidad de bytes>");

    if (mount(argv[1]) == FAILURE) {
        return failure("No ha sido posible montar correctamente el dispositivo");
    }

    const unsigned int count = strtol(argv[3], NULL, 10);
    const unsigned int inode_position = strtol(argv[2], NULL, 10);

    if (count == 0) {
        free_inode(inode_position);
    } else {
        my_trunc(inode_position, count);
    }

    struct Metadata metadata;

    if (my_stat(inode_position, &metadata) == FAILURE) {
        return failure("No ha sido posible leer correctamente el i-nodo");
    }

    char string[64];
    sprintf(string, "Metadatos i-nodo %d", inode_position);
    print_inode(&metadata, string);

    if (umount() == FAILURE) return failure("No ha sido posible desmontar correctamente el dispositivo");

    return EXIT_SUCCESS;
}