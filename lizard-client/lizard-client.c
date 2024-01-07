#include "default_consts.h"
#include "proto-encoder.h"
#include "window.h"
#include "util.h"

typedef struct
{
    void *subscriber;
    window_data *game_window;
    WINDOW *score_window;

} display_args;

/**
 * @brief Create a and connect sockets object
 *
 * @param req_server_socket_address - Address of the server socket to connect to
 * @param sub_server_socket_address - Address of the server socket to connect to
 * @param context - ZMQ context
 * @param requester - ZMQ REQ socket
 * @param subscriber - ZMQ SUB socket
 * @return int - 0 if successful, -1 otherwise
 */
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
    // In your create_and_connect_sockets function:
    int rc = zmq_setsockopt(*requester, ZMQ_CURVE_SECRETKEY, CLIENT_SECRET_KEY, 40);
    if (rc != 0)
    {
        printf("Failed to set ZMQ_CURVE_SECRETKEY: %s\n", zmq_strerror(errno));
        zmq_close(*requester);
        zmq_ctx_destroy(*context);
        return -1;
    }
    rc = zmq_setsockopt(*requester, ZMQ_CURVE_PUBLICKEY, CLIENT_PUBLIC_KEY, 40);
    if (rc != 0)
    {
        printf("Failed to set ZMQ_CURVE_PUBLICKEY: %s\n", zmq_strerror(errno));
        zmq_close(*requester);
        zmq_ctx_destroy(*context);
        return -1;
    }

    rc = zmq_setsockopt(*requester, ZMQ_CURVE_SERVERKEY, SERVER_PUBLIC_KEY, 40);
    if (rc != 0)
    {
        printf("Failed to set ZMQ_CURVE_SERVERKEY: %s\n", zmq_strerror(errno));
        zmq_close(*requester);
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

    zmq_setsockopt(*subscriber, ZMQ_CURVE_SECRETKEY, CLIENT_SECRET_KEY, 40);
    zmq_setsockopt(*subscriber, ZMQ_CURVE_PUBLICKEY, CLIENT_PUBLIC_KEY, 40);
    zmq_setsockopt(*subscriber, ZMQ_CURVE_SERVERKEY, SERVER_PUBLIC_KEY, 40);

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

int connect_lizard(void *requester, message_to_server *send_message)
{
    printf("Starting connect_lizard function\n");

    int lizard_id;

    send_message->client_id = LIZARD;
    send_message->type = CONNECT;
    send_message->value = CONNECT;

    // Send a message to the server to connect a lizard
    printf("Attempting to connect lizard\n");

    // Start of proto encoder
    MessageToServerProto *message_to_server_proto = malloc(sizeof(MessageToServerProto));
    message_to_server_proto__init(message_to_server_proto);

    // Convert message to server to proto message to server
    printf("Converting message to server to proto message to server\n");
    message_to_server_to_proto_message_to_server(message_to_server_proto, send_message);

    // Get the size of the serialized message
    printf("Getting the size of the serialized message\n");
    size_t message_to_server_proto_size = message_to_server_proto__get_packed_size(message_to_server_proto);

    // Serialize the message
    printf("Serializing the message\n");
    void *message_to_server_proto_buffer = malloc(message_to_server_proto_size);
    message_to_server_proto__pack(message_to_server_proto, message_to_server_proto_buffer);

    // Send the message
    printf("Sending the message\n");
    zmq_send(requester, message_to_server_proto_buffer, message_to_server_proto_size, 0);

    // Free the serialized message
    printf("Freeing the serialized message\n");
    free(message_to_server_proto_buffer);
    message_to_server_proto__free_unpacked(message_to_server_proto, NULL);

    // Server replies with either failure or the assigned lizard id
    printf("Receiving server reply\n");
    zmq_recv(requester, &lizard_id, sizeof(int), 0);
    if (lizard_id < 0)
    {
        printf("Failed to connect lizard! No more slots available\n");
        return -1;
    }
    printf("Lizard connected with id: %d\n", lizard_id);

    printf("Ending connect_lizard function\n");

    return lizard_id;
}

/**
 * @brief - Move a lizard on the screen
 *
 * @param lizard_id - Lizard id
 * @param requester - ZMQ socket
 * @param send_message - Message to send to the server
 * @return int - 0 if successful, -1 otherwise
 */
int move_lizard(int lizard_id, void *requester, message_to_server *send_message)
{
    int keypress;
    int server_reply;
    int lizard_score;
    int stop = 0;

    send_message->client_id = LIZARD;
    send_message->type = MOVEMENT;
    send_message->value = lizard_id;

    while (!stop)
    {
        // Read a character from the keyboard
        keypress = getch();

        // Check if the character is an arrow key, 'q' or 'Q'
        switch (keypress)
        {
        case KEY_UP:
            send_message->direction = UP;
            break;
        case KEY_DOWN:
            send_message->direction = DOWN;
            break;
        case KEY_LEFT:
            send_message->direction = LEFT;
            break;
        case KEY_RIGHT:
            send_message->direction = RIGHT;
            break;
        case 'q':
            stop = 1;
            break;
        case 'Q':
            stop = 1;
            break;
        default:
            continue;
        }

        // Start of proto encoder
        MessageToServerProto *message_to_server_proto = malloc(sizeof(MessageToServerProto));
        message_to_server_proto__init(message_to_server_proto);

        // Convert message to server to proto message to server
        message_to_server_to_proto_message_to_server(message_to_server_proto, send_message);

        // Get the size of the serialized message
        size_t message_to_server_proto_size = message_to_server_proto__get_packed_size(message_to_server_proto);

        // Serialize the message
        void *message_to_server_proto_buffer = malloc(message_to_server_proto_size);
        message_to_server_proto__pack(message_to_server_proto, message_to_server_proto_buffer);

        // Send the message
        zmq_send(requester, message_to_server_proto_buffer, message_to_server_proto_size, 0);

        // Free the serialized message
        free(message_to_server_proto_buffer);
        message_to_server_proto__free_unpacked(message_to_server_proto, NULL);

        // Server replies with failure if Lizard should disconnect
        zmq_recv(requester, &server_reply, sizeof(int), 0);
        if (server_reply == 404)
        {
            // End ncurses mode and print error message
            endwin();
            printf("Lizard could not be found in server!\n");
            return -1;
        }

        // If not, the server replies with the lizard's score
        lizard_score = server_reply;

        // Print the lizard's score
        // mvprintw(0, 0, "Your lizard's score is %d", lizard_score);

        // Clear the rest of the line
        // clrtoeol();

        // Refresh the screen
        // refresh();
    }

    // End ncurses mode and
    endwin();

    printf("Lizard movement stopped\n");
    printf("Your lizard's final score was %d\n", lizard_score);

    return 0;
}

void connect_to_server_and_get_display(void *requester, message_to_server *send_message, window_data **window_data_struct)
{
    printf("Starting connect_to_server_and_get_display function\n");

    send_message->client_id = DISPLAY_APP;
    send_message->type = CONNECT;

    // -- Start ProtoEncoder --
    MessageToServerProto *proto = malloc(sizeof(MessageToServerProto));
    message_to_server_proto__init(proto);

    message_to_server_to_proto_message_to_server(proto, send_message);

    size_t proto_size = message_to_server_proto__get_packed_size(proto);
    void *proto_buffer = malloc(proto_size);

    message_to_server_proto__pack(proto, proto_buffer);

    // Send the message
    zmq_send(requester, proto_buffer, proto_size, 0);

    free(proto_buffer);
    message_to_server_proto__free_unpacked(proto, NULL);

    // -- End ProtoEncoder --

    printf("Message sent to server, waiting for response\n");

    // ---------- START RECEIVE INITAL DATA FROM SERVER ----------

    printf("Starting to receive initial data from server\n");

    zmq_msg_t message;

    zmq_msg_init(&message);

    if (zmq_msg_recv(&message, requester, 0) == -1)
    {
        printf("Error receiving message: %s\n", zmq_strerror(errno));
        zmq_close(requester);
        exit(1);
    }

    printf("Message received from server\n");

    // Get the size of the received message
    size_t size = zmq_msg_size(&message);
    void *buffer = zmq_msg_data(&message);

    printf("Deserializing the received message\n");

    // Deserialize the field update from the buffer
    WindowDataProto *window_data_proto = window_data_proto__unpack(NULL, size, buffer);
    if (window_data_proto == NULL)
    {
        printf("Error unpacking message\n");
        zmq_close(requester);
        exit(1);
    }

    printf("Message deserialized successfully\n");

    // Convert the field update proto to a field update
    *window_data_struct = malloc(sizeof(window_data));
    proto_window_data_to_window_data(window_data_proto, *window_data_struct);

    printf("Window data converted successfully\n");
    window_data_proto__free_unpacked(window_data_proto, NULL);
    zmq_msg_close(&message);

    printf("Finished receiving initial data from server\n");
    // ---------- END RECEIVE INITAL DATA FROM SERVER ----------
    printf("Finished connect_to_server_and_get_display function\n");
}

/**
 * @brief - Disconnect a lizard from the server
 *
 * @param lizard_id - Lizard id
 * @param requester - ZMQ socket
 * @param send_message - Message to send to the server
 * @return int - 0 if successful, -1 otherwise
 */
int disconnect_lizard(int lizard_id, void *requester, message_to_server *send_message)
{
    int server_reply;

    send_message->client_id = LIZARD;
    send_message->type = DISCONNECT;
    send_message->value = lizard_id;

    // Start of proto encoder
    MessageToServerProto *message_to_server_proto = malloc(sizeof(MessageToServerProto));
    message_to_server_proto__init(message_to_server_proto);

    // Convert message to server to proto message to server
    message_to_server_to_proto_message_to_server(message_to_server_proto, send_message);

    // Get the size of the serialized message
    size_t message_to_server_proto_size = message_to_server_proto__get_packed_size(message_to_server_proto);

    // Serialize the message
    void *message_to_server_proto_buffer = malloc(message_to_server_proto_size);
    message_to_server_proto__pack(message_to_server_proto, message_to_server_proto_buffer);

    // Send the message
    zmq_send(requester, message_to_server_proto_buffer, message_to_server_proto_size, 0);

    // Free the serialized message
    free(message_to_server_proto_buffer);
    message_to_server_proto__free_unpacked(message_to_server_proto, NULL);

    // Server replies with failure if Lizard could not be disconnected
    zmq_recv(requester, &server_reply, sizeof(int), 0);
    if (server_reply != 0)
    {
        printf("Failed to disconnect lizard!\n");
        return -1;
    }

    printf("Lizard disconnected\n");

    return 0;
}

void *display_thread_function(void *arg)
{
    display_args *args = (display_args *)arg;
    void *subscriber = args->subscriber;
    window_data *game_window = args->game_window;
    WINDOW *score_window = args->score_window;

    zmq_msg_t message;
    box(game_window->win, 0, 0);
    while (1)
    {
        char *type = s_recv(subscriber);

        // ---------- START RECEIVE FIELD UPDATE FROM SERVER ----------
        zmq_msg_init(&message);
        if (zmq_msg_recv(&message, subscriber, 0) == -1)
        {
            printf("Error receiving message: %s\n", zmq_strerror(errno));
            zmq_close(subscriber);
            exit(1);
        }

        // Get the size of the received message
        size_t size = zmq_msg_size(&message);
        void *buffer = zmq_msg_data(&message);

        // Deserialize the field update from the buffer
        FieldUpdateProto *field_update_proto = field_update_proto__unpack(NULL, size, buffer);
        if (field_update_proto == NULL)
        {
            printf("Error unpacking message\n");
            zmq_close(subscriber);
            exit(1);
        }
        // Convert the field update proto to a field update
        field_update *field_update_struct = malloc(sizeof(field_update));
        proto_field_update_to_field_update(field_update_proto, field_update_struct);

        // Update the matrix with the received field update
        update_matrix_cells(game_window, field_update_struct->updated_cells, field_update_struct->updated_cell_indexes, field_update_struct->size_of_updated_cells);

        // Draw the scores
        // Print the scores in the score window
        int i, j;
        for (i = 0, j = 0; j < field_update_struct->size_of_scores; i++)
        {

            // j is the line number and only increments when a lizard is printed
            mvwprintw(score_window, j, 0, "Lizard %c: Score %d", field_update_struct->scores[i].ch, field_update_struct->scores[i].score);
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

        draw_updated_matrix(game_window, field_update_struct->updated_cells, field_update_struct->updated_cell_indexes, field_update_struct->size_of_updated_cells);

        //  ---------- END RECEIVE FIELD UPDATE FROM SERVER ----------
        free(type);
        zmq_msg_close(&message);
        field_update_proto__free_unpacked(field_update_proto, NULL);
        free(field_update_struct);
    }
}

/**
 * @brief - Main function
 *
 * @param argc - Number of command line arguments
 * @param argv - Command line arguments
 * @return int - 0 if successful, -1 otherwise
 */
int main(int argc, char *argv[])
{
    void *context;
    void *requester;
    void *subscriber;
    char *req_server_socket_address;
    char *sub_server_socket_address;

    printf("Usage: ./lizard-client <req_server_address> <req_server_port> <sub_server_address> <sub_server_port>\n");

    // Check if address and port were provided as command line arguments, if not, use default values
    if (argc != 5)
    {
        printf("No address and port provided!\n");
        printf("Using default server socket address: %s\n", DEFAULT_SERVER_SOCKET_ADDRESS);
        req_server_socket_address = malloc(strlen(DEFAULT_SERVER_SOCKET_ADDRESS) + 1);
        req_server_socket_address = DEFAULT_SERVER_SOCKET_ADDRESS;
        printf("Using default PUB server socket address: %s\n", DEFAULT_SUBS_SERVER_SOCKET_ADDRESS);
        sub_server_socket_address = malloc(strlen(DEFAULT_SUBS_SERVER_SOCKET_ADDRESS) + 1);
        sub_server_socket_address = DEFAULT_SUBS_SERVER_SOCKET_ADDRESS;
    }
    else
    {
        printf("Using address and port: %s %s\n", argv[1], argv[2]);
        char *address = argv[1];
        char *port = argv[2];
        req_server_socket_address = malloc(strlen("tcp://") + strlen(address) + strlen(":") + strlen(port) + 1);
        strcpy(req_server_socket_address, "tcp://");
        strcat(req_server_socket_address, address);
        strcat(req_server_socket_address, ":");
        strcat(req_server_socket_address, port);
        printf("Using SUB address and port: %s %s\n", argv[3], argv[4]);
        char *sub_address = argv[3];
        char *sub_port = argv[4];
        sub_server_socket_address = malloc(strlen("tcp://") + strlen(sub_address) + strlen(":") + strlen(sub_port) + 1);
        strcpy(sub_server_socket_address, "tcp://");
        strcat(sub_server_socket_address, sub_address);
        strcat(sub_server_socket_address, ":");
        strcat(sub_server_socket_address, sub_port);
    }

    if (create_and_connect_sockets(req_server_socket_address, sub_server_socket_address, &context, &requester, &subscriber) != 0)
        return -1;

    zmq_setsockopt(subscriber, ZMQ_SUBSCRIBE, "field_update", 3);

    // Initialize ncurses
    initscr();
    cbreak();
    keypad(stdscr, TRUE);
    noecho();

    message_to_server send_message;

    // Create lizard and connect it to the server
    printf("Creating lizard...\n");
    int lizard_id = connect_lizard(requester, &send_message);
    if (lizard_id < 0)
    {
        zmq_close(requester);
        zmq_ctx_destroy(context);
        return -1;
    }
    printf("Lizard created!\n");

    // Get the current state of the field
    printf("Waiting for field update...\n");
    window_data *game_window;
    connect_to_server_and_get_display(requester, &send_message, &game_window);
    printf("Field update received!\n");
    init_ncurses_window(&game_window, WINDOW_SIZE, WINDOW_SIZE);
    box(game_window->win, 0, 0);
    draw_entire_matrix(game_window);

    // Create a new window for the scores
    WINDOW *score_window = newwin(MAX_LIZARDS_ALLOWED, 50, 0, WINDOW_SIZE + 2);

    display_args *args = malloc(sizeof(display_args));
    args->subscriber = subscriber;
    args->game_window = game_window;
    args->score_window = score_window;

    pthread_t display_thread;
    pthread_create(&display_thread, NULL, display_thread_function, (void *)args);

    // Handle lizard movement until SIGINT is received (Ctrl+C) or the user presses the "q" or "Q" keys
    move_lizard(lizard_id, requester, &send_message);

    // Disconnect lizard from server
    disconnect_lizard(lizard_id, requester, &send_message);

    pthread_cancel(display_thread);
    endwin();

    // Close socket and destroy context
    zmq_close(requester);
    zmq_close(subscriber);
    zmq_ctx_destroy(context);

    return 0;
}
