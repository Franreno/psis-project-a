CC = gcc
CFLAGS = -Wall -Wextra -pedantic
LIBS = -lncurses -lzmq

all: lizardsNroaches-server roaches-client

lizardsNroaches-server.o: lizardsNroaches-server.c
	$(CC) $(CFLAGS) -c $< -o $@

roaches-client.o: roaches-client.c
	$(CC) $(CFLAGS) -c $< -o $@

lizardsNroaches-server: lizardsNroaches-server.o
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

roaches-client: roaches-client.o
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -f *.o lizardsNroaches-server roaches-client
