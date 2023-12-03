#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>

#include <zmq.h>

#include "remote-char.h"

int main(int argc, char* argv []) {
    /* Create context */
    void* context = zmq_ctx_new();
    if(context == NULL) {
        printf("Failed to create context: %s\n", zmq_strerror(errno));
        return 1;
    }

    /* Create REQ socket to send messages to server */
    void* requester = zmq_socket(context, ZMQ_REQ);
    if(requester == NULL) {
        printf("Failed to create socket: %s\n", zmq_strerror(errno));
        zmq_ctx_destroy(context);
        return 1;
    }

    /* Connect to the server using ZMQ_REQ */
    if(zmq_connect (requester, SERVER_SOCKET_IP) != 0) {
        printf("Failed to connect: %s\n", zmq_strerror(errno));
        zmq_close(requester);
        zmq_ctx_destroy(context);
        return 1;
    }

    char character = '1';

    message m;
    m.msg_type = 0;
    m.ch = character;
    m.direction = 0;

    // write(fd, &m, sizeof(message));
    zmq_send(requester, &m, sizeof(message), 0);

    int sleep_delay;
    direction_t direction;
    int n = 0;
    m.msg_type = 1;
    int success;

    while(1) {
        sleep_delay = random() % 700000;
        usleep(sleep_delay);
        direction = random() % 4;
        n++;

        m.direction = direction;

        // write(fd, &m, sizeof(message));
        zmq_send(requester, &m, sizeof(message), 0);

        zmq_recv(requester, &success, sizeof(success), 0);
        if(success == 1) {
            printf("\nMessage sent successfully!\n");
        } else {
            printf("\nFailed to send message!\n");
        }
    }

    /* Close socket and destroy context */
    zmq_close(requester);
    zmq_ctx_destroy(context);

    return 0;
}
