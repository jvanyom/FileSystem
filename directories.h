#include "files.h"

#define ENTRY_SIZE sizeof(struct Entry)
#define ENTRIES_PER_BLOCK (BLOCK_SIZE / ENTRY_SIZE)
#define FILENAME_SIZE 60

#define EMPTY_STR ""
#define SLASH_STR "/"

#define HYPHEN '-'
#define SLASH '/'
#define EOL '\0'

struct Entry {
    char filename[FILENAME_SIZE];
    unsigned int inodePosition;
};

/**
 * Obtener el número de i-nodo a partir de la ruta del archivo.
 *
 * @param path Ruta del archivo.
 * @param parent_inode_position Número de i-nodo del padre del archivo.
 *
 * @return Número de i-nodo. Puede devolver error.
 */
int get_inode(const char *path, unsigned int *parent_inode_position, unsigned int *entry_position);

/**
 * Crea una entrada para la ruta especificada en el directorio padre.
 *
 * @param path Ruta del archivo del que se quiere crear una entrada.
 * @param permissions Permisos que se le asignarán inicialmente al archivo.
 * @param type Tipo de entrada que se quiere crear. INODE_DIR o INODE_FILE.
 *
 * @return Número del i-nodo creado. Puede devolver error.
 */
int create_entry(const char *path, unsigned char permissions, unsigned char type);

/**
 * Cambiar los permisos de un archivo.
 *
 * @param path Ruta del archivo del que se quieren cambiar los permisos.
 * @param new_permissions Nuevos permisos del archivo.
 *
 * @return Número del i-nodo correspondiente a la ruta especificada. Puede devolver error.
 */
int my_chmod(const char *path, unsigned char new_permissions);

/**
 * Obtener información de un i-nodo a partir de su ruta.
 *
 * @param path Ruta del archivo a consultar.
 * @param metadata Buffer para los metadatos del i-nodo.
 *
 * @return Número del i-nodo correspondiente a la ruta especificada. Puede devolver error.
 */
int my_stat(const char *path, struct Metadata *metadata);