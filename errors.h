#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#define MAX_MESSAGE_SIZE 1024

#define SUCCESS 0
#define FAILURE (-1)

#define NO_READ_PERMISSIONS (-2)
#define NO_WRITE_PERMISSIONS (-3)
#define MOUNT (-4)
#define NAN (-5)
#define SYNTAX (-6)

#define RESET       "\x1b[0m"
#define RED         "\x1b[31m"
#define LIGHT_GRAY  "\x1b[90m"
#define LIGHT_BLUE  "\x1B[38;2;53;149;240m"

/**
 * Mostrar error personalizado.
 *
 * @param message Mensaje a mostrar.
 * @param ... Parámetros del formato
 *
 * @return 1
 */
int f(const char *message, ...);

/**
 * Mostrar error según su código.
 *
 * @param code Código del error. Siempre es un valor negativo.
 *
 * @return 1.
 */
int failure(signed int code, ...);