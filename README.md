# FileSystem

Sistema de archivos escrito en C e inspirado en conceptos de ext2. Utiliza un archivo ordinario como dispositivo de almacenamiento: dentro de él guarda sus propios metadatos, directorios y contenidos. Las herramientas de línea de comandos permiten formatear ese disco virtual y trabajar con los archivos que contiene.

El proyecto permite estudiar cómo se relacionan las operaciones sobre rutas con la gestión de inodos y bloques. Implementa un formato propio; las imágenes generadas no son imágenes ext2 compatibles con las herramientas del sistema operativo, ni se montan como un sistema de archivos del kernel.

## Funcionalidades

- Creación de discos virtuales, archivos y directorios.
- Lectura y escritura a partir de un desplazamiento en bytes, con reserva de bloques según se necesitan.
- Consulta de metadatos: tipo, permisos, tamaño lógico, bloques ocupados, número de enlaces y fechas de acceso y modificación.
- Cambio de permisos, enlaces duros entre archivos y eliminación de entradas.
- Renombrado, movimiento, copia de archivos y directorios, y eliminación recursiva de directorios.
- Truncado de archivos mediante la API de inodos y la utilidad de prueba correspondiente.
- Caché de rutas a inodos, con implementaciones FIFO y LRU; la configuración incluida utiliza LRU y un máximo de tres entradas por proceso.

## Organización del código

| Archivos | Responsabilidad |
| --- | --- |
| `blocks.c`, `blocks.h` | Apertura y cierre del dispositivo, y lectura y escritura de bloques físicos. La configuración actual utiliza `mmap`; también hay una alternativa con `lseek`, `read` y `write`. |
| `basic_files.c`, `basic_files.h` | Superbloque, mapa de bits, reserva y liberación de bloques e inodos, y traducción de bloques lógicos a físicos. |
| `files.c`, `files.h` | Operaciones sobre inodos: lectura, escritura, metadatos, permisos y truncado. |
| `directories.c`, `directories.h` | Resolución de rutas, entradas de directorio, enlaces y operaciones sobre archivos y directorios. |
| `dcache.c`, `dcache.h` | Caché en memoria para búsquedas de rutas. |
| `errors.c`, `errors.h` | Códigos de error y mensajes de diagnóstico. |
| `my_*.c` | Programas de línea de comandos que exponen las operaciones del sistema de archivos. |
| `test/` | Makefile, utilidades de inspección y scripts con escenarios de prueba. |

### Estructura del disco virtual

El archivo que representa el disco se divide en bloques de **1024 bytes**:

```text
Superbloque | Mapa de bits | Tabla de inodos | Bloques de datos
```

El superbloque registra la ubicación de las zonas, los contadores de espacio libre y el inodo raíz. El mapa de bits indica qué bloques están ocupados. Los inodos tienen un tamaño definido de **128 bytes** y contienen metadatos, **12 punteros directos** y tres punteros indirectos: simple, doble y triple.

Los directorios almacenan entradas que relacionan un nombre con un número de inodo. Al formatear, se reserva una cantidad de inodos equivalente a la cuarta parte del número de bloques y se crea el directorio raíz `/`.

## Compilación

Se necesita un entorno Linux/POSIX con **GCC**, **GNU Make** y **Bash**. El Makefile utiliza C con extensiones GNU (`-std=gnu99`). En Windows puede utilizarse una distribución Linux mediante WSL con esas herramientas instaladas.

Desde la raíz del repositorio:

```bash
make -C test
```

Los ejecutables `my_*` se generan en la raíz y las utilidades auxiliares en `test/`.

También se incluye `bash build.sh`, que limpia y recompila ocultando la salida estándar. Para ver los mensajes de compilación, utiliza directamente el comando anterior. La regla `clean` elimina además imágenes cuyos nombres coinciden con `disco*` en la raíz y en `test/`, y con `ext*` dentro de `test/`.

## Ejemplo de uso

Ejecuta los siguientes comandos desde la raíz del repositorio. `disco_demo` es el archivo del sistema anfitrión que contendrá el disco virtual; `/documentos/nota.txt` es una ruta interna de ese disco.

