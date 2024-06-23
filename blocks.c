#include "blocks.h"

static int descriptor = 0;

#ifdef MMAP
static long mem_size = 0;
static void *ptr = NULL;

int do_mmap() {
    struct stat st;

    fstat(descriptor, &st);
    mem_size = st.st_size;

    if ((ptr = mmap(NULL, mem_size, PROT_WRITE, MAP_SHARED, descriptor, 0)) == (void *) -1) {
        return FAILURE;
    }

    return close(descriptor) < 0 ? FAILURE : SUCCESS;
}
#endif

int dev_mount(const char *path) {
    if ((descriptor = open(path, O_RDWR | O_CREAT)) < 0) return FAILURE;
    chmod(path, 0666);

#ifdef MMAP
    return do_mmap();
#else
    return descriptor;
#endif
}

int dev_umount() {
#ifdef MMAP
    if (msync(ptr, mem_size, MS_ASYNC | MS_INVALIDATE) < 0) return FAILURE;

    return munmap(ptr, mem_size) < 0 ? FAILURE : SUCCESS;
#else
    return close(descriptor) ? FAILURE : SUCCESS;
#endif
}

int write_block(unsigned int physical_block, const void *buffer) {
#ifdef MMAP
    const unsigned int bytes = physical_block * BLOCK_SIZE + BLOCK_SIZE <= mem_size
                               ? BLOCK_SIZE
                               : mem_size - physical_block * BLOCK_SIZE;

    if (bytes > 0) memcpy(ptr + physical_block * BLOCK_SIZE, buffer, bytes);

    return (int) bytes;
#else
    if (lseek(descriptor, physical_block * BLOCK_SIZE, SEEK_SET) < 0) return FAILURE;

    return (int) write(descriptor, buffer, BLOCK_SIZE);
#endif
}

int read_block(unsigned int physical_block, void *buffer) {
#ifdef MMAP
    const unsigned int bytes = physical_block * BLOCK_SIZE + BLOCK_SIZE <= mem_size
                               ? BLOCK_SIZE
                               : mem_size - physical_block * BLOCK_SIZE;

    if (bytes > 0) memcpy(buffer, ptr + physical_block * BLOCK_SIZE, bytes);

    return (int) bytes;
#else
    if (lseek(descriptor, physical_block * BLOCK_SIZE, SEEK_SET) < 0) return FAILURE;

    return (int) read(descriptor, buffer, BLOCK_SIZE);
#endif
}
