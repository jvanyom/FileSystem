#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>

#define BLOCK_SIZE 1024

#define SUCCESS 1
#define FAILURE (-1)

#define BOLD    "\x1b[1m"
#define RESET   "\x1b[0m"

#define BLACK   "\x1B[30m"
#define RED     "\x1b[31m"
#define GREEN   "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define BLUE    "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN    "\x1b[36m"
#define WHITE   "\x1B[37m"
#define ORANGE  "\x1B[38;2;255;128;0m"
#define ROSE    "\x1B[38;2;255;151;203m"
#define LIGHT_BLUE   "\x1B[38;2;53;149;240m"
#define LIGHT_GREEN  "\x1B[38;2;17;245;120m"
#define GRAY    "\x1B[38;2;176;174;174m"
#define LIGHT_GRAY "\x1b[90m"

int bmount(const char *path);

int bumount();

int bwrite(unsigned int physical_block, const void *buffer);

int bread(unsigned int physical_block, void *buffer);

signed int failure(char *message);