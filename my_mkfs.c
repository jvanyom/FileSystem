#include "basic_files.h"
#include "errors.h"

int clear_all_blocks(unsigned int total_blocks) {
    const static unsigned char empty_block[BLOCK_SIZE] = {0};

    for (unsigned int i = 0; i < total_blocks; ++i) {
        if (write_block(i, empty_block) < 0) return FAILURE;
    }

    return SUCCESS;
}

int main(int argc, char **argv) {
    if (argc != 3) return print_error(SYNTAX, argv[0], "<dispositivo> <número de bloques>");

    const unsigned int total_blocks = strtol(argv[2], NULL, 10);
    if (errno == EINVAL) return print_error(NAN, "número de bloques");

    const unsigned int total_inodes = total_blocks >> 2;

    const signed int failed = mount(argv[1]) < 0 ||
                              clear_all_blocks(total_blocks) < 0 ||
                              init_super_block(total_blocks, total_inodes) < 0 ||
                              init_bitmap() < 0 ||
                              init_inodes() < 0 ||
                              reserve_inode(INODE_DIR, RWX) < 0 ||
                              umount() < 0;

    return failed ? print_error(MOUNT, argv[1]) : EXIT_SUCCESS;
}