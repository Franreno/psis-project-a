#include "default_consts.h"
#include "proto-encoder.h"
#include "server_messages.pb-c.h"

volatile sig_atomic_t stop = 0;

/**
 * @brief - Handle SIGINT signal
 */
void handle_sigint()
{
    stop = 1;
}

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

    // Add security keys to the socket
    if (zmq_setsockopt(*requester, ZMQ_CURVE_SERVERKEY, SERVER_PUBLIC_KEY, 40) != 0)
    {
        printf("Failed to set server public key: %s\n", zmq_strerror(errno));
        zmq_close(*requester);
        zmq_ctx_destroy(*context);
        return -1;
    }

    if (zmq_setsockopt(*requester, ZMQ_CURVE_PUBLICKEY, CLIENT_PUBLIC_KEY, 40) != 0)
    {
        printf("Failed to set client public key: %s\n", zmq_strerror(errno));
        zmq_close(*requester);
        zmq_ctx_destroy(*context);
        return -1;
    }

    if (zmq_setsockopt(*requester, ZMQ_CURVE_SECRETKEY, CLIENT_SECRET_KEY, 40) != 0)
    {
        printf("Failed to set client secret key: %s\n", zmq_strerror(errno));
        zmq_close(*requester);
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
 * @brief - Generate and Connect a wasp to the server
 *
 * @param num_wasps - Number of wasps to generate
 * @param wasps - Array of wasps
 * @param requester - ZMQ socket
 * @param send_message - Message to send to the server
 * @return int - 0 if successful, -1 otherwise
 */
int generate_and_connect_wasps(int num_wasps, int *wasps, void *requester, message_to_server *send_message)
{
    int server_reply;
    int wasp_id;

    send_message->client_id = WASP;
    send_message->type = CONNECT;

    // For each wasp, send a connect message to the server and wait for a response
    for (int i = 0; i < num_wasps; i++)
    {
        // Send message to server to connect a wasp
        printf("Attempting to connect wasp\n");

        // Start of proto encoder
        MessageToServerProto *message_to_server_proto = malloc(sizeof(MessageToServerProto));
        message_to_server_proto__init(message_to_server_proto);

        // Convert message_to_server to proto message_to_server
        message_to_server_to_proto_message_to_server(message_to_server_proto, send_message);

        // Get size of the serialized message
        size_t proto_size = message_to_server_proto__get_packed_size(message_to_server_proto);

        // Serialize the message
        void *serialized_proto = malloc(proto_size);
        message_to_server_proto__pack(message_to_server_proto, serialized_proto);

        // Send the message
        zmq_send(requester, serialized_proto, proto_size, 0);

        // Free the serialized message
        free(serialized_proto);
        message_to_server_proto__free_unpacked(message_to_server_proto, NULL);

        // Server replies with either failure or the assigned wasp id
        zmq_recv(requester, &server_reply, sizeof(int), 0);
        if (server_reply < 0)
        {
            printf("Failed to connect wasp! No more slots available\n");
            num_wasps = i;
            return -1;
        }

        // If the server replies with a wasp id, store the id in the wasps array
        wasp_id = server_reply;
        wasps[i] = wasp_id;
        printf("Wasp connected with id: %d\n", wasp_id);
    }

    return 0;
}

/**
 * @brief - Move wasps on the screen
 *
 * @param num_wasps - Number of wasps
 * @param wasps - Array of wasps
 * @param requester - ZMQ socket
 * @param send_message - Message to send to the server
 * @param stop - Flag to stop wasps movement
 * @return int - 0 if successful, -1 otherwise
 */
int move_wasps(int num_wasps, int *wasps, void *requester, message_to_server *send_message, volatile sig_atomic_t *stop)
{
    int sleep_delay;
    int server_reply;

    send_message->client_id = WASP;
    send_message->type = MOVEMENT;

    while (!*stop)
    {
        // Iterate over each wasp and send a movement message to the server
        for (int i = 0; i < num_wasps && !*stop; i++)
        {
            // Decide randomly whether this wasp should move in this cycle
            if (rand() % 100 < WASP_MOVE_CHANCE)
            {
                // Sleep for a random amount of time
                sleep_delay = random() % WASP_MOVE_DELAY;
                usleep(sleep_delay);

                // Prepare wasp movement message selecting a random direction
                send_message->value = wasps[i];
                send_message->direction = rand() % 4;

                // Start of proto encoder
                MessageToServerProto *message_to_server_proto = malloc(sizeof(MessageToServerProto));
                message_to_server_proto__init(message_to_server_proto);

                // Convert message to server to proto message to server
                message_to_server_to_proto_message_to_server(message_to_server_proto, send_message);

                // Get the size of the serialized message
                size_t proto_size = message_to_server_proto__get_packed_size(message_to_server_proto);

                // Serialize the message
                void *serialized_proto = malloc(proto_size);
                message_to_server_proto__pack(message_to_server_proto, serialized_proto);

                // Send the message
                zmq_send(requester, serialized_proto, proto_size, 0);

                // Free the serialized message
                free(serialized_proto);
                message_to_server_proto__free_unpacked(message_to_server_proto, NULL);

                // Server replies with failure if wasp should disconnect
                zmq_recv(requester, &server_reply, sizeof(int), 0);
                if (server_reply == 404)
                {
                    printf("Wasp could not be found in server!\n");
                    return -1;
                }
            }
        }
    }

    printf("Wasps movement stopped\n");

    return 0;
}

/**
 * @brief - Disconnect wasps from the server
 *
 * @param num_wasps  - Number of wasps
 * @param wasps - Array of wasps
 * @param requester -   ZMQ socket
 * @param send_message - Message to send to the server
 * @return int - 0 if successful, -1 otherwise
 */
int disconnect_wasps(int num_wasps, int *wasps, void *requester, message_to_server *send_message)
{
    int server_reply;
    int success = 0;
    send_message->client_id = WASP;
    send_message->type = DISCONNECT;

    // Iterate over each wasp and send a disconnect message to the server
    for (int i = 0; i < num_wasps; i++)
    {
        send_message->value = wasps[i];

        // Start of proto encoder
        MessageToServerProto *message_to_server_proto = malloc(sizeof(MessageToServerProto));
        message_to_server_proto__init(message_to_server_proto);

        // Convert message_to_server to proto message_to_server
        message_to_server_to_proto_message_to_server(message_to_server_proto, send_message);

        // Get size of the serialized message
        size_t proto_size = message_to_server_proto__get_packed_size(message_to_server_proto);

        // Serialize the message
        void *serialized_proto = malloc(proto_size);
        message_to_server_proto__pack(message_to_server_proto, serialized_proto);

        // Send the message
        zmq_send(requester, serialized_proto, proto_size, 0);

        // Free the serialized message
        free(serialized_proto);
        message_to_server_proto__free_unpacked(message_to_server_proto, NULL);

        // Server replies with failure if wasp could not be disconnected
        zmq_recv(requester, &server_reply, sizeof(int), 0);
        if (server_reply != 0)
        {
            printf("Failed to disconnect wasp!\n");
            success = -1;
        }
    }

    num_wasps = 0;

    printf("Wasps disconnected\n");

    return success;
}

int main(int argc, char *argv[])
{
    signal(SIGINT, handle_sigint);

    int num_wasps;
    void *context;
    void *requester;
    char *server_socket_address;
    int *wasps;

    // Seed random number generator
    srand(time(NULL));

    printf("Usage: ./wasps-client <num_wasps> <server_address> <server_port>\n");
    printf("Number of wasps must be between 1 and %d\n", MAX_WASPS_GENERATED);

    // Check if number of wasps was provided as command line argument, if not, use random number
    if (argc < 2 || atoi(argv[1]) < 1 || atoi(argv[1]) > MAX_WASPS_GENERATED)
    {
        printf("No number of wasps provided or value is invalid!\n");
        num_wasps = rand() % MAX_WASPS_GENERATED + 1;
        printf("Using random number of wasps: %d\n", num_wasps);
    }
    else
    {
        num_wasps = atoi(argv[1]);
        printf("Using provided number of wasps: %d\n", num_wasps);
    }

    // Check if address and port were provided as command line arguments, if not, use default values
    if (argc != 4)
    {
        printf("No address and port provided!\n");
        printf("Using default server socket address: %s\n", DEFAULT_SERVER_SOCKET_ADDRESS);
        server_socket_address = malloc(strlen(DEFAULT_SERVER_SOCKET_ADDRESS) + 1);
        server_socket_address = DEFAULT_SERVER_SOCKET_ADDRESS;
    }
    else
    {
        printf("Using address and port: %s %s\n", argv[2], argv[3]);
        char *address = argv[2];
        char *port = argv[3];
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

    // Create an array of wasps and connect them to the server
    wasps = malloc(sizeof(int) * num_wasps);
    generate_and_connect_wasps(num_wasps, wasps, requester, &send_message);

    // Handle wasps movement until SIGINT is received (Ctrl+C)
    move_wasps(num_wasps, wasps, requester, &send_message, &stop);

    // Disconnect wasps from server
    disconnect_wasps(num_wasps, wasps, requester, &send_message);

    // Close socket and destroy context and free memory
    zmq_close(requester);
    zmq_ctx_destroy(context);

    return 0;
}