```bash
# Crear y formatear un disco de 100000 bloques (102400000 bytes).
./my_mkfs disco_demo 100000

# Crear un directorio y un archivo con permisos de lectura y escritura.
./my_mkdir disco_demo 6 /documentos/
./my_touch disco_demo 6 /documentos/nota.txt

# Escribir desde el byte 0 y recuperar el contenido.
./my_write disco_demo /documentos/nota.txt "Hola, FileSystem" 0
./my_cat disco_demo /documentos/nota.txt

# Consultar el directorio y los metadatos del archivo.
./my_ls -l disco_demo /documentos/
./my_stat disco_demo /documentos/nota.txt

# Crear un segundo nombre para el mismo inodo.
./my_link disco_demo /documentos/nota.txt /documentos/enlace.txt

# Eliminar el enlace, el archivo original y el directorio vacío.
./my_rm disco_demo /documentos/enlace.txt
./my_rm disco_demo /documentos/nota.txt
./my_rmdir disco_demo /documentos/
```

`my_mkfs` sobrescribe el archivo indicado al formatear. Utiliza una imagen de prueba. Los directorios padre deben existir antes de crear sus entradas y las rutas internas deben ser absolutas, comenzando por `/`.

Los permisos son un único número de **0 a 7** que combina lectura (`4`), escritura (`2`) y ejecución (`1`): por ejemplo, `6` representa lectura y escritura. No hay permisos separados para propietario, grupo y otros. El bit de ejecución se almacena y se muestra, pero no implementa la ejecución de programas.

## Comandos disponibles

En esta tabla, `disco` es la imagen del sistema de archivos. Los argumentos que representan rutas de archivos o directorios pertenecen a esa imagen.

| Sintaxis | Operación |
| --- | --- |
| `./my_mkfs disco bloques` | Crear y formatear el disco virtual. |
| `./my_mkdir disco permisos /directorio/` | Crear un directorio. |
| `./my_touch disco permisos /archivo` | Crear un archivo nuevo. |
| `./my_write disco /archivo "texto" offset` | Escribir texto desde el desplazamiento indicado, en bytes. |
| `./my_cat disco /archivo` | Mostrar el contenido del archivo. |
| `./my_ls [-l] disco /ruta` | Listar entradas; `-l` añade metadatos. |
| `./my_stat disco /ruta` | Mostrar el número de inodo y sus metadatos. |
| `./my_chmod disco permisos /ruta` | Cambiar los permisos. |
| `./my_link disco /archivo /enlace` | Crear un enlace duro a un archivo existente. |
| `./my_rn disco /ruta nuevo_nombre` | Cambiar el nombre dentro del mismo directorio. |
| `./my_mv disco /origen /destino/` | Mover una entrada a un directorio existente. |
| `./my_cp disco /origen /destino/` | Copiar un archivo o un directorio y su contenido a un directorio existente. |
| `./my_rm disco /archivo` | Eliminar una entrada de archivo y liberar su inodo cuando ya no tenga enlaces. |
| `./my_rm -r disco /directorio/` | Eliminar un directorio y su contenido recursivamente. |
| `./my_rmdir disco /directorio/` | Eliminar un directorio vacío. |

Las opciones `-l` y `-r` se escriben **antes del nombre del disco**. `my_rn` recibe un nombre nuevo; `my_mv` y `my_cp` reciben un directorio de destino.

## Pruebas e inspección

La carpeta `test/` incluye programas para examinar el superbloque (`leer_sf`), leer y escribir por inodo, modificar permisos, truncar y ejercitar la caché. También contiene escenarios para enlaces, copias, movimientos, renombrado y eliminación recursiva.

Los scripts usan rutas relativas y deben ejecutarse desde `test/`. Por ejemplo:

```bash
cd test
bash test10.sh
```

`test10.sh` recompila, crea una imagen llamada `disco` y muestra operaciones sobre enlaces y eliminación de entradas, incluidos casos que provocan errores intencionadamente. Los scripts sirven para inspeccionar el comportamiento; su salida debe revisarse y no constituye por sí sola una batería de comprobaciones automáticas. Varios scripts ejecutan `make clean` y vuelven a crear las imágenes de prueba.

## Alcance

El formato persiste estructuras de C directamente, por lo que depende de la representación de tipos y de la arquitectura utilizada. No incorpora un mecanismo de journaling ni sincronización para operaciones concurrentes entre procesos. Está orientado a explorar la implementación de un sistema de archivos mediante imágenes de prueba.
