#include "../basic_files.h"

int main(int argc, char **argv) {
    if (argc != 3) return failure("Debe haber exactamente 2 parámetros");

    if (mount(argv[1]) == FAILURE) {
        return failure("No ha sido posible montar correctamente el dispositivo");
    }

    unsigned char empty_block[BLOCK_SIZE] = {0};

    const long total_blocks = strtol(argv[2], NULL, 10);

    if (total_blocks == 0) return failure("El parámetro de bloques debe ser un número válido");

    for (int i = 0; i < total_blocks; ++i) {
        if (write_block(i, empty_block) == FAILURE) {
            return failure("No ha sido posible escribir un bloque vacío correctamente");
        }
    }

    if (init_super_block(total_blocks, total_blocks / 4) == FAILURE) {
        return failure("No ha sido posible inicializar correctamente el súper bloque");
    }

    if (init_bitmap() == FAILURE) {
        return failure("No ha sido posible inicializar correctamente el mapa de bits");
    }

    if (init_inodes() == FAILURE) {
        return failure("No ha sido posible inicializar correctamente el array de i-nodos");
    }

    if (reserve_inode(DIR_INODE, ALL_PERMS) == FAILURE) {
        return failure("No ha sido posible reservar correctamente el i-nodo raíz");
    }

    if (umount() == FAILURE) return failure("No ha sido posible desmontar correctamente el dispositivo");

    return EXIT_SUCCESS;
}