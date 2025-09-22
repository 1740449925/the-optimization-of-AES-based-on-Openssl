CC=gcc
CFLAGS= -o3 
SOURCES=$(wildcard src/*.c)
HEADERS=$(wildcard src/*.h)

all: aes

aes: $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) -o $@ src/aes.c

run: aes
	./aes

clean:
	rm aes

