#include <ncurses.h>
#include "remote-char.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <stdlib.h>

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

    // TODO_4
    //  create and open the FIFO for writing
    int fd;
    open_fifo_file(FIFO_LOCATION, &fd, O_WRONLY);

    // TODO_5
    //  read the character from the user
    char character;
    printf("Enter a character: ");
    scanf("%c", &character);

    // TODO_6
    // send connection message
    message m;
    m.msg_type = 0;
    m.ch = character;
    m.direction = 0;
    write(fd, &m, sizeof(message));

    initscr();            /* Start curses mode 		*/
    cbreak();             /* Line buffering disabled	*/
    keypad(stdscr, TRUE); /* We get F1, F2 etc..		*/
    noecho();             /* Don't echo() while we do getch */

    int ch;
    int ch_to_direction;

    m.msg_type = 1;
    int n = 0;
    do
    {
        ch = getch();
        n++;
        switch (ch)
        {
        case KEY_LEFT:
            mvprintw(0, 0, "%d Left arrow is pressed", n);
            ch_to_direction = LEFT;
            break;
        case KEY_RIGHT:
            mvprintw(0, 0, "%d Right arrow is pressed", n);
            ch_to_direction = RIGHT;
            break;
        case KEY_DOWN:
            mvprintw(0, 0, "%d Down arrow is pressed", n);
            ch_to_direction = DOWN;
            break;
        case KEY_UP:
            mvprintw(0, 0, "%d :Up arrow is pressed", n);
            ch_to_direction = UP;
            break;
        default:
            ch = 'x';
            break;
        }
        refresh(); /* Print it on to the real screen */
        // TODO_9
        //  prepare the movement message
        if (ch == 'x')
            continue;

        m.direction = ch_to_direction;

        // TODO_10
        // send the movement message
        write(fd, &m, sizeof(message));

    } while (ch != 27);

    endwin(); /* End curses mode		  */

    return 0;
}