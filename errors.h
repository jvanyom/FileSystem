#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#define MAX_MESSAGE_SIZE 1024

#define SUCCESS 0
#define FAILURE (-1)

#define NOT_READ_PERMISSIONS (-2)
#define NOT_WRITE_PERMISSIONS (-3)
#define NOT_VALID_PERMISSIONS (-4)
#define MOUNT (-5)
#define DEV_NOT_EXISTS (-6)
#define NAN (-7)
#define SYNTAX (-8)
#define FILE_NOT_EXISTS (-9)
#define FILE_ALREADY_EXISTS (-10)
#define IS_FILE (-11)
#define IS_NOT_FILE (-12)
#define NOT_EMPTY_DIR (-13)
#define NOT_CREATED (-14)

#define RESET       "\x1b[0m"
#define RED         "\x1b[31m"
#define LIGHT_GRAY  "\x1b[90m"
#define LIGHT_BLUE  "\x1B[38;2;53;149;240m"
#define MAGENTA     "\x1b[35m"

/**
 * Mostrar error personalizado.
 *
 * @param message Mensaje a mostrar.
 * @param ... Parámetros del formato
 *
 * @return 1
 */
int print_cerror(const char *message, ...);

/**
 * Mostrar error según su código.
 *
 * @param code Código del error. Siempre es un valor negativo.
 *
 * @return 1.
 */
int print_error(signed int code, ...);

/**
 * Equivalente a 'print_error(FAILURE)'.
 *
 * @return EXIT_FAILURE.
 */
int print_unexpected();