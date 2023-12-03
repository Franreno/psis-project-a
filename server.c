#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>

#include <zmq.h>

#include <ncurses.h>

#include "remote-char.h"

#define WINDOW_SIZE 15

direction_t random_direction()
{
    return random() % 4;
}

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

int main(int argc, char* argv []) {
    /* Create context */
    void *context = zmq_ctx_new ();
    if(context == NULL) {
        printf("Failed to create context: %s\n", zmq_strerror(errno));
        return 1;
    }

    /* Create REP socket to receive messages from clients */
    void* responder = zmq_socket(context, ZMQ_REP);
    if(responder == NULL) {
        printf("Failed to create REP socket: %s\n", zmq_strerror(errno));
        zmq_ctx_destroy(context);
        return 1;
    }

    /* Bind to the REP socket */
    if(zmq_bind(responder, SERVER_SOCKET_IP) != 0) {
        printf("Failed to bind REP socket: %s\n", zmq_strerror(errno));
        zmq_close(responder);
        zmq_ctx_destroy(context);
        return 1;
    }

    // ncurses initialization
    initscr();
    cbreak();
    keypad(stdscr, TRUE);
    noecho();

    /* creates a window and draws a border */
    WINDOW *my_win = newwin(WINDOW_SIZE, WINDOW_SIZE, 0, 0);
    box(my_win, 0, 0);
    wrefresh(my_win);

    /* information about the character */
    user *users;
    users = malloc(sizeof(user) * 10);

    int ch;
    int pos_x;
    int pos_y;

    direction_t direction;
    message request_message;

    int amount_of_users = 0;
    int success = 1;
    while(1) {
        //read(request_fd, &request_message, sizeof(message));
        zmq_recv(responder, &request_message, sizeof(message) - 1, 0);
        zmq_send(responder, &success, sizeof(success), 0);

        if (request_message.msg_type == 0)
        {
            if (amount_of_users > 10)
            {
                printf("Too many users\n");
                continue;
            }

            ch = request_message.ch;
            pos_x = WINDOW_SIZE / 2;
            pos_y = WINDOW_SIZE / 2;

            // add to user struct
            user u;
            u.ch = ch;
            u.x = pos_x;
            u.y = pos_y;
            users[amount_of_users] = u;
            amount_of_users++;
        }

        if (request_message.msg_type == 1) {
            // find user of ch
            int user_id;
            for (int i = 0; i < amount_of_users; i++)
            {
                if (users[i].ch == request_message.ch)
                {
                    user_id = i;
                    break;
                }
            }

            wmove(my_win, users[user_id].x, users[user_id].y);
            waddch(my_win, ' ');

            direction = request_message.direction;
            new_position(&users[user_id].x, &users[user_id].y, direction);

            wmove(my_win, users[user_id].x, users[user_id].y);
            waddch(my_win, users[user_id].ch | A_BOLD);
            wrefresh(my_win);
        }
    }

    endwin();
    zmq_close(responder);
    zmq_ctx_destroy(context);
    free(users);

    return 0;
}