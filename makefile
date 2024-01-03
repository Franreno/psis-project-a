CC = gcc
CFLAGS = -Wall -Wextra -pedantic -I./headers -I./shared
LDFLAGS = -lncurses -lzmq
SHARED_DIR = ./shared
OUTPUT_DIR = ./output
SERVER_DIR = ./server
LIZARD_CLIENT_DIR = ./lizard-client
ROACHES_CLIENT_DIR = ./roaches-client
WASPS_CLIENT_DIR = ./wasps-client
DISPLAY_APP_DIR = ./display-app

# Define the object files for each target using the appropriate directory for the main file
OBJS_LIZARDS_SERVER = $(SERVER_DIR)/lizardsNroachesNwasps-server.o $(SHARED_DIR)/util.o $(SHARED_DIR)/logger.o $(SHARED_DIR)/window.o $(SHARED_DIR)/lizard-mover.o $(SHARED_DIR)/roach-mover.o 	$(SHARED_DIR)/wasp-mover.o
OBJS_DISPLAY_APP = $(DISPLAY_APP_DIR)/display-app.o $(SHARED_DIR)/util.o $(SHARED_DIR)/logger.o $(SHARED_DIR)/window.o $(SHARED_DIR)/lizard-mover.o $(SHARED_DIR)/roach-mover.o $(SHARED_DIR)/wasp-mover.o
OBJS_LIZARD_CLIENT = $(LIZARD_CLIENT_DIR)/lizard-client.o $(SHARED_DIR)/logger.o
OBJS_ROACHES_CLIENT = $(ROACHES_CLIENT_DIR)/roaches-client.o $(SHARED_DIR)/logger.o
OBJS_WASPS_CLIENT = $(WASPS_CLIENT_DIR)/wasps-client.o $(SHARED_DIR)/logger.o

# Create the output directory if it doesn't exist
$(shell mkdir -p $(OUTPUT_DIR))

all: $(OUTPUT_DIR)/lizardsNroachesNwasps-server $(OUTPUT_DIR)/display-app $(OUTPUT_DIR)/lizard-client $(OUTPUT_DIR)/roaches-client $(OUTPUT_DIR)/wasps-client

# Link the final executables from their respective object files and shared object files
$(OUTPUT_DIR)/lizardsNroachesNwasps-server: $(OBJS_LIZARDS_SERVER)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(OUTPUT_DIR)/display-app: $(OBJS_DISPLAY_APP)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(OUTPUT_DIR)/lizard-client: $(OBJS_LIZARD_CLIENT)
	$(CC) $(CFLAGS) -o $@ $^ -lncurses -lzmq

$(OUTPUT_DIR)/roaches-client: $(OBJS_ROACHES_CLIENT)
	$(CC) $(CFLAGS) -o $@ $^ -lzmq

$(OUTPUT_DIR)/wasps-client: $(OBJS_WASPS_CLIENT)
	$(CC) $(CFLAGS) -o $@ $^ -lzmq

# Compile the ".c" files from the shared directory to ".o" objects
$(SHARED_DIR)/%.o: $(SHARED_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Compile the main modules located in their specific directories
$(SERVER_DIR)/%.o: $(SERVER_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(LIZARD_CLIENT_DIR)/%.o: $(LIZARD_CLIENT_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(ROACHES_CLIENT_DIR)/%.o: $(ROACHES_CLIENT_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(WASPS_CLIENT_DIR)/%.o: $(WASPS_CLIENT_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(DISPLAY_APP_DIR)/%.o: $(DISPLAY_APP_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@
# Clean up all the ".o" files and executables in the output directory
clean:
	rm -f $(SHARED_DIR)/*.o $(OUTPUT_DIR)/* $(SERVER_DIR)/*.o $(LIZARD_CLIENT_DIR)/*.o $(ROACHES_CLIENT_DIR)/*.o $(WASPS_CLIENT_DIR)/*.o $(DISPLAY_APP_DIR)/*.o

zip:
	zip -r project.zip . -x "*.zip"

.PHONY: all clean zip

