#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <time.h>

#include <zmq.h>

#include "remote-char.h"

/* TODO */
// DONE - The address and port of the server should be supplied to the program as a command line argument
// The period between cockroaches movement should not be fixed
// Not all cockroaches should move at the same time
// Each Roaches-client can control between 1 and 10 cockroaches
// Each roach has a score between 1 and 5 that is randomly defined by the Roaches-client for each of its roaches at startup. This value is used to draw the cockroach and is given to the lizards that eat cockroaches
// The Roaches-client does not need any special user interface, although it may print the cockroaches movements as they are generated and sent to the server
// Cockroaches are created whenever a Roaches-client successfully connects and are never destroyed. When cockroaches are created, the server places them in random positions
// When a cockroach is eaten by a Lizard it is not destroyed, it disappears for 5 seconds and reappears on a random place
// Cockroaches can move to the same place of other cockroaches, can be below or on top the body of lizards, but can not move to the head of the lizards
// Cockroaches cannot occupy more than 1/3 of the field. Students should decide what happens if a Roaches-client tries to connect and there already are enough cockroaches in the board
// A roaches-client is inserted into a list or array when the server receives a Roaches_connect messages

// Roaches_connect + response (from the Roaches-client to the server)
// Roaches_movement + response (from the Roaches-client to the server)

int main(int argc, char *argv[]) {
    char *address = DEFAULT_SERVER_ADDRESS;
    char *port = DEFAULT_SERVER_PORT;

    // Check if address and port were provided as command line arguments, if not, use default values
    if(argc != 3) {
        printf("No address and port provided!\n");
        printf("Using default address and port: %s %s\n", DEFAULT_SERVER_ADDRESS, DEFAULT_SERVER_PORT);
    } else {
        printf("Using address and port: %s %s\n", argv[1], argv[2]);
        address = argv[1];
        port = argv[2];
    }

    // Create socket address
    char *server_socket_address = malloc(strlen("tcp://") + strlen(address) + strlen(":") + strlen(port) + 1);
    strcpy(server_socket_address, "tcp://");
    strcat(server_socket_address, address);
    strcat(server_socket_address, ":");
    strcat(server_socket_address, port);
    server_socket_address = "ipc:///tmp/server"; // WORKAROUND
    printf("Connecting to server at %s\n", server_socket_address);

    // Create context
    void *context = zmq_ctx_new();
    if(context == NULL) {
        printf("Failed to create context: %s\n", zmq_strerror(errno));
        return 1;
    }

    // Create REQ socket to send messages to server
    void *requester = zmq_socket(context, ZMQ_REQ);
    if(requester == NULL) {
        printf("Failed to create socket: %s\n", zmq_strerror(errno));
        zmq_ctx_destroy(context);
        return 1;
    }

    // Connect to the server using ZMQ_REQ
    if(zmq_connect(requester, server_socket_address) != 0) {
        printf("Failed to connect: %s\n", zmq_strerror(errno));
        zmq_close(requester);
        zmq_ctx_destroy(context);
        return 1;
    }

    message_to_server send_message;
    send_message.client_id = 2;
    send_message.type = 1;

    int num_roaches;
    int reply;

    // Seed random number generator
    srand(time(NULL));

    // Randomly select the number of roaches to attempt to generate
    num_roaches = rand() % 10 + 1;
    printf("Number of cockroaches to attempt to generate: %d\n", num_roaches);

    // Create an array of roaches
    int *roaches = malloc(sizeof(int) * num_roaches);

    // For each roach, randomly generate a value from 1 to 5 for its score
    for(int i = 0; i < num_roaches; i++) {
        roaches[i] = rand() % 5 + 1;
    }

    printf("Connecting cockroaches...\n");

    // For each roach, send a connect message to the server and wait for a response
    for(int i = 0; i < num_roaches; i++) {
        send_message.value = roaches[i];
        // Send message to server connecting a roach
        zmq_send(requester, &send_message, sizeof(message_to_server), 0);
        printf("Trying to connect roach with score %d\n", roaches[i]);

        // Server replies with either failure or the roach id that was assigned to the roach
        zmq_recv(requester, &reply, sizeof(int), 0);

        // If the server replies with failure, stop attempting to connect roaches
        if(reply == FAILURE) {
            printf("Failed to connect roach! No more slots available\n");
            num_roaches = i;
            break;
        // If the server replies with a roach id, store the id in the roaches array, replacing it's score
        } else {
            printf("Successfully connected roach! Roach id: %d\n", reply);
            roaches[i] = reply;
        }
    }

    int sleep_delay;
    send_message.type = 2;

    while(1) {
        // Iterate over each roach and send a movement message to the server
        for(int i = 0; i < num_roaches; i++) {
            // Sleep for a random amount of time
            sleep_delay = random() % 2000000;
            usleep(sleep_delay);

            // Prepare roach movement message selecting a random direction
            send_message.value = roaches[i];
            send_message.direction = rand() % 4;

            // Send roach movement message to server
            zmq_send(requester, &send_message, sizeof(message_to_server), 0);

            // Server replies with success if the movement was successful
            zmq_recv(requester, &reply, sizeof(int), 0);
        }
    }

    // Close socket and destroy context
    zmq_close(requester);
    zmq_ctx_destroy(context);

    return 0;
}
