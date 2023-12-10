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

static char *s_recv(void *socket)
{
    enum
    {
        cap = 256
    };
    char buffer[cap];
    int size = zmq_recv(socket, buffer, cap - 1, 0);
    if (size == -1)
        return NULL;
    buffer[size < cap ? size : cap - 1] = '\0';

    return strndup(buffer, sizeof(buffer) - 1);
}

int create_and_connect_sockets(char *req_server_socket_address, char *sub_server_socket_address, void **context, void **requester, void **subscriber)
{
    // Create context
    *context = zmq_ctx_new();
    if (*context == NULL)
    {
        printf("Failed to create context: %s\n", zmq_strerror(errno));
        return -1;
    }

    // Create REQ socket to send messages to server
    *requester = zmq_socket(*context, ZMQ_REQ);
    if (*requester == NULL)
    {
        printf("Failed to create socket: %s\n", zmq_strerror(errno));
        zmq_ctx_destroy(*context);
        return -1;
    }

    // Create a subscriber socket to send messages to the display app
    *subscriber = zmq_socket(*context, ZMQ_SUB);
    if (*subscriber == NULL)
    {
        printf("Failed to create PUB socket: %s\n", zmq_strerror(errno));
        zmq_close(*requester);
        zmq_ctx_destroy(*context);
        return -1;
    }

    // Connect to the server using ZMQ_REQ
    if (zmq_connect(*requester, req_server_socket_address) != 0)
    {
        printf("Failed to connect: %s\n", zmq_strerror(errno));
        zmq_close(*requester);
        zmq_ctx_destroy(*context);
        return -1;
    }
    printf("Connected to server REQ\n");

    // Connect to the server using ZMQ_PUB
    if (zmq_connect(*subscriber, sub_server_socket_address) != 0)
    {
        printf("Failed to connect: %s\n", zmq_strerror(errno));
        zmq_close(*subscriber);
        zmq_ctx_destroy(*context);
        return -1;
    }

    return 0;
}

int main(int argc, char *argv[])
{
    // Initialize logger
    log_init("Display-app.log");
    void *context;
    void *requester;
    void *subscriber;
    char *req_server_socket_address;
    char *sub_server_socket_address;

    printf("Usage: ./lizardsNroaches-server <req_server_address> <req_server_port> <sub_server_address> <sub_server_port>\n");

    // Check if address and port were provided as command line arguments, if not, use default values
    if (argc != 5)
    {
        printf("Addresses and ports were not provided!\n");
        printf("Using default REP server socket address: %s\n", DEFAULT_SERVER_SOCKET_ADDRESS);
        req_server_socket_address = malloc(strlen(DEFAULT_SERVER_SOCKET_ADDRESS) + 1);
        req_server_socket_address = DEFAULT_SERVER_SOCKET_ADDRESS;
        printf("Using default PUB server socket address: %s\n", DEFAULT_SUBS_SERVER_SOCKET_ADDRESS);
        sub_server_socket_address = malloc(strlen(DEFAULT_SUBS_SERVER_SOCKET_ADDRESS) + 1);
        sub_server_socket_address = DEFAULT_SUBS_SERVER_SOCKET_ADDRESS;
    }
    else
    {
        printf("Using REQ address and port: %s %s\n", argv[1], argv[2]);
        char *req_address = argv[1];
        char *req_port = argv[2];
        req_server_socket_address = malloc(strlen("tcp://") + strlen(req_address) + strlen(":") + strlen(req_port) + 1);
        strcpy(req_server_socket_address, "tcp://");
        strcat(req_server_socket_address, req_address);
        strcat(req_server_socket_address, ":");
        strcat(req_server_socket_address, req_port);

        printf("Using SUB address and port: %s %s\n", argv[3], argv[4]);
        char *sub_address = argv[3];
        char *sub_port = argv[4];
        sub_server_socket_address = malloc(strlen("tcp://") + strlen(sub_address) + strlen(":") + strlen(sub_port) + 1);
        strcpy(sub_server_socket_address, "tcp://");
        strcat(sub_server_socket_address, sub_address);
        strcat(sub_server_socket_address, ":");
        strcat(sub_server_socket_address, sub_port);
    }

    // Create and connect sockets for clients to connect to
    if (create_and_connect_sockets(req_server_socket_address, sub_server_socket_address, &context, &requester, &subscriber) != 0)
        return -1;

    zmq_setsockopt(subscriber, ZMQ_SUBSCRIBE, "field_update_movement", 3);
    zmq_setsockopt(subscriber, ZMQ_SUBSCRIBE, "field_update_connect", 3);
    zmq_setsockopt(subscriber, ZMQ_SUBSCRIBE, "field_update_disconnect", 3);

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

    // ---------- START RECEIVE INITAL DATA FROM SERVER ----------

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

    // ---------- END RECEIVE INITAL DATA FROM SERVER ----------

    // ---------- START RECEIVE ROACH AND LIZARD MOVER DATA FROM SERVER ----------
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

    // ---------- END RECEIVE ROACH AND LIZARD MOVER DATA FROM SERVER ----------

    // Creates a window and draws a border
    window_data *game_window;
    window_init_with_matrix(&game_window, WINDOW_SIZE, WINDOW_SIZE, buffer);
    draw_entire_matrix(game_window);

    // Create a new window for the scores
    WINDOW *score_window = newwin(MAX_LIZARDS_ALLOWED, 50, 0, WINDOW_SIZE + 2);

    // Include not serialized components into the movers
    roach_payload->game_window = game_window;
    lizard_payload->game_window = game_window;
    lizard_payload->should_use_responder = 0;
    roach_payload->should_use_responder = 0;

    // Create dummy pointer to eaten roaches
    roach **eaten_roaches = (roach **)malloc(sizeof(roach *) * MAX_ROACHES_ALLOWED);
    int eaten_roaches_count = 0;

    roach_payload->eaten_roaches = eaten_roaches;
    roach_payload->amount_eaten_roaches = &eaten_roaches_count;
    lizard_payload->eaten_roaches = eaten_roaches;
    lizard_payload->amount_eaten_roaches = &eaten_roaches_count;
    roach_payload->lizards = lizard_payload->lizards;
    lizard_payload->roaches = roach_payload->roaches;

    while (1)
    {
        // Receive the field update
        char *type = s_recv(subscriber);
        // ---------- START PRINTING SCORES ----------
        // Print the scores in the score window
        int i, j;
        for (i = 0, j = 0; j < *lizard_payload->num_lizards; i++)
        {
            if (lizard_payload->lizards[i].ch == -1)
                continue;

            // j is the line number and only increments when a lizard is printed
            mvwprintw(score_window, j, 0, "Lizard id %c: Score %d", (char)lizard_payload->lizards[i].ch, lizard_payload->lizards[i].score);
            j++;

            // Clear the rest of the line
            wclrtoeol(score_window);
        }

        // Clear the remaining lines
        for (; j < MAX_LIZARDS_ALLOWED; j++)
        {
            wmove(score_window, j, 0);
            wclrtoeol(score_window);
        }

        // Update the score window
        wrefresh(score_window);

        // ---------- END PRINTING SCORES ----------

        // ---------- START RECEIVE FIELD UPDATE FROM SERVER ----------

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
                int should_process_movement = refresh_eaten_roach_for_display(roach_payload, field_update_message->new_x, field_update_message->new_y, field_update_message->is_eaten);
                if (should_process_movement)
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
            message_to_server recv_message = field_update_message->message;
            roach_payload->recv_message = &recv_message;
            lizard_payload->recv_message = &recv_message;
            switch (field_update_message->message.client_id)
            {
            case LIZARD:
                log_write("Processing lizard connect message\n");
                process_lizard_inject_connect(lizard_payload, field_update_message->connected_lizard, field_update_message->position_in_array);
                break;
            case ROACH:
                log_write("Processing roach connect message\n");
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
            message_to_server recv_message = field_update_message->message;
            roach_payload->recv_message = &recv_message;
            lizard_payload->recv_message = &recv_message;
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
            free(field_update_message);
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