CC=gcc
CFLAGS=-c -g -Wall -std=gnu99

SOURCES=../my_mkfs.c ../my_mkdir.c ../my_touch.c ../my_chmod.c ../my_ls.c ../my_stat.c ../my_write.c ../my_cat.c ../my_link.c ../my_rm.c ../my_rmdir.c ../my_rn.c ../my_mv.c ../my_cp.c ../errors.c ../blocks.c ../basic_files.c ../files.c ../directories.c ../dcache.c leer_sf.c escribir.c leer.c truncar.c permitir.c test_cache.c batch_write.c #semaforo_mutex_posix.c simulacion.c verificacion.c
LIBRARIES=../errors.o ../blocks.o ../basic_files.o ../files.o ../directories.o ../dcache.o # semaforo_mutex_posix.o
INCLUDES=../errors.h ../blocks.h ../basic_files.h ../files.h ../directories.h ../dcache.h # semaforo_mutex_posix.h simulacion.h
PROGRAMS=../my_mkfs ../my_mkdir ../my_touch ../my_chmod ../my_ls ../my_stat ../my_write ../my_cat ../my_link ../my_rm ../my_rmdir ../my_rn ../my_mv ../my_cp leer_sf escribir leer truncar permitir test_cache batch_write #simulacion verificacion
OBJS=$(SOURCES:.c=.o)

all: $(OBJS) $(PROGRAMS)

$(PROGRAMS): $(LIBRARIES) $(INCLUDES)
	$(CC) $(LDFLAGS) $(LIBRARIES) $@.o -o $@

%.o: %.c $(INCLUDES)
	$(CC) $(CFLAGS) -o $@ -c $<

.PHONY: clean
clean:
	rm -rf *.o ../*.o *~ $(PROGRAMS) ../disco* disco* ext*
