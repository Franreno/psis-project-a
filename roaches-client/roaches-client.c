#include <protobuf-c/protobuf-c.h>
#include "default_consts.h"
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
 * @brief - Generate and Connect a roach to the server
 *
 * @param num_roaches - Number of roaches to generate
 * @param roaches - Array of roaches
 * @param requester - ZMQ socket
 * @param send_message - Message to send to the server
 * @return int - 0 if successful, -1 otherwise
 */

int generate_and_connect_roaches(int num_roaches, int *roaches, void *requester)
{
    MessageToServer send_message = MESSAGE_TO_SERVER__INIT;
    void *buf;    // Buffer to hold serialized data
    unsigned len; // Length of serialized data

    send_message.client_id = CLIENT_TYPE__ROACH;
    send_message.type = MESSAGE_TYPE__CONNECT;

    for (int i = 0; i < num_roaches; i++)
    {
        roaches[i] = rand() % MAX_ROACH_SCORE + 1;
        send_message.value = roaches[i];

        // Serialize the message
        len = message_to_server__get_packed_size(&send_message);
        buf = malloc(len);
        message_to_server__pack(&send_message, buf);

        // Send the serialized message
        zmq_send(requester, buf, len, 0);
        printf("Attempting to connect roach with score: %d\n", roaches[i]);
        free(buf); // Free the buffer after sending

        // Receive the server's reply
        int server_reply;
        zmq_recv(requester, &server_reply, sizeof(int), 0);
        if (server_reply == -1)
        {
            printf("Failed to connect roach! No more slots available\n");
            num_roaches = i;
            return -1;
        }

        int roach_id = server_reply;
        roaches[i] = roach_id;
        printf("Roach connected with id: %d\n", roach_id);
    }

    return 0;
}

/**
 * @brief - Move roaches on the screen
 *
 * @param num_roaches - Number of roaches
 * @param roaches - Array of roaches
 * @param requester - ZMQ socket
 * @param send_message - Message to send to the server
 * @param stop - Flag to stop roaches movement
 * @return int - 0 if successful, -1 otherwise
 */
int move_roaches(int num_roaches, int *roaches, void *requester, message_to_server *send_message, volatile sig_atomic_t *stop)
{
    int sleep_delay;
    int server_reply;

    send_message->client_id = ROACH;
    send_message->type = MOVEMENT;

    while (!*stop)
    {
        // Iterate over each roach and send a movement message to the server
        for (int i = 0; i < num_roaches && !*stop; i++)
        {
            // Decide randomly whether this roach should move in this cycle
            if (rand() % 100 < ROACH_MOVE_CHANCE)
            {
                // Sleep for a random amount of time
                sleep_delay = random() % ROACH_MOVE_DELAY;
                usleep(sleep_delay);

                // Prepare roach movement message selecting a random direction
                send_message->value = roaches[i];
                send_message->direction = rand() % 4;

                // Send roach movement message to server
                zmq_send(requester, send_message, sizeof(message_to_server), 0);

                // Server replies with failure if roaches should disconnect
                zmq_recv(requester, &server_reply, sizeof(int), 0);
                if (server_reply != 0)
                {
                    printf("Server ordered roaches should stop and disconnect!\n");
                    return -1;
                }
            }
        }
    }

    printf("Roaches movement stopped\n");

    return 0;
}

/**
 * @brief - Disconnect roaches from the server
 *
 * @param num_roaches  - Number of roaches
 * @param roaches - Array of roaches
 * @param requester -   ZMQ socket
 * @param send_message - Message to send to the server
 * @return int - 0 if successful, -1 otherwise
 */
int disconnect_roaches(int num_roaches, int *roaches, void *requester, message_to_server *send_message)
{
    int server_reply;
    int success = 0;
    send_message->client_id = ROACH;
    send_message->type = DISCONNECT;

    // Iterate over each roach and send a disconnect message to the server
    for (int i = 0; i < num_roaches; i++)
    {
        send_message->value = roaches[i];
        zmq_send(requester, send_message, sizeof(message_to_server), 0);
        zmq_recv(requester, &server_reply, sizeof(int), 0);
        if (server_reply != 0)
        {
            printf("Failed to disconnect roach!\n");
            success = -1;
        }
    }

    num_roaches = 0;

    printf("Roaches disconnected\n");

    return success;
}

int main(int argc, char *argv[])
{
    signal(SIGINT, handle_sigint);

    int num_roaches;
    void *context;
    void *requester;
    char *server_socket_address;
    int *roaches;

    // Seed random number generator
    srand(time(NULL));

    printf("Usage: ./roaches-client <num_roaches> <server_address> <server_port>\n");
    printf("Number of roaches must be between 1 and %d\n", MAX_ROACHES_GENERATED);

    // Check if number of roaches was provided as command line argument, if not, use random number
    if (argc < 2 || atoi(argv[1]) < 1 || atoi(argv[1]) > MAX_ROACHES_GENERATED)
    {
        printf("No number of roaches provided or value is invalid!\n");
        num_roaches = rand() % MAX_ROACHES_GENERATED + 1;
        printf("Using random number of roaches: %d\n", num_roaches);
    }
    else
    {
        num_roaches = atoi(argv[1]);
        printf("Using provided number of roaches: %d\n", num_roaches);
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

    // Create an array of roaches and connect them to the server
    roaches = malloc(sizeof(int) * num_roaches);
    generate_and_connect_roaches(num_roaches, roaches, requester);

    // Handle roaches movement until SIGINT is received (Ctrl+C)
    move_roaches(num_roaches, roaches, requester, &send_message, &stop);

    // Disconnect roaches from server
    disconnect_roaches(num_roaches, roaches, requester, &send_message);

    // Close socket and destroy context and free memory
    zmq_close(requester);
    zmq_ctx_destroy(context);

    return 0;
}
