CC=gcc
CFLAGS=-c -g -Wall -std=gnu99

SOURCES=blocks.c basic_files.c files.c test/mi_mkfs.c test/leer_sf.c test/escribir.c test/leer.c test/truncar.c test/permitir.c# directorios.c mi_mkdir.c mi_chmod.c mi_ls.c mi_link.c mi_escribir.c mi_cat.c mi_stat.c mi_rm.c semaforo_mutex_posix.c #simulacion.c verificacion.c
LIBRARIES=blocks.o basic_files.o files.o# directorios.o semaforo_mutex_posix.o
INCLUDES=blocks.h basic_files.h files.h# directorios.h semaforo_mutex_posix.h #simulacion.h
PROGRAMS=test/mi_mkfs test/leer_sf test/escribir test/leer test/truncar test/permitir #mi_mkdir mi_chmod mi_ls mi_link mi_escribir mi_cat mi_stat mi_rm  #simulacion verificacion
OBJS=$(SOURCES:.c=.o)

all: $(OBJS) $(PROGRAMS)

$(PROGRAMS): $(LIBRARIES) $(INCLUDES)
	$(CC) $(LDFLAGS) $(LIBRARIES) $@.o -o $@

%.o: %.c $(INCLUDES)
	$(CC) $(CFLAGS) -o $@ -c $<

.PHONY: clean
clean:
	rm -rf *.o *~ $(PROGRAMS) disco* ext*
