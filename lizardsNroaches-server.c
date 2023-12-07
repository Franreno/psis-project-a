#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <stdio.h>

#include <zmq.h>

#include <ncurses.h>

#include "remote-char.h"
#include "logger.h"
#include "window.h"
#include "roach-mover.h"
#include "lizard-mover.h"
#include "util.h"

void print_constants()
{
    printf("The server is running with the following parameters:\n");
    printf("WINDOW_SIZE: %d\n", WINDOW_SIZE);
    printf("ROACH_MOVE_CHANCE: %d\n", ROACH_MOVE_CHANCE);
    printf("MAX_ROACH_SCORE: %d\n", MAX_ROACH_SCORE);
    printf("MAX_ROACHES_GENERATED: %d\n", MAX_ROACHES_GENERATED);
    printf("MAX_ROACHES_ALLOWED: %d\n", MAX_ROACHES_ALLOWED);
    printf("MAX_LIZARDS_ALLOWED: %d\n", MAX_LIZARDS_ALLOWED);
}

void process_display_app_message(void *responder, window_data *data)
{
    // Serialize the window matrix
    char *serialized_data;
    size_t serialized_size;
    serialize_window_matrix(data->matrix, &serialized_data, &serialized_size);

    if (!serialized_data)
    {
        // Handle memory allocation error
        // Optionally, send an error message back to the client
        int error_code = -1; // Example error code
        zmq_send(responder, &error_code, sizeof(error_code), 0);
        return;
    }

    // Send the serialized data
    zmq_send(responder, &serialized_size, sizeof(serialized_size), ZMQ_SNDMORE);
    zmq_send(responder, serialized_data, serialized_size, 0);

    // Cleanup
    free(serialized_data);
}

int main()
{
    // Print the parameters the server is running with
    print_constants(); // NOT PRINTING ??

    // Initialize logger
    log_init("server.log");

    // Create socket address
    char *address = DEFAULT_SERVER_ADDRESS;
    char *port = DEFAULT_SERVER_PORT;
    char *server_socket_address = malloc(sizeof(char) * (strlen("tcp://") + strlen(address) + strlen(":") + strlen(port) + 1));
    strcpy(server_socket_address, "tcp://");
    strcat(server_socket_address, address);
    strcat(server_socket_address, ":");
    strcat(server_socket_address, port);
    server_socket_address = "ipc:///tmp/server"; // WORKAROUND
    printf("Connecting to server at %s\n", server_socket_address);

    // Create context
    void *context = zmq_ctx_new();
    if (context == NULL)
    {
        printf("Failed to create context: %s\n", zmq_strerror(errno));
        return 1;
    }

    // Create REP socket to receive messages from clients
    void *responder = zmq_socket(context, ZMQ_REP);
    if (responder == NULL)
    {
        printf("Failed to create REP socket: %s\n", zmq_strerror(errno));
        zmq_ctx_destroy(context);
        return 1;
    }
    // Create a publisher socket to send messages to the display app
    void *publisher = zmq_socket(context, ZMQ_PUB);
    if (publisher == NULL)
    {
        printf("Failed to create PUB socket: %s\n", zmq_strerror(errno));
        zmq_close(responder);
        zmq_ctx_destroy(context);
        return 1;
    }

    // Bind to the REP socket
    if (zmq_bind(responder, server_socket_address) != 0)
    {
        printf("Failed to bind REP socket: %s\n", zmq_strerror(errno));
        zmq_close(responder);
        zmq_close(publisher);
        zmq_ctx_destroy(context);
        return 1;
    }

    // Initialize ncurses
    initscr();
    cbreak();
    keypad(stdscr, TRUE);
    noecho();

    // Creates a window and draws a border
    window_data *game_window;
    window_init(&game_window, WINDOW_SIZE, WINDOW_SIZE);

    // Initialize variables used for roach tracking
    int num_roaches = 0;
    int slot_roaches = MAX_ROACHES_ALLOWED;
    roach roaches[MAX_ROACHES_ALLOWED];

    // Initialize variables used for lizard tracking
    int num_lizards = 0;
    int slot_lizards = MAX_LIZARDS_ALLOWED;
    lizard lizards[MAX_LIZARDS_ALLOWED];

    message_to_server recv_message;

    // Seed random number generator
    srand(time(NULL));

    roach_mover *roach_payload = malloc(sizeof(roach_mover));
    log_write("Creating roach mover\n");
    new_roach_mover(&roach_payload, &recv_message, roaches, responder, &num_roaches, &slot_roaches, game_window);

    log_write("Creating lizard mover\n");
    lizard_mover *lizard_payload = malloc(sizeof(lizard_mover));
    new_lizard_mover(&lizard_payload, &recv_message, lizards, responder, &num_lizards, &slot_lizards, game_window);

    while (1)
    {
        // Receive message from one of the clients
        zmq_recv(responder, &recv_message, sizeof(message_to_server), 0);
        log_write("Received message from client %d\n", recv_message.client_id);

        switch (recv_message.client_id)
        {
        case LIZARD:
            log_write("Processing lizard message\n");
            process_lizard_message(lizard_payload);
            break;
        case ROACH:
            log_write("Processing roach message\n");
            process_roach_message(roach_payload);
            break;
        case DISPLAY_APP:
            log_write("Processing display app message\n");
            process_display_app_message(responder, game_window);
            break;
        default:
            break;
        }

        // Publish the message to the display app
    }

    endwin();
    zmq_close(responder);
    zmq_ctx_destroy(context);
    log_close();

    return 0;
}