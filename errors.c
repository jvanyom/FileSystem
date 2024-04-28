#include "errors.h"

const static char *errors[] = {
        "Ha ocurrido un error desconocido",
        "No tienes permisos de lectura",
        "No tienes permisos de escritura",
        "Los permisos no son válidos",
        "No ha sido posible montar correctamente el dispositivo '%s'",
        "El dispositivo '%s' no existe",
        "El valor de '%s' debe ser un número",
        "Sintaxi: %s %s",
        "El archivo especificado no existe",
        "El archivo especificado ya existe",
        "El archivo especificado es un fichero",
        "No ha sido posible crear correctamente el recurso"
};

int print(const char *message) {
    return fprintf(stderr, RED"Error: "RESET"%s\n", message);
}

int print_cerror(const char *message, ...) {
    char buffer[MAX_MESSAGE_SIZE];
    va_list args;

    va_start(args, message);
    vsnprintf(buffer, MAX_MESSAGE_SIZE, message, args);
    va_end(args);

    print(buffer);
    return EXIT_FAILURE;
}

int print_error(signed int code, ...) {
    char buffer[MAX_MESSAGE_SIZE];
    va_list args;

    va_start(args, code);
    vsnprintf(buffer, MAX_MESSAGE_SIZE, errors[code * -1 - 1], args);
    va_end(args);

    print(buffer);
    return EXIT_FAILURE;
}

int print_unexpected() {
    print(*errors);
    return EXIT_FAILURE;
}