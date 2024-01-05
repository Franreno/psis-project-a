CC = gcc
CFLAGS = -Wall -Wextra -pedantic -I./headers -I./shared -I./proto/compiled
LDFLAGS = -lncurses -lzmq -lprotobuf-c

SHARED_DIR = ./shared
OUTPUT_DIR = ./output
SERVER_DIR = ./server
PROTO_DIR = ./proto
LIZARD_CLIENT_DIR = ./lizard-client
ROACHES_CLIENT_DIR = ./roaches-client
WASPS_CLIENT_DIR = ./wasps-client
DISPLAY_APP_DIR = ./display-app
PROTO_DEFINITIONS_DIR = $(PROTO_DIR)/definitions
PROTO_COMPILED_DIR = $(PROTO_DIR)/compiled

# Define the object files for each target using the appropriate directory for the main file
OBJS_LIZARDS_SERVER = $(SERVER_DIR)/lizardsNroachesNwasps-server.o $(SHARED_DIR)/util.o $(SHARED_DIR)/logger.o $(SHARED_DIR)/window.o $(SHARED_DIR)/lizard-mover.o $(SHARED_DIR)/roach-mover.o 	$(SHARED_DIR)/wasp-mover.o $(SHARED_DIR)/proto-encoder.o
OBJS_DISPLAY_APP = $(DISPLAY_APP_DIR)/display-app.o $(SHARED_DIR)/util.o $(SHARED_DIR)/logger.o $(SHARED_DIR)/window.o $(SHARED_DIR)/lizard-mover.o $(SHARED_DIR)/roach-mover.o $(SHARED_DIR)/wasp-mover.o $(SHARED_DIR)/proto-encoder.o
OBJS_LIZARD_CLIENT = $(LIZARD_CLIENT_DIR)/lizard-client.o $(SHARED_DIR)/logger.o $(SHARED_DIR)/proto-encoder.o
OBJS_ROACHES_CLIENT = $(ROACHES_CLIENT_DIR)/roaches-client.o $(SHARED_DIR)/logger.o $(SHARED_DIR)/proto-encoder.o
OBJS_WASPS_CLIENT = $(WASPS_CLIENT_DIR)/wasps-client.o $(SHARED_DIR)/logger.o $(SHARED_DIR)/proto-encoder.o

PROTO_SOURCES := $(wildcard $(PROTO_DEFINITIONS_DIR)/*.proto)
PROTO_OBJECTS := $(patsubst $(PROTO_DEFINITIONS_DIR)/%.proto, $(PROTO_COMPILED_DIR)/%.pb-c.o, $(PROTO_SOURCES))

# Create the output directory if it doesn't exist
$(shell mkdir -p $(OUTPUT_DIR) $(PROTO_COMPILED_DIR))


all: protos $(OUTPUT_DIR)/lizardsNroachesNwasps-server $(OUTPUT_DIR)/display-app $(OUTPUT_DIR)/lizard-client $(OUTPUT_DIR)/roaches-client $(OUTPUT_DIR)/wasps-client

protos: $(PROTO_OBJECTS)

# Compile the Protocol Buffer files
$(PROTO_COMPILED_DIR)/%.pb-c.o: $(PROTO_DEFINITIONS_DIR)/%.proto
	protoc-c --c_out=$(PROTO_COMPILED_DIR) -I=$(PROTO_DEFINITIONS_DIR) $<
	$(CC) $(CFLAGS) -c $(PROTO_COMPILED_DIR)/$*.pb-c.c -o $(PROTO_COMPILED_DIR)/$*.pb-c.o


# Link the final executables from their respective object files and shared object files
$(OUTPUT_DIR)/lizardsNroachesNwasps-server: $(OBJS_LIZARDS_SERVER) $(PROTO_OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(OUTPUT_DIR)/display-app: $(OBJS_DISPLAY_APP) $(PROTO_OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(OUTPUT_DIR)/lizard-client: $(OBJS_LIZARD_CLIENT) $(PROTO_OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(OUTPUT_DIR)/roaches-client: $(OBJS_ROACHES_CLIENT) $(PROTO_OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(OUTPUT_DIR)/wasps-client: $(OBJS_WASPS_CLIENT) $(PROTO_OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

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

# Clean up all the ".o" files, executables in the output directory, and protobuf generated files
clean:
	rm -f $(SHARED_DIR)/*.o $(OUTPUT_DIR)/* $(SERVER_DIR)/*.o $(LIZARD_CLIENT_DIR)/*.o $(ROACHES_CLIENT_DIR)/*.o $(DISPLAY_APP_DIR)/*.o $(PROTO_COMPILED_DIR)/*.pb-c.o $(WASPS_CLIENT_DIR)/*.o

zip:
	zip -r project.zip . -x "*.zip"

.PHONY: all clean zip

