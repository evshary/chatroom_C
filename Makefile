GCC  = gcc
LIB = -lpthread
PROG = client server

client: client.c
	$(GCC) $@.c -o $@ $(LIB)

server: server.c
	$(GCC) $@.c -o $@ $(LIB)

all: client server

clean:
	rm $(PROG)
