#include "files.h"
#include "dcache.h"

#define DEBUG_ENTRIES 0

#define FILENAME_SIZE 60

#define EMPTY_STR ""
#define SLASH_STR "/"
#define ROOT SLASH_STR

#define HYPHEN '-'

#define ENTRY_SIZE sizeof(dentry_t)
#define ENTRIES_PER_BLOCK (BLOCK_SIZE / ENTRY_SIZE)

typedef struct {
    unsigned int inode_position;
    char filename[FILENAME_SIZE];
} dentry_t;

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
int my_stat(const char *path, metadata_t *metadata);

/**
 * Escribir contenido en un fichero a partir de su ruta.
 *
 * @param path Ruta del fichero donde se quiere escribir.
 * @param buffer Datos a escribir.
 * @param offset Byte a partir del que se quiere escribir.
 * @param count Número de bytes a escribir.
 *
 * @return Bytes escritos.
 */
int my_write(const char *path, const void *buffer, unsigned int offset, unsigned int count);

/**
 * Leer contenido de un fichero a partir de su ruta.
 *
 * @param path Ruta del fichero.
 * @param buffer Contenedor.
 * @param offset Byte a partir del que se quiere leer.
 * @param count Número de bytes a leer.
 *
 * @return Bytes leídos.
 */
int my_read(const char *path, void *buffer, unsigned int offset, unsigned int count);

/**
 * Crear enlace de 'target' con el nombre 'link_name'
 *
 * @param target Archivo que se quiere enlazar.
 * @param link Nombre del enlace
 *
 * @return 0. Puede devolver error.
 */
int my_link(const char *target, const char *link);

/**
 * Borrar entrada de directorio del fichero especificado.
 *
 * @param path Ruta del fichero del que se quiere borrar la entrada.
 * @param type Tipo del fichero. INODE_FILE o INODE_DIR.
 *
 * @return 0. Puede devolver error.
 */
int my_unlink(const char *path, unsigned char type);