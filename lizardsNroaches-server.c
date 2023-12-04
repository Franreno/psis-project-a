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

#define WINDOW_SIZE 15
// #define MAX_ROACHES (WINDOW_SIZE * WINDOW_SIZE / 3)
#define MAX_ROACHES 8


void new_position(int *x, int *y, direction_t direction) {
    switch(direction) {
    case UP:
        (*x)--;
        if(*x == 0)
            *x = 2;
        break;
    case DOWN:
        (*x)++;
        if(*x == WINDOW_SIZE - 1)
            *x = WINDOW_SIZE - 3;
        break;
    case LEFT:
        (*y)--;
        if(*y == 0)
            *y = 2;
        break;
    case RIGHT:
        (*y)++;
        if(*y == WINDOW_SIZE - 1)
            *y = WINDOW_SIZE - 3;
        break;
    default:
        break;
    }
}

int main(int argc, char *argv[]) {
    // Create socket address
    char *address = DEFAULT_SERVER_ADDRESS;
    char *port = DEFAULT_SERVER_PORT;
    char *server_socket_address = malloc(strlen("tcp://") + strlen(address) + strlen(":") + strlen(port) + 1);
    strcpy(server_socket_address, "tcp://");
    strcat(server_socket_address, address);
    strcat(server_socket_address, ":");
    strcat(server_socket_address, port);
    server_socket_address = "ipc:///tmp/server"; // WORKAROUND
    printf("Connecting to server at %s\n", server_socket_address);

    // Create context
    void *context = zmq_ctx_new ();
    if(context == NULL) {
        printf("Failed to create context: %s\n", zmq_strerror(errno));
        return 1;
    }

    // Create REP socket to receive messages from clients
    void *responder = zmq_socket(context, ZMQ_REP);
    if(responder == NULL) {
        printf("Failed to create REP socket: %s\n", zmq_strerror(errno));
        zmq_ctx_destroy(context);
        return 1;
    }

    // Bind to the REP socket
    if(zmq_bind(responder, server_socket_address) != 0) {
        printf("Failed to bind REP socket: %s\n", zmq_strerror(errno));
        zmq_close(responder);
        zmq_ctx_destroy(context);
        return 1;
    }

    // Initialize ncurses
    initscr();
    cbreak();
    keypad(stdscr, TRUE);
    noecho();

    // Creates a window and draws a border
    WINDOW *game_window = newwin(WINDOW_SIZE, WINDOW_SIZE, 0, 0);
    box(game_window, 0, 0);
    wrefresh(game_window);

    // Initialize variables used for roach tracking
    int num_roaches = 0;
    int slot_roaches = MAX_ROACHES;
    roach roaches[MAX_ROACHES];
    
    message_to_server recv_message;
    direction_t direction;

    int failure = FAILURE;
    int success = SUCCESS;

    int id;

    // Seed random number generator
    srand(time(NULL));

    while(1) {
        // Receive message from one of the clients
        zmq_recv(responder, &recv_message, sizeof(message_to_server), 0);

        // If the message is from a lizard client
        if(recv_message.client_id == 1) {
            // TODO
            continue;




        // If the message received is from a roach client
        } else if(recv_message.client_id == 2) {
            // If the message received is a connect type message
            if(recv_message.type == 1) {
                // If there are available slots, add the roach to the array
                if(slot_roaches > 0) {
                    num_roaches++;
                    slot_roaches--;
                    id = num_roaches - 1;
                    // Initialize the roach in a random position
                    roaches[id].ch = recv_message.value;
                    roaches[id].x = rand() % (WINDOW_SIZE - 2) + 1;
                    roaches[id].y = rand() % (WINDOW_SIZE - 2) + 1;
                    // Draw the roach in the random position
                    wmove(game_window, roaches[id].x, roaches[id].y);
                    waddch(game_window, roaches[id].ch+48 | A_BOLD);
                    wrefresh(game_window);

                    // Reply indicating position of the roach in the array
                    zmq_send(responder, &id, sizeof(int), 0);
                } else {
                    // Reply indicating failure adding the roach
                    zmq_send(responder, &failure, sizeof(int), 0);
                }
            // If the message received is a movement type message
            } else if(recv_message.type == 2) {
                // Move the specified roach
                id = recv_message.value;
                direction = recv_message.direction;
                // Erase the roach from the screen
                wmove(game_window, roaches[id].x, roaches[id].y);
                waddch(game_window, ' '); // MIGHT CAUSE A PROBLEM
                new_position(&roaches[id].x, &roaches[id].y, direction);
                // Draw the roach in the new position
                wmove(game_window, roaches[id].x, roaches[id].y);
                waddch(game_window, roaches[id].ch+48| A_BOLD);
                wrefresh(game_window);
                // Reply indicating success moving the roach
                zmq_send(responder, &success, sizeof(int), 0);
            }




        // If the message is from a display-app client
        } else if(recv_message.client_id == 3) {
            // TODO
            continue;
        }
    }

    endwin();
    zmq_close(responder);
    zmq_ctx_destroy(context);

    return 0;
}