CC = gcc
CFLAGS = -Wall -Wextra -pedantic -I./headers
LDFLAGS = -lncurses -lzmq
SRC_DIR = ./src
OUTPUT_DIR = ./output

# Define the object files for each target
OBJS_LIZARDS_SERVER = $(SRC_DIR)/lizardsNroaches-server.o $(SRC_DIR)/util.o $(SRC_DIR)/logger.o $(SRC_DIR)/window.o $(SRC_DIR)/lizard-mover.o $(SRC_DIR)/roach-mover.o
OBJS_DISPLAY_APP = $(SRC_DIR)/display-app.o $(SRC_DIR)/util.o $(SRC_DIR)/logger.o $(SRC_DIR)/window.o $(SRC_DIR)/lizard-mover.o $(SRC_DIR)/roach-mover.o
OBJS_LIZARD_CLIENT = $(SRC_DIR)/lizard-client.o $(SRC_DIR)/logger.o
OBJS_ROACHES_CLIENT = $(SRC_DIR)/roaches-client.o $(SRC_DIR)/logger.o

# Create the output directory if it doesn't exist
$(shell mkdir -p $(OUTPUT_DIR))

all: $(OUTPUT_DIR)/lizardsNroaches-server $(OUTPUT_DIR)/display-app $(OUTPUT_DIR)/lizard-client $(OUTPUT_DIR)/roaches-client

$(OUTPUT_DIR)/lizardsNroaches-server: $(OBJS_LIZARDS_SERVER)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(OUTPUT_DIR)/display-app: $(OBJS_DISPLAY_APP)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(OUTPUT_DIR)/lizard-client: $(OBJS_LIZARD_CLIENT)
	$(CC) $(CFLAGS) -o $@ $^ -lncurses -lzmq

$(OUTPUT_DIR)/roaches-client: $(OBJS_ROACHES_CLIENT)
	$(CC) $(CFLAGS) -o $@ $^ -lzmq

# Compile the ".c" files in the src directory to ".o" objects
$(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up all the ".o" files and executables in the output directory
clean:
	rm -f $(SRC_DIR)/*.o $(OUTPUT_DIR)/*

.PHONY: all clean
