#include <limits.h>
#include <time.h>

#include "blocks.h"

#define DEBUG_INODES 0

#define SUPER_BLOCK_SIZE BLOCK_SIZE
#define INODE_SIZE 128

#define INODES_PER_BLOCK (BLOCK_SIZE / INODE_SIZE)
#define SUPER_BLOCKS_PER_BLOCK (BLOCK_SIZE / SUPER_BLOCK_SIZE)

#define SUPER_BLOCK_POSITION 0

#define BYTE_LENGTH CHAR_BIT
#define FULL_BYTE 0b11111111
#define EMPTY_BYTE 0b00000000
#define BIT7 0b10000000

#define NO_RESERVE 0
#define RESERVE 1

#define NO_CREATE 0
#define CREATE 1

#define FREE_BLOCK 0
#define BUSY_BLOCK 1

#define INODE_FREE 'l'
#define INODE_DIR 'd'
#define INODE_FILE 'f'

#define READ 0b100
#define WRITE 0b010
#define EXEC 0b001

#define RW (READ | WRITE)
#define RWX (RW | EXEC)

#define DIRECT_POINTERS 12
#define INDIRECT_POINTERS 3
#define POINTERS (DIRECT_POINTERS + INDIRECT_POINTERS)

#define POINTERS_PER_BLOCK (BLOCK_SIZE / sizeof(unsigned int))

#define LEVEL0_MAX DIRECT_POINTERS
#define LEVEL1_MAX (POINTERS_PER_BLOCK + LEVEL0_MAX)
#define LEVEL2_MAX (POINTERS_PER_BLOCK * POINTERS_PER_BLOCK + LEVEL1_MAX)
#define LEVEL3_MAX (POINTERS_PER_BLOCK * POINTERS_PER_BLOCK * POINTERS_PER_BLOCK + LEVEL2_MAX)

#define NUM_LEVELS 4

#define DATETIME_FORMAT ("%H:%M:%S %d/%m/%Y")
#define DATETIME_LENGTH 20

#define debug(d, fmt, ...) \
        do { if (d) fprintf(\
                            stderr, LIGHT_GRAY"[%s:%d:%s(): " fmt "]\n"RESET, \
                            __FILE__, __LINE__, __func__, __VA_ARGS__\
                        ); \
        } while (0)

/**
 * Súper bloque del sistema de ficheros. Almacena toda la metainformación relevante de este.
 */
typedef struct {
    /**
     * Posición absoluta del primer bloque del mapa de bits.
     */
    unsigned int bitmap_first_block;
    /**
     * Posición absoluta del último bloque del mapa de bits.
     */
    unsigned int bitmap_last_block;

    /**
     * Posición absoluta del primer bloque del array de i-nodos.
     */
    unsigned int inodes_first_block;
    /**
     * Posición absoluta del último bloque del array de i-nodos.
     */
    unsigned int inodes_last_block;

    /**
     * Posición absoluta del primer bloque de datos.
     */
    unsigned int data_first_block;
    /**
     * Posición absoluta del último bloque de datos.
     */
    unsigned int data_last_block;

    /**
     * Posición del i-nodo del directorio raíz (relativa al array de i-nodos).
     */
    unsigned int root_inode;
    /**
     * Posición del primer i-nodo libre (relativa al array de i-nodos).
     */
    unsigned int next_free_inode_position;

    /**
     * Cantidad de bloques libres (en el disco completo).
     */
    unsigned int free_blocks_count;
    /**
     * Cantidad de i-nodos libres (en el array de i-nodos).
     */
    unsigned int free_inodes_count;

    /**
     * Cantidad total de bloques del disco.
     */
    unsigned int blocks_count;
    /**
     * Cantidad total de i-nodos (heurística).
     */
    unsigned int inodes_count;

    /**
     * Reservado.
     */
    char __padding[SUPER_BLOCK_SIZE - 12 * sizeof(unsigned int)];
} super_block_t;

typedef struct {
    /**
     * Directorio, fichero o libre.
     */
    unsigned char type;
    /**
     * Escritura (0b100), lectura (0b010) y ejecución (0b001).
     */
    unsigned char permissions;

    unsigned char __memoryAlignment[6];

    /**
     * Fecha y hora del último acceso a datos.
     */
    time_t data_accessed_at;
    /**
     * Fecha y hora de la última modificación de datos.
     */
    time_t data_modified_at;
    /**
     * Fecha y hora de la última modificación del i-nodo.
     */
    time_t modified_at;

    /**
     * Cantidad de enlaces de entradas en directorio.
     */
    unsigned int links_count;
    /**
     * Tamaño en bytes lógicos (EOF).
     */
    unsigned int size;
    /**
     * Cantidad de bloques ocupados por la zona de datos.
     */
    unsigned int busy_blocks_count;
} metadata_t;

typedef struct {
    /**
     * Metadatos del i-nodo.
     */
    metadata_t metadata;

    /**
     * Apuntan directamente a bloques de datos.
     */
    unsigned int direct[DIRECT_POINTERS];
    /**
     * Apuntan a bloques de punteros. De nivel 1, nivel 2 y nivel 3 respectivamente.
     */
    unsigned int indirect[INDIRECT_POINTERS];

    /**
     * Reservado.
     */
    char __padding[INODE_SIZE - sizeof(metadata_t) - POINTERS * sizeof(unsigned int)];
} inode_t;

/**
 * Calcular tamaño en bloques en función del número de bytes.
 *
 * @param bytes Bytes.

 * @return Tamaño en bloques.
 */
unsigned int block_size(unsigned int bytes);

/**
 * Calcular tamaño en bloques del mapa de bits (MB).
 *
 * @param total_blocks Número total de bloques del dispositivo.
 *
 * @return Tamaño en bloques del mapa de bits.
 */
unsigned int bitmap_size(unsigned int total_blocks);

/**
 * Calcular tamaño en bloques del array de i-nodos.
 *
 * @param total_inodes Número total de i-nodos.
 *
 * @return Tamaño en bloques del array de i-nodos.
 */
unsigned int inodes_size(unsigned int total_inodes);

/**
 * Inicializar súper bloque.
 *
 * @param total_blocks Número total de bloques del dispositivo.
 * @param total_inodes Número total de i-nodos.
 *
 * @return Número de bytes escritos. Puede devolver error.
 */
int init_super_block(unsigned int total_blocks, unsigned int total_inodes);

/**
 * Inicializar mapa de bits ocupando los bits que representan los metadatos.
 *
 * @return 1 si se ha inicializado correctamente. Puede devolver error.
 */
int init_bitmap();

/**
 * Inicializar lista de i-nodos libres.
 * Todos los i-nodos se enlazan con el siguiente a través del primer puntero directo.
 *
 * @return 1 si se ha inicializado correctamente. Puede devolver error.
 */
int init_inodes();

/**
 * Modificar estado de un bloque físico en el mapa de bits. 0 si se libera. 1 si se ocupa.
 *
 * @param physical_block Bloque físico del que se quiere modificar su estado.
 * @param state 0 o 1. Indica que el bloque se queda libre o ocupado, respectivamente.
 *
 * @return 1 si se ha cambiado el estado correctamente. Puede devolver error.
 */
int set_block_state(unsigned int physical_block, unsigned int state);

/**
 * Consultar estado de un bloque físico en el mapa de bits.
 *
 * @param physical_block Bloque físico del que se quiere conocer el estado.
 *
 * @return Estado del bloque físico. 0 si está libre. 1 si está ocupado. Puede devolver error.
 */
char get_block_state(unsigned int physical_block);

/**
 * Encontrar el primer bloque físico libre y ocuparlo.
 *
 * @return Posición del bloque físico reservado. Puede devolver error.
 */
int reserve_block();

/**
 * Librar un bloque físico.
 *
 * @param physical_block Bloque físico a liberar.
 *
 * @return Número del bloque liberado. Puede devolver error.
 */
int free_block(unsigned int physical_block);

/**
 * Escribir i-nodo.
 *
 * @param inode_position Posición del i-nodo relativa al array de i-nodos.
 * @param inode Contenido del i-nodo.
 *
 * @return 1 si se guardado correctamente la información. Puede devolver error.
 */
int write_inode(unsigned int inode_position, inode_t *inode);

/**
 * Leer i-nodo.
 *
 * @param inode_position Posición del i-nodo relativa al array de i-nodos.
 * @param inode Buffer del i-nodo.
 *
 * @return 1 si se ha leído correctamente. Puede devolver error.
 */
int read_inode(unsigned int inode_position, inode_t *inode);

/**
 * Reservar primer i-nodo libre.
 *
 * @param type Tipo del i-nodo.
 * @param permissions Permisos del i-nodo.
 *
 * @return Posición del i-nodo reservado. Puede devolver error.
 */
int reserve_inode(unsigned char type, unsigned char permissions);

/**
 * Liberar i-nodo.
 * Liberar un i-nodo implica que pasará a encabezar la lista de i-nodos libres y todos los bloques de datos y bloques
 * de punteros también se liberaran.
 *
 * @param inode_position Posición del i-nodo.
 *
 * @return Posición del i-nodo liberado. Puede devolver error.
 */
int free_inode(unsigned int inode_position);

/**
 * Liberar bloques de datos y bloques de punteros del i-nodo.
 *
 * @param first_logical_block Bloque lógico a partir del cual se tiene que liberar el i-nodo.
 * @param inode I-Nodo a liberar.
 *
 * @return Cantidad de bloques liberados. Puede devolver error.
 */
int free_inode_blocks(unsigned int first_logical_block, inode_t *inode);

/**
 * Obtener el bloque físico correspondiente al bloque lógico del i-nodo.
 *
 * @param inode I-Nodo.
 * @param logical_block Número de bloque lógico.
 * @param reserve 1 si se reserva. 0 si no se reserva.
 *
 * @return Puntero al bloque físico correspondiente al bloque lógico especificado. Puede devolver error.
 */
int get_physical_block(inode_t *inode, unsigned int logical_block, unsigned char reserve);

/**
 * Formatea dentro de 'str' el valor de 'time'
 *
 * @param str Cadena donde se va a realizar el formateo.
 * @param time Tiemo que se quiere formatear.
 */
void format_datetime(char *str, const time_t *time);

/**
 * Mostrar los metadatos de un i-nodo por pantalla.
 *
 * @param metadata Metadatos del i-nodo a mostrar.
 * @param name Título/Nombre del i-nodo.
 *
 * @return 0.
 */
int print_inode(metadata_t *metadata, char *name);

/**
 * Comprobar que los permisos son correctos. Los permisos son correctos si es un número entre 0 y 7.
 *
 * @param permissions Permisos a comprobar.
 *
 * @return 1 si son válidos 0 en cualquier otro caso.
 */
int are_valid(char permissions);