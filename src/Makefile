CC = gcc
CFLAGS = -O2
LDFLAGS =

all: sola

sola: main.o ulawapi.o solaapi.o
	$(CC) $(LDFLAGS) main.o ulawapi.o solaapi.o -lm -o sola

main.o: main.c typedef.h ulawapi.h solaapi.h
	$(CC) $(CFLAGS) -c main.c -o main.o

ulawapi.o: ulawapi.c typedef.h ulawapi.h
	$(CC) $(CFLAGS) -c ulawapi.c -o ulawapi.o

solaapi.o: solaapi.c typedef.h solaapi.h
	$(CC) $(CFLAGS) -c solaapi.c -o solaapi.o

clean:
	rm -rf main.o ulawapi.o solaapi.o sola
