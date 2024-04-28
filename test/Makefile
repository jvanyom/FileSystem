CC=gcc
CFLAGS=-c -g -Wall -std=gnu99

SOURCES=../my_mkfs.c ../my_mkdir.c ../my_touch.c ../my_chmod.c ../my_ls.c ../my_stat.c ../errors.c ../blocks.c ../basic_files.c ../files.c ../directories.c leer_sf.c escribir.c leer.c truncar.c permitir.c #../my_link.c ../my_escribir.c ../my_cat.c ../my_rm.c semaforo_mutex_posix.c #simulacion.c verificacion.c
LIBRARIES=../errors.o ../blocks.o ../basic_files.o ../files.o ../directories.o # semaforo_mutex_posix.o
INCLUDES=../errors.h ../blocks.h ../basic_files.h ../files.h ../directories.h # semaforo_mutex_posix.h #simulacion.h
PROGRAMS=../my_mkfs ../my_mkdir ../my_touch ../my_chmod ../my_ls ../my_stat leer_sf escribir leer truncar permitir #../my_link ../my_cat ../my_rm my_escribir  #simulacion verificacion
OBJS=$(SOURCES:.c=.o)

all: $(OBJS) $(PROGRAMS)

$(PROGRAMS): $(LIBRARIES) $(INCLUDES)
	$(CC) $(LDFLAGS) $(LIBRARIES) $@.o -o $@

%.o: %.c $(INCLUDES)
	$(CC) $(CFLAGS) -o $@ -c $<

.PHONY: clean
clean:
	rm -rf *.o ../*.o *~ $(PROGRAMS) ../disco* disco* ext*
