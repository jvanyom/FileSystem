CC=gcc
CFLAGS=-c -g -Wall -std=gnu99

SOURCES=../errors.c ../blocks.c ../basic_files.c ../files.c ../directories.c mi_mkfs.c leer_sf.c escribir.c leer.c truncar.c permitir.c# mi_mkdir.c mi_chmod.c mi_ls.c mi_link.c mi_escribir.c mi_cat.c mi_stat.c mi_rm.c semaforo_mutex_posix.c #simulacion.c verificacion.c
LIBRARIES=../errors.o ../blocks.o ../basic_files.o ../files.o ../directories.o# semaforo_mutex_posix.o
INCLUDES=../errors.h ../blocks.h ../basic_files.h ../files.h ../directories.h# semaforo_mutex_posix.h #simulacion.h
PROGRAMS=mi_mkfs leer_sf escribir leer truncar permitir #mi_mkdir mi_chmod mi_ls mi_link mi_escribir mi_cat mi_stat mi_rm  #simulacion verificacion
OBJS=$(SOURCES:.c=.o)

all: $(OBJS) $(PROGRAMS)

$(PROGRAMS): $(LIBRARIES) $(INCLUDES)
	$(CC) $(LDFLAGS) $(LIBRARIES) $@.o -o $@

%.o: %.c $(INCLUDES)
	$(CC) $(CFLAGS) -o $@ -c $<

.PHONY: clean
clean:
	rm -rf *.o ../*.o *~ $(PROGRAMS) disco* ext*
