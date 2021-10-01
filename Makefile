CC=gcc -Wall

PROGS=server client

all: $(PROGS)

clean:
	rm -f $(PROGS)

server: server.c
	$(CC) $< -o $@
	
client: client.c
	$(CC) $< -o $@
	
