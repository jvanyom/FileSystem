#include "ficheros_basico.h"

struct Metadata {
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
};

int mi_write_f(unsigned int inode_position, const void *buffer, unsigned int file_offset, unsigned int count);

int mi_read_f(unsigned int inode_position, void *buffer, unsigned int file_offset, unsigned int count);

int mi_stat_f(unsigned int inode_position, struct Metadata *metadata);

int mi_chmod_f(unsigned int inode_position, unsigned char permissions);