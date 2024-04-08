#include "ficheros_basico.h"

int main(int argc, char **argv) {
    if (argc != 3) return failure("Debe haber exactamente 2 parámetros");

    if (bmount(argv[1]) == FAILURE) {
        return failure("No ha sido posible montar correctamente el dispositivo");
    }

    unsigned char empty_block[BLOCK_SIZE];
    memset(empty_block, EMPTY_BYTE, BLOCK_SIZE);

    const long num_blocks = strtol(argv[2], NULL, 10);

    if (num_blocks == 0) return failure("El parámetro de bloques debe ser un número válido");

    for (int i = 0; i < num_blocks; ++i) {
        if (bwrite(i, empty_block) == FAILURE) {
            return failure("No ha sido posible escribir un bloque vacío correctamente");
        }
    }

    if (initSB(num_blocks, num_blocks / 4) == FAILURE) {
        return failure("No ha sido posible inicializar correctamente el súper bloque");
    }

    if (initMB() == FAILURE) {
        return failure("No ha sido posible inicializar correctamente el mapa de bits");
    }

    if (initAI() == FAILURE) {
        return failure("No ha sido posible inicializar correctamente el array de i-nodos");
    }

    if (reservar_inodo(INODE_DIR, ALL_PERMS) == FAILURE) {
        return failure("No ha sido posible reservar correctamente el i-nodo raíz");
    }

    if (bumount() == FAILURE) return failure("No ha sido posible desmontar correctamente el dispositivo");

    return EXIT_SUCCESS;
}