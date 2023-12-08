#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <zmq.h>
#include <ncurses.h>
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

int connect_lizard(void *requester, message_to_server *send_message)
{
    char lizard_char;
    int reply;

    send_message->client_id = LIZARD;
    send_message->type = CONNECT;

    // Ask the user to enter a character to identify the lizard
    printf("Please enter a character to identify the lizard: ");
    lizard_char = getchar();

    // Send a connect message to the server and wait for a response
    send_message->value = (int)lizard_char;
    zmq_send(requester, send_message, sizeof(message_to_server), 0);
    printf("Attempting to connect lizard with char: %c\n", lizard_char);

    // Server replies with either failure or the assigned lizard id
    zmq_recv(requester, &reply, sizeof(int), 0);

    // If the server replies with -1, no more slots for lizards are available
    if (reply == -1)
    {
        printf("Failed to connect lizard! No more slots available\n");
    }

    // If the server replies with -2, there's already a lizard with that character
    else if (reply == -2)
    {
        printf("Failed to connect lizard! There's already a lizard with that character\n");
    }

    // If the server replies with a lizard id, return the id
    else
    {
        printf("Lizard connected with id: %d\n", reply);
    }

    return reply;
}

void move_lizard(int lizard_id, void *requester, message_to_server *send_message, volatile sig_atomic_t *stop)
{
    int ch;
    int reply;

    // Initialize ncurses mode
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    send_message->client_id = LIZARD;
    send_message->type = MOVEMENT;
    send_message->value = lizard_id;

    while (!*stop)
    {
        // Read a character from the keyboard
        ch = getch();

        // Check if the character is an arrow key, 'q' or 'Q'
        switch (ch)
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
        case 'Q':
            *stop = 1;
            break;
        default:
            continue;
        }

        // Send lizard movement message to server
        zmq_send(requester, send_message, sizeof(message_to_server), 0);

        // Server replies with lizard score after the movement
        zmq_recv(requester, &reply, sizeof(int), 0);

        // Move the cursor to the beginning of the line
        move(0, 0);

        // Print the lizards score
        printw("Your Lizard's score is %d!", reply);

        // Clear the rest of the line
        clrtoeol();

        // Refresh the screen to show the new content
        refresh();
    }

    // End ncurses mode
    endwin();
}

void disconnect_lizard(int lizard_id, void *requester, message_to_server *send_message)
{
    int reply;

    send_message->client_id = LIZARD;
    send_message->type = DISCONNECT;
    send_message->value = lizard_id;

    zmq_send(requester, send_message, sizeof(message_to_server), 0);
    zmq_recv(requester, &reply, sizeof(int), 0);
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

    // Create lizard and connect it to the server
    int lizard_id = connect_lizard(requester, &send_message);
    if (lizard_id < 0)
    {
        zmq_close(requester);
        zmq_ctx_destroy(context);
        return 1;
    }

    // Handle lizard movement until SIGINT is received (Ctrl+C) or the user presses the "q" or "Q" keys
    move_lizard(lizard_id, requester, &send_message, &stop);

    // Disconnect lizard from server
    disconnect_lizard(lizard_id, requester, &send_message);

    // Close socket and destroy context
    zmq_close(requester);
    zmq_ctx_destroy(context);

    return 0;
}
