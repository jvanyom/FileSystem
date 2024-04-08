#include <limits.h>
#include <time.h>

#include "bloques.h"

#define SUPER_BLOCK_SIZE BLOCK_SIZE
#define INODE_SIZE 128

#define INODES_PER_BLOCK (BLOCK_SIZE / INODE_SIZE)
#define SUPER_BLOCKS_PER_BLOCK (BLOCK_SIZE / SUPER_BLOCK_SIZE)

#define SUPER_BLOCK_POSITION 0

#define LAST_INODE (INODES_PER_BLOCK - 1)

#define BYTE_LENGTH CHAR_BIT
#define FULL_BYTE 0b11111111
#define EMPTY_BYTE 0b00000000
#define BIT7 0b10000000

#define NO_RESERVE 0
#define RESERVE 1

#define FREE_BLOCK 0
#define BUSY_BLOCK 1

#define INODE_FREE 'l'
#define INODE_DIR 'd'
#define INODE_FILE 'f'

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

#define POINTERS_PER_BLOCK (BLOCK_SIZE / sizeof(unsigned int))

#define LEVEL0_MAX DIRECT_POINTERS
#define LEVEL1_MAX (POINTERS_PER_BLOCK + LEVEL0_MAX)
#define LEVEL2_MAX (POINTERS_PER_BLOCK * POINTERS_PER_BLOCK + LEVEL1_MAX)
#define LEVEL3_MAX (POINTERS_PER_BLOCK * POINTERS_PER_BLOCK * POINTERS_PER_BLOCK + LEVEL2_MAX)

#define NUM_LEVELS 4

#define DEBUG 1

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
    time_t iNodeModifiedAt;

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

int tamMB(unsigned int blocks_num);

int tamAI(unsigned int inodes_num);

int initSB(unsigned int blocks_num, unsigned int inodes_num);

int initMB();

int initAI();

int escribir_bit(unsigned int block, unsigned int bit);

char leer_bit(unsigned int block);

int reservar_bloque();

int liberar_bloque(unsigned int block);

int escribir_inodo(unsigned int inode_position, struct INode *inode);

int leer_inodo(unsigned int inode_position, struct INode *inode);

int reservar_inodo(unsigned char type, unsigned char permissions);

int traducir_bloque_inodo(struct INode *inode, unsigned int logical_block, unsigned char reserve);