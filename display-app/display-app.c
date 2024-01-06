#include "default_consts.h"
#include "logger.h"
#include "window.h"
#include "roach-mover.h"
#include "lizard-mover.h"
#include "proto-encoder.h"

/**
 * @brief Receives a message from a socket
 *
 * @param socket  Socket to receive the message from
 * @return char* The received message
 */
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
    log_init("display-app.log");
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

    zmq_setsockopt(subscriber, ZMQ_SUBSCRIBE, "field_update", 3);

    // Initialize ncurses
    initscr();
    cbreak();
    keypad(stdscr, TRUE);
    noecho();

    // Connect to the server and get the matrix information from it
    message_to_server send_message;
    send_message.client_id = DISPLAY_APP;
    send_message.type = CONNECT;
    // -- Start ProtoEncoder --
    MessageToServerProto *proto = malloc(sizeof(MessageToServerProto));
    message_to_server_proto__init(proto);

    message_to_server_to_proto_message_to_server(proto, &send_message);

    size_t proto_size = message_to_server_proto__get_packed_size(proto);
    void *proto_buffer = malloc(proto_size);

    message_to_server_proto__pack(proto, proto_buffer);

    // Send the message
    zmq_send(requester, proto_buffer, proto_size, 0);

    free(proto_buffer);
    message_to_server_proto__free_unpacked(proto, NULL);

    // -- End ProtoEncoder --

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
    roach **eaten_roaches = (roach **)malloc(sizeof(roach *) * MAX_SLOTS_ALLOWED);
    int eaten_roaches_count = 0;

    roach_payload->eaten_roaches = eaten_roaches;
    roach_payload->amount_eaten_roaches = &eaten_roaches_count;
    lizard_payload->eaten_roaches = eaten_roaches;
    lizard_payload->amount_eaten_roaches = &eaten_roaches_count;
    roach_payload->lizards = lizard_payload->lizards;
    lizard_payload->roaches = roach_payload->roaches;

    zmq_msg_t message;
    while (1)
    {
        // Receive the field update
        char *type = s_recv(subscriber);

        // ---------- START RECEIVE FIELD UPDATE FROM SERVER ----------
        zmq_msg_init(&message);
        if (zmq_msg_recv(&message, subscriber, 0) == -1)
        {
            printf("Error receiving message: %s\n", zmq_strerror(errno));
            zmq_close(requester);
            zmq_close(subscriber);
            zmq_ctx_destroy(context);
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
            zmq_close(requester);
            zmq_close(subscriber);
            zmq_ctx_destroy(context);
            exit(1);
        }
        // Convert the field update proto to a field update
        field_update *field_update_struct = malloc(sizeof(field_update));
        proto_field_update_to_field_update(field_update_proto, field_update_struct);

        // Update the matrix with the received field update
        update_matrix_cells(game_window, field_update_struct->updated_cells, field_update_struct->updated_cell_indexes, field_update_struct->size_of_updated_cells);

        draw_updated_matrix(game_window, field_update_struct->updated_cells, field_update_struct->updated_cell_indexes, field_update_struct->size_of_updated_cells);
        //  ---------- END RECEIVE FIELD UPDATE FROM SERVER ----------
        free(type);
        zmq_msg_close(&message);
        field_update_proto__free_unpacked(field_update_proto, NULL);
        free(field_update_struct);
    }

    endwin();
    zmq_close(requester);
    zmq_close(subscriber);
    zmq_ctx_destroy(context);
    log_close();

    return 0;
}