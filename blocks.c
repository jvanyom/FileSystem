#include "blocks.h"

static int descriptor = 0;

int mount(const char *path) {
    if ((descriptor = open(path, O_RDWR | O_CREAT)) == FAILURE) return FAILURE;

    chmod(path, 0666);

    return descriptor;
}

int umount() {
    return close(descriptor) ? FAILURE : SUCCESS;
}

int write_block(unsigned int physical_block, const void *buffer) {
    if (lseek(descriptor, physical_block * BLOCK_SIZE, SEEK_SET) == FAILURE) return FAILURE;

    return (int) write(descriptor, buffer, BLOCK_SIZE);
}

int read_block(unsigned int physical_block, void *buffer) {
    if (lseek(descriptor, physical_block * BLOCK_SIZE, SEEK_SET) == FAILURE) return FAILURE;

    return (int) read(descriptor, buffer, BLOCK_SIZE);
}

signed int failure(char *message) {
    fprintf(stderr, RED"Error: "RESET"%s\n", message);
    return EXIT_FAILURE;
}