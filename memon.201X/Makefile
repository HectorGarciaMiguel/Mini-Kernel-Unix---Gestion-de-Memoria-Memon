#
# memon/Makefile
#       Makefile general de la práctica
#

CC=cc
CFLAGS=-Wall -g

all: version pruebas memon

version:
	@ln -sf solucion_`getconf LONG_BIT` solucion
	@ln -sf apoyo.o_`getconf LONG_BIT` apoyo.o

fallo.o mapa.o marcos.o: mapa.h

memon: memon.o apoyo.o mapa.o fallo.o marcos.o
	$(CC) -o $@ memon.o apoyo.o mapa.o fallo.o marcos.o -ldl

pruebas:
	cd programas; make

clean:
	rm -f memon.o mapa.o marcos.o fallo.o memon solucion apoyo.o
	cd programas; make clean
