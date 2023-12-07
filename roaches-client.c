#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <zmq.h>
#include "remote-char.h"

volatile sig_atomic_t stop = 0;

void handle_sigint(int sig)
{
    stop = 1;
}

int create_and_connect_socket(int argc, char *argv[], char **server_socket_address, void **context, void **requester)
{
    char *address = DEFAULT_SERVER_ADDRESS;
    char *port = DEFAULT_SERVER_PORT;

    // Check if address and port were provided as command line arguments, if not, use default values
    if (argc != 3)
    {
        printf("No address and port provided!\n");
        printf("Using default address and port: %s %s\n", DEFAULT_SERVER_ADDRESS, DEFAULT_SERVER_PORT);
    }
    else
    {
        printf("Using address and port: %s %s\n", argv[1], argv[2]);
        address = argv[1];
        port = argv[2];
    }

    // Create socket address
    *server_socket_address = malloc(strlen("tcp://") + strlen(address) + strlen(":") + strlen(port) + 1);
    strcpy(*server_socket_address, "tcp://");
    strcat(*server_socket_address, address);
    strcat(*server_socket_address, ":");
    strcat(*server_socket_address, port);

    // TODO - FIX THIS WORKAROUND
    *server_socket_address = "ipc:///tmp/server"; // WORKAROUND, for some reason, the server doesn't work with tcp

    printf("Connecting to server at %s\n", *server_socket_address);

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
    if (zmq_connect(*requester, *server_socket_address) != 0)
    {
        printf("Failed to connect: %s\n", zmq_strerror(errno));
        zmq_close(*requester);
        zmq_ctx_destroy(*context);
        return -1;
    }

    return 0;
}

void generate_and_connect_roaches(int num_roaches, int *roaches, void *requester, message_to_server *send_message)
{
    int reply;

    send_message->client_id = ROACH;
    send_message->type = CONNECT;

    // For each roach, randomly generate a value for its score
    for (int i = 0; i < num_roaches; i++)
    {
        roaches[i] = rand() % MAX_ROACH_SCORE + 1;
    }

    // For each roach, send a connect message to the server and wait for a response
    for (int i = 0; i < num_roaches; i++)
    {
        send_message->value = roaches[i];
        // Send message to server connecting a roach
        zmq_send(requester, send_message, sizeof(message_to_server), 0);
        printf("Attempting to connect roach with score: %d\n", roaches[i]);

        // Server replies with either failure or the roach id that was assigned to the roach
        zmq_recv(requester, &reply, sizeof(int), 0);

        // If the server replies with failure, stop attempting to connect roaches
        if (reply == FAILURE)
        {
            printf("Failed to connect roach! No more slots available\n");
            num_roaches = i;
            break;
        }
        // If the server replies with a roach id, store the id in the roaches array, replacing it's score
        else
        {
            printf("Roach connected with id: %d\n", reply);
            roaches[i] = reply;
        }
    }
}

void move_roaches(int num_roaches, int *roaches, void *requester, message_to_server *send_message, volatile sig_atomic_t *stop)
{
    int sleep_delay;
    int reply;

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
                sleep_delay = random() % 200000;
                usleep(sleep_delay);

                // Prepare roach movement message selecting a random direction
                send_message->value = roaches[i];
                send_message->direction = rand() % 4;

                // Send roach movement message to server
                zmq_send(requester, send_message, sizeof(message_to_server), 0);

                // Server replies with success if the movement was successful
                zmq_recv(requester, &reply, sizeof(int), 0);
            }
        }
    }
}

void disconnect_roaches(int num_roaches, int *roaches, void *requester, message_to_server *send_message)
{
    int reply;
    send_message->client_id = ROACH;
    send_message->type = DISCONNECT;

    for (int i = 0; i < num_roaches; i++)
    {
        send_message->value = roaches[i];
        zmq_send(requester, send_message, sizeof(message_to_server), 0);
        zmq_recv(requester, &reply, sizeof(int), 0);
    }
}

int main(int argc, char *argv[])
{
    signal(SIGINT, handle_sigint);

    char *server_socket_address;
    void *context;
    void *requester;

    if (create_and_connect_socket(argc, argv, &server_socket_address, &context, &requester) != 0)
    {
        return 1;
    }

    message_to_server send_message;

    // Seed random number generator
    srand(time(NULL));

    // Randomly select the number of roaches to attempt to generate
    int num_roaches = rand() % MAX_ROACHES_GENERATED + 1;
    printf("Number of cockroaches to attempt to generate: %d\n", num_roaches);

    // Create an array of roaches and connect them to the server
    int *roaches = malloc(sizeof(int) * num_roaches);
    generate_and_connect_roaches(num_roaches, roaches, requester, &send_message);

    // Handle roaches movement until SIGINT is received (Ctrl+C)
    move_roaches(num_roaches, roaches, requester, &send_message, &stop);

    // Disconnect roaches from server
    disconnect_roaches(num_roaches, roaches, requester, &send_message);

    // Close socket and destroy context
    zmq_close(requester);
    zmq_ctx_destroy(context);

    return 0;
}
