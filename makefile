CC = gcc
CFLAGS = -Wall -Wextra -pedantic
LIBS = -lncurses -lzmq

all: server roaches-client

server.o: server.c
	$(CC) $(CFLAGS) -c $< -o $@

roaches-client.o: roaches-client.c
	$(CC) $(CFLAGS) -c $< -o $@

server: server.o
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

roaches-client: roaches-client.o
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -f *.o server roaches-client
