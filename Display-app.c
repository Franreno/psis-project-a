#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>

#include <zmq.h>

#include <ncurses.h>

#include "remote-char.h"
#include "logger.h"
#include "window.h"
#include "roach-mover.h"
#include "lizard-mover.h"
#include "zhelpers.h"

int main()
{
    // Initialize logger
    log_init("Display-app.log");

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
    void *requester = zmq_socket(context, ZMQ_REQ);
    if (requester == NULL)
    {
        printf("Failed to create socket: %s\n", zmq_strerror(errno));
        zmq_ctx_destroy(context);
        return 1;
    }
    // Create a subscriber socket to send messages to the display app
    void *subscriber = zmq_socket(context, ZMQ_SUB);
    if (subscriber == NULL)
    {
        printf("Failed to create PUB socket: %s\n", zmq_strerror(errno));
        zmq_close(requester);
        zmq_ctx_destroy(context);
        return 1;
    }
    // Connect to the server using ZMQ_REQ
    if (zmq_connect(requester, server_socket_address) != 0)
    {
        printf("Failed to connect: %s\n", zmq_strerror(errno));
        zmq_close(requester);
        zmq_ctx_destroy(context);
        return 1;
    }
    // Connect to the server using ZMQ_PUB
    if (zmq_connect(subscriber, "tcp://127.0.0.1:5556") != 0)
    {
        printf("Failed to connect: %s\n", zmq_strerror(errno));
        zmq_close(subscriber);
        zmq_ctx_destroy(context);
        return 1;
    }

    zmq_setsockopt(subscriber, ZMQ_SUBSCRIBE, "field_update_movement", 3);
    zmq_setsockopt(subscriber, ZMQ_SUBSCRIBE, "field_update_connect", 3);

    // Initialize ncurses
    initscr();
    cbreak();
    keypad(stdscr, TRUE);
    noecho();

    // Connect to the server and get the matrix information from it
    message_to_server send_message;
    send_message.client_id = DISPLAY_APP;
    send_message.type = CONNECT;
    zmq_send(requester, &send_message, sizeof(message_to_server), 0);

    // Buffer to hold the received data
    size_t buffer_size;
    zmq_recv(requester, &buffer_size, sizeof(buffer_size), 0);

    // Allocate the buffer based on the received size
    char *buffer = malloc(buffer_size);
    if (!buffer)
    {
        exit(1);
    }

    // First, receive the size of the incoming message
    zmq_recv(requester, buffer, buffer_size, 0);

    // Receive the roach mover data
    size_t roach_mover_buffer_size;
    zmq_recv(requester, &roach_mover_buffer_size, sizeof(roach_mover_buffer_size), 0);

    // Allocate the buffer based on the received size
    char *roach_mover_buffer = malloc(roach_mover_buffer_size);
    if (!roach_mover_buffer)
    {
        exit(1);
    }

    zmq_recv(requester, roach_mover_buffer, roach_mover_buffer_size, 0);

    // Deserialize the roach mover from the buffer
    roach_mover *roach_payload = malloc(sizeof(roach_mover));
    deserialize_roach_mover(roach_payload, roach_mover_buffer);

    // Receive the lizard mover data
    size_t lizard_mover_buffer_size;
    zmq_recv(requester, &lizard_mover_buffer_size, sizeof(lizard_mover_buffer_size), 0);

    // Allocate the buffer based on the received size
    char *lizard_mover_buffer = malloc(lizard_mover_buffer_size);
    if (!lizard_mover_buffer)
    {
        exit(1);
    }

    zmq_recv(requester, lizard_mover_buffer, lizard_mover_buffer_size, 0);

    // Deserialize the lizard mover from the buffer
    lizard_mover *lizard_payload = malloc(sizeof(lizard_mover));
    deserialize_lizard_mover(lizard_payload, lizard_mover_buffer);

    // Creates a window and draws a border
    window_data *game_window;
    window_init_with_matrix(&game_window, WINDOW_SIZE, WINDOW_SIZE, buffer);
    draw_entire_matrix(game_window);

    // Include not serialized components into the movers
    roach_payload->game_window = game_window;
    lizard_payload->game_window = game_window;
    lizard_payload->should_use_responder = 0;
    roach_payload->should_use_responder = 0;

    while (1)
    {
        // Receive the field update
        char *type = s_recv(subscriber);

        if (strcmp(type, "field_update_movement") == 0)
        {
            log_write("Received field update movement\n");
            field_update_movement *field_update_message = malloc(sizeof(field_update_movement));
            zmq_recv(subscriber, field_update_message, sizeof(field_update_movement), 0);

            // Parse the field_update_message
            message_to_server recv_message = field_update_message->message;
            // Point movers to the message received
            roach_payload->recv_message = &recv_message;
            lizard_payload->recv_message = &recv_message;

            switch (recv_message.client_id)
            {
            case LIZARD:
                log_write("Processing lizard message\n");
                process_lizard_movement(lizard_payload);
                break;
            case ROACH:
                log_write("Processing roach message\n");
                process_roach_movement(roach_payload);
                break;
            }

            free(field_update_message);
        }
        else if (strcmp(type, "field_update_connect") == 0)
        {
            log_write("Received field update connect\n");
            field_update_connect *field_update_message = malloc(sizeof(field_update_connect));
            zmq_recv(subscriber, field_update_message, sizeof(field_update_connect), 0);
            switch (field_update_message->message.client_id)
            {
            case LIZARD:
                log_write("Processing lizard message\n");
                process_lizard_inject_connect(lizard_payload, field_update_message->connected_lizard, field_update_message->position_in_array);
                break;
            case ROACH:
                log_write("Processing roach message\n");
                process_roach_inject_connect(roach_payload, field_update_message->connected_roach, field_update_message->position_in_array);
                break;
            }

            free(field_update_message);
        }
        else if (strcmp(type, "field_update_disconnect") == 0)
        {
            log_write("Received field update disconnect\n");
            field_update_disconnect *field_update_message = malloc(sizeof(field_update_disconnect));
            zmq_recv(subscriber, field_update_message, sizeof(field_update_disconnect), 0);
            switch (field_update_message->message.client_id)
            {
            case LIZARD:
                log_write("Processing lizard message\n");
                process_lizard_disconnect(lizard_payload);
                break;
            case ROACH:
                log_write("Processing roach message\n");
                process_roach_disconnect(roach_payload);
                break;
            }
        }
        free(type);
    }

    endwin();
    zmq_close(requester);
    zmq_close(subscriber);
    zmq_ctx_destroy(context);
    log_close();

    return 0;
}