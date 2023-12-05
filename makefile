CC = gcc
CFLAGS = -Wall -Wextra -pedantic
LIBS = -lncurses -lzmq

# Define .o files for lizardsNroaches-server and roaches-client that depend on logger.o
OBJ_SERVER = lizardsNroaches-server.o logger.o window.o
OBJ_CLIENT = roaches-client.o logger.o
OBJ_DISPLAY = Display-app.o logger.o window.o

all: lizardsNroaches-server roaches-client Display-app

# Link the server and client executables with logger.o
lizardsNroaches-server: $(OBJ_SERVER)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

roaches-client: $(OBJ_CLIENT)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

Display-app: $(OBJ_DISPLAY)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

# Compile the .o files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up the .o files and executables
clean:
	rm -f *.o lizardsNroaches-server roaches-client Display-app
