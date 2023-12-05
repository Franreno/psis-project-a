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

int main()
{
    // Initialize logger
    log_init("Display-app.log");

    // Create socket address
    char *address = DEFAULT_SERVER_ADDRESS;
    char *port = DEFAULT_SERVER_PORT;
    char *server_socket_address = malloc(sizeof(char) * (strlen("tcp://") + strlen(address) + strlen(":") + strlen(port) + 1));
    strcpy(server_socket_address, "tcp://");
    strcat(server_socket_address, address);
    strcat(server_socket_address, ":");
    strcat(server_socket_address, port);
    server_socket_address = "ipc:///tmp/server"; // WORKAROUND
    printf("Connecting to server at %s\n", server_socket_address);

    // Create context
    void *context = zmq_ctx_new();
    if (context == NULL)
    {
        printf("Failed to create context: %s\n", zmq_strerror(errno));
        return 1;
    }

    // Create REP socket to receive messages from clients
    void *requester = zmq_socket(context, ZMQ_REQ);
    if (requester == NULL)
    {
        printf("Failed to create socket: %s\n", zmq_strerror(errno));
        zmq_ctx_destroy(context);
        return 1;
    }
    // Create a subscriber socket to send messages to the display app
    void *subscriber = zmq_socket(context, ZMQ_PUB);
    if (subscriber == NULL)
    {
        printf("Failed to create PUB socket: %s\n", zmq_strerror(errno));
        zmq_close(requester);
        zmq_ctx_destroy(context);
        return 1;
    }
    // Connect to the server using ZMQ_REQ
    if (zmq_connect(requester, server_socket_address) != 0)
    {
        printf("Failed to connect: %s\n", zmq_strerror(errno));
        zmq_close(requester);
        zmq_ctx_destroy(context);
        return 1;
    }
    // Conect to the server usin ZMQ_PUB
    if (zmq_connect(subscriber, "tcp://127.0.0.1:5556") != 0)
    {
        printf("Failed to connect: %s\n", zmq_strerror(errno));
        zmq_close(subscriber);
        zmq_ctx_destroy(context);
        return 1;
    }

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

    // Buffer to hold the received data
    char *buffer = NULL;
    size_t buffer_size = 0;

    // First, receive the size of the incoming message
    log_write("Waiting for buffer size");
    zmq_recv(requester, &buffer_size, sizeof(buffer_size), 0);

    log_write("Received buffer size: %d", buffer_size);
    // Allocate memory for the buffer based on the received size
    buffer = malloc(buffer_size);
    if (!buffer)
    {
        log_write("Failed to allocate memory for buffer");
        // Handle memory allocation error
        exit(1);
    }

    // Creates a window and draws a border
    window_data *game_window;
    log_write("Initializing window");
    window_init_with_matrix(&game_window, WINDOW_SIZE, WINDOW_SIZE, buffer);
    log_write("Window initialized");
    draw_entire_matrix(game_window);
    log_write("Window drawn");

    int sleep_delay;
    while (1)
    {
        // Sleep for a random amount of time
        sleep_delay = random() % 2000000;
        usleep(sleep_delay);
    }

    endwin();
    zmq_close(requester);
    zmq_close(subscriber);
    zmq_ctx_destroy(context);
    log_close();

    return 0;
}