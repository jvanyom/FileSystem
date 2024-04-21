#include "errors.h"

const static char *errors[] = {
        "Ha ocurrido un error desconocido",
        "No tienes permisos de lectura",
        "No tienes permisos de escritura",
        "No ha sido posible montar correctamente el dispositivo '%s'",
        "El valor de '%s' debe ser un número",
        "Sintaxi: %s %s"
};

int print(const char *message) {
    return fprintf(stderr, RED"Error: "RESET"%s\n", message);
}

int f(const char *message, ...) {
    char buffer[MAX_MESSAGE_SIZE];

    va_list args;
    va_start(args, message);
    vsnprintf(buffer, MAX_MESSAGE_SIZE, message, args);
    va_end(args);

    print(buffer);
    return EXIT_FAILURE;
}

int failure(signed int code, ...) {
    char buffer[MAX_MESSAGE_SIZE];

    va_list args;
    va_start(args, code);
    vsnprintf(buffer, MAX_MESSAGE_SIZE, errors[code * -1 - 1], args);
    va_end(args);

    print(buffer);
    return EXIT_FAILURE;
}