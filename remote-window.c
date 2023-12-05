
#include <ncurses.h>
#include <zmq.h>
#include "remote-char.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#define WINDOW_SIZE 15

int main()
{

    // initialize zmq to subscribe to the publisher
    void *context = zmq_ctx_new();
    void *subscriber = zmq_socket(context, ZMQ_SUB);
    zmq_connect(subscriber, "tcp://127.0.0.1:5556");
    zmq_setsockopt(subscriber, ZMQ_SUBSCRIBE, "positions_update", 3);

    // ncurses initialization
    initscr();
    cbreak();
    keypad(stdscr, TRUE);
    noecho();

    /* creates a window and draws a border */
    WINDOW *my_win = newwin(WINDOW_SIZE, WINDOW_SIZE, 0, 0);
    box(my_win, 0, 0);
    wrefresh(my_win);

    direction_t direction;
    window_update update_message;

    int amount_of_users;
    user *users;
    while (1)
    {
        // receive the update message
        zmq_recv(subscriber, &update_message, sizeof(window_update), 0);
        amount_of_users = update_message.amount_of_users;
        users = malloc(sizeof(user) * amount_of_users);
        users = update_message.users;

        // Clear board
        werase(my_win);

        // process the movement message
        for (int i = 0; i < amount_of_users; i++)
        {
            wmove(my_win, users[i].x, users[i].y);
            waddch(my_win, users[i].ch | A_BOLD);
        }

        wrefresh(my_win);
        free(users);
    }
    endwin();
    zmq_close(subscriber);
    zmq_ctx_destroy(context);

    return 0;
}