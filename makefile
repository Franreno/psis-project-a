CC = gcc
CFLAGS = -Wall -Wextra -pedantic

all: lizardsNroaches-server Display-app lizard-client roaches-client

lizardsNroaches-server: lizardsNroaches-server.o logger.o window.o
	$(CC) $(CFLAGS) -o $@ $^ -lncurses -lzmq

Display-app: Display-app.o logger.o window.o
	$(CC) $(CFLAGS) -o $@ $^ -lncurses -lzmq

lizard-client: lizard-client.o logger.o
	$(CC) $(CFLAGS) -o $@ $^ -lncurses -lzmq

roaches-client: roaches-client.o logger.o
	$(CC) $(CFLAGS) -o $@ $^ -lzmq

# Compile all the ".o" files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up all the ".o" and executables files
clean:
	rm -f *.o lizardsNroaches-server Display-app lizard-client roaches-client
