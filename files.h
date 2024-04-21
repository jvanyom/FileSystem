#include "basic_files.h"

/**
 * Escribir contenido de 'buffer', de tamaño 'count', en un fichero/directorio referido por el i-nodo.
 *
 * @param inode_position Posición del i-nodo que se quiere escribir.
 * @param buffer Contenido que se quiere escribir.
 * @param file_offset Posición inicial de escritura en bytes.
 * @param count Número de bytes a escribir.
 *
 * @return Bytes escritos. Puede devolver error.
 */
int my_write(unsigned int inode_position, const void *buffer, unsigned int file_offset, unsigned int count);

/**
 * Leer contenido de un fichero/directorio dentro de 'buffer'.
 *
 * @param inode_position Posición del i-nodo que se quiere leer.
 * @param buffer Contenedor donde se almacena la información leída.
 * @param file_offset Posición inicial de escritura en bytes.
 * @param count Cantidad a leer en bytes.
 *
 * @return Bytes leídos. Puede devolver error.
 */
int my_read(unsigned int inode_position, void *buffer, unsigned int file_offset, unsigned int count);

/**
 * Leer metadatos de un i-nodo.
 *
 * @param inode_position Posición del i-nodo del que se quieren consultar los metadatos.
 * @param metadata Puntero al contenedor de los metadatos.
 *
 * @return 1 si se ha leído correctamente los metadatos del i-nodo. Puede devolver error.
 */
int my_stat(unsigned int inode_position, struct Metadata *metadata);

/**
 * Modificar permisos de un i-nodo.
 *
 * @param inode_position Posición del i-nodo del que se quieren modificar los permisos.
 * @param permissions Nuevos permisos del i-nodo.
 *
 * @return 1 si ha sido posible modificar los permisos correctamente. Puede devolver error.
 */
int my_chmod(unsigned int inode_position, unsigned char permissions);

/**
 * Truncar contenido de un i-nodo a los bytes indicados por 'count'.
 *
 * @param inode_position Posición del i-nodo a truncar.
 * @param count Número de bytes a los que tiene que truncar los datos del i-nodo.
 *
 * @return Cantidad de bloques liberados. Puede devolver error.
 */
int my_trunc(unsigned int inode_position, unsigned int count);