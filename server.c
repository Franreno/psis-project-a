
#include <ncurses.h>
#include "remote-char.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#define WINDOW_SIZE 15

direction_t random_direction()
{
    return random() % 4;
}
void new_position(int *x, int *y, direction_t direction)
{
    switch (direction)
    {
    case UP:
        (*x)--;
        if (*x == 0)
            *x = 2;
        break;
    case DOWN:
        (*x)++;
        if (*x == WINDOW_SIZE - 1)
            *x = WINDOW_SIZE - 3;
        break;
    case LEFT:
        (*y)--;
        if (*y == 0)
            *y = 2;
        break;
    case RIGHT:
        (*y)++;
        if (*y == WINDOW_SIZE - 1)
            *y = WINDOW_SIZE - 3;
        break;
    default:
        break;
    }
}
void open_fifo_file(char *path, int *fd, int mode)
{
    while ((*fd = open(path, mode)) == -1)
    {
        if (mkfifo(path, 0666) != 0)
        {
            printf("problem creating the fifo\n");
            exit(-1);
        }
        else
            printf("fifo created\n");
    }
}

int main()
{

    // TODO_3
    // create and open the FIFO for reading
    int request_fd;
    open_fifo_file(FIFO_LOCATION, &request_fd, O_RDONLY);

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
    while (1)
    {
        // TODO_7
        // receive message from the clients
        read(request_fd, &request_message, sizeof(message));

        // TODO_8
        //  process connection messages
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

        // TODO_11
        // process the movement message
        if (request_message.msg_type == 1)
        {
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
    endwin(); /* End curses mode		  */

    free(users);

    return 0;
}