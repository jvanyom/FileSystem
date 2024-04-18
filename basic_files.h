#include <limits.h>
#include <time.h>

#include "blocks.h"

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

#define FREE_BLOCK 0
#define BUSY_BLOCK 1

#define FREE_INODE 'l'
#define DIR_INODE 'd'
#define FILE_INODE 'f'

#define READ 0b100
#define WRITE 0b010
#define EXEC 0b001

#define ALL_PERMS (READ | WRITE | EXEC)
#define RW (READ | WRITE)
#define RX (READ | EXEC)
#define WX (WRITE | EXEC)
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

#define DEBUG 1

#define debug(fmt, ...) \
        do { if (DEBUG) fprintf(\
                            stderr, LIGHT_GRAY"[%s:%d:%s(): " fmt "]\n"RESET, \
                            __FILE__, __LINE__, __func__, __VA_ARGS__\
                        ); \
        } while (0)

// 1024 Bytes
struct SuperBlock {
    /**
     * Posición absoluta del primer bloque del mapa de bits
     */
    unsigned int bitMapFirstBlock;
    /**
     * Posición absoluta del último bloque del mapa de bits
     */
    unsigned int bitMapLastBlock;

    /**
     * Posición absoluta del primer bloque del array de i-nodos
     */
    unsigned int iNodesFirstBlock;
    /**
     * Posición absoluta del último bloque del array de i-nodos
     */
    unsigned int iNodesLastBlock;

    /**
     * Posición absoluta del primer bloque de datos
     */
    unsigned int dataFirstBlock;
    /**
     * Posición absoluta del último bloque de datos
     */
    unsigned int dataLastBlock;

    /**
     * Posición del i-nodo del directorio raíz (relativa al array de i-nodos)
     */
    unsigned int rootINode;
    /**
     * Posición del primer i-nodo libre (relativa al array de i-nodos)
     */
    unsigned int firstFreeINode;

    /**
     * Cantidad de bloques libres (en el disco completo)
     */
    unsigned int totalFreeBlocks;
    /**
     * Cantidad de i-nodos libres (en el array de i-nodos)
     */
    unsigned int totalFreeINodes;

    /**
     * Cantidad total de bloques del disco
     */
    unsigned int totalBlocks;
    /**
     * Cantidad total de i-nodos (heurística)
     */
    unsigned int totalINodes;

    /**
     * Reservado
     */
    char _padding[SUPER_BLOCK_SIZE - 12 * sizeof(unsigned int)];
};

// 128 Bytes
struct INode {
    /**
     * Directorio, fichero o libre.
     */
    unsigned char type;
    /**
     * Escritura (0b100), lectura (0b010) y ejecución (0b001)
     */
    unsigned char permissions;

    unsigned char _memoryAlignment[6];

    /**
     * Fecha y hora del último acceso a datos
     */
    time_t dataAccessedAt;
    /**
     * Fecha y hora de la última modificación de datos
     */
    time_t dataModifiedAt;
    /**
     * Fecha y hora de la última modificación del i-nodo
     */
    time_t modifiedAt;

    /**
     * Cantidad de enlaces de entradas en directorio
     */
    unsigned int totalLinks;
    /**
     * Tamaño en bytes lógicos (EOF)
     */
    unsigned int logicalBytesSize;
    /**
     * Cantidad de bloques ocupados por la zona de datos
     */
    unsigned int totalBusyBlocks;

    unsigned int directPointers[DIRECT_POINTERS];
    unsigned int indirectPointers[INDIRECT_POINTERS];

    /**
     * Reservado
     */
    char _padding[
            INODE_SIZE
            - 2 * sizeof(unsigned char)
            - 3 * sizeof(time_t)
            - 18 * sizeof(unsigned int)
            - 6 * sizeof(unsigned char)
    ];
};

/**
 * Calcular tamaño en bloques del mapa de bits (MB).
 *
 * @param total_blocks Número total de bloques del dispositivo.
 *
 * @return Tamaño en bloques del mapa de bits.
 */
int bitmap_size(unsigned int total_blocks);

/**
 * Calcular tamaño en bloques del array de i-nodos.
 *
 * @param total_inodes Número total de i-nodos.
 *
 * @return Tamaño en bloques del array de i-nodos.
 */
int inodes_size(unsigned int total_inodes);

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
int write_inode(unsigned int inode_position, struct INode *inode);

/**
 * Leer i-nodo.
 *
 * @param inode_position Posición del i-nodo relativa al array de i-nodos.
 * @param inode Buffer del i-nodo.
 *
 * @return 1 si se ha leído correctamente. Puede devolver error.
 */
int read_inode(unsigned int inode_position, struct INode *inode);

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
int free_inode_blocks(unsigned int first_logical_block, struct INode *inode);

/**
 * Obtener el bloque físico correspondiente al bloque lógico del i-nodo.
 *
 * @param inode I-Nodo.
 * @param logical_block Número de bloque lógico.
 * @param reserve 1 si se reserva. 0 si no se reserva.
 *
 * @return Puntero al bloque físico correspondiente al bloque lógico especificado.
 */
int get_physical_by_logical_block(struct INode *inode, unsigned int logical_block, unsigned char reserve);