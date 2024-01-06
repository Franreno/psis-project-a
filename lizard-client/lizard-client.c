#include "default_consts.h"
#include "proto-encoder.h"

/**
 * @brief Create a and connect socket object
 *
 * @param server_socket_address  - Address of the server socket
 * @param context  - ZMQ context
 * @param requester - ZMQ socket
 * @return int - 0 if successful, -1 otherwise
 */
int create_and_connect_socket(char *server_socket_address, void **context, void **requester)
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

    // Connect to the server using ZMQ_REQ
    if (zmq_connect(*requester, server_socket_address) != 0)
    {
        printf("Failed to connect: %s\n", zmq_strerror(errno));
        zmq_close(*requester);
        zmq_ctx_destroy(*context);
        return -1;
    }

    return 0;
}

/**
 * @brief - Connect a lizard to the server
 *
 * @param requester  - ZMQ socket
 * @param send_message - Message to send to the server
 * @return int - Lizard id if successful, -1 otherwise
 */
int connect_lizard(void *requester, message_to_server *send_message)
{
    int lizard_id;

    send_message->client_id = LIZARD;
    send_message->type = CONNECT;
    send_message->value = CONNECT;

    // Send a message to the server to connect a lizard
    printf("Attempting to connect lizard");

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

    // Server replies with either failure or the assigned lizard id
    zmq_recv(requester, &lizard_id, sizeof(int), 0);
    if (lizard_id < 0)
    {
        printf("Failed to connect lizard! No more slots available\n");
        return -1;
    }
    printf("Lizard connected with id: %d\n", lizard_id);

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

    // Initialize ncurses mode
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

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
        mvprintw(0, 0, "Your lizard's score is %d", lizard_score);

        // Clear the rest of the line
        clrtoeol();

        // Refresh the screen
        refresh();
    }

    // End ncurses mode and
    endwin();

    printf("Lizard movement stopped\n");
    printf("Your lizard's final score was %d\n", lizard_score);

    return 0;
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
    char *server_socket_address;

    printf("Usage: ./lizard-client <server_address> <server_port>\n");

    // Check if address and port were provided as command line arguments, if not, use default values
    if (argc != 3)
    {
        printf("No address and port provided!\n");
        printf("Using default server socket address: %s\n", DEFAULT_SERVER_SOCKET_ADDRESS);
        server_socket_address = malloc(strlen(DEFAULT_SERVER_SOCKET_ADDRESS) + 1);
        server_socket_address = DEFAULT_SERVER_SOCKET_ADDRESS;
    }
    else
    {
        printf("Using address and port: %s %s\n", argv[1], argv[2]);
        char *address = argv[1];
        char *port = argv[2];
        server_socket_address = malloc(strlen("tcp://") + strlen(address) + strlen(":") + strlen(port) + 1);
        strcpy(server_socket_address, "tcp://");
        strcat(server_socket_address, address);
        strcat(server_socket_address, ":");
        strcat(server_socket_address, port);
    }

    printf("Connecting to server at %s\n", server_socket_address);

    if (create_and_connect_socket(server_socket_address, &context, &requester) != 0)
        return -1;

    message_to_server send_message;

    // Create lizard and connect it to the server
    int lizard_id = connect_lizard(requester, &send_message);
    if (lizard_id < 0)
    {
        zmq_close(requester);
        zmq_ctx_destroy(context);
        return -1;
    }

    // Handle lizard movement until SIGINT is received (Ctrl+C) or the user presses the "q" or "Q" keys
    move_lizard(lizard_id, requester, &send_message);

    // Disconnect lizard from server
    disconnect_lizard(lizard_id, requester, &send_message);

    // Close socket and destroy context
    zmq_close(requester);
    zmq_ctx_destroy(context);

    return 0;
}
