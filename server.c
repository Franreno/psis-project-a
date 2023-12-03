#include <zmq.h>
#include <assert.h>
#include <stdlib.h>
#include "remote-char.h"

#define WINDOW_SIZE 15

// Function definitions
direction_t random_direction();
void new_position(int *x, int *y, direction_t direction);

int main()
{
    // Socket setup
    void *context = zmq_ctx_new();
    void *responder = zmq_socket(context, ZMQ_REP);
    void *publisher = zmq_socket(context, ZMQ_PUB);

    int responder_bind = zmq_bind(responder, "tcp://127.0.0.1:5555");
    int publisher_bind = zmq_bind(publisher, "tcp://127.0.0.1:5556");
    assert(responder_bind == 0);
    assert(publisher_bind == 0);

    // User management
    user *users;
    users = malloc(sizeof(user) * 10);
    int amount_of_users = 0;
    message request_message;

    // Main loop
    while (1)
    {
        // Receive and process messages
        zmq_recv(responder, &request_message, sizeof(message), 0);

        // Handle connection and movement messages
        // ...

        // Publish updates to subscribers
        // ...
    }

    // Clean up
    free(users);
    zmq_close(responder);
    zmq_close(publisher);
    zmq_ctx_destroy(context);

    return 0;
}