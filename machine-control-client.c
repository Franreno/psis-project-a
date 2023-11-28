#include "remote-char.h"
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>

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
    int fd;
    open_fifo_file(FIFO_LOCATION, &fd, O_WRONLY);

    printf("fifo just opened\n");

    // TODO_5
    char character = 'k';

    // TODO_6
    message m;
    m.msg_type = 0;
    m.ch = character;
    m.direction = 0;
    write(fd, &m, sizeof(message));

    int sleep_delay;
    direction_t direction;
    int n = 0;
    m.msg_type = 1;
    while (1)
    {
        sleep_delay = random() % 700000;
        usleep(sleep_delay);
        direction = random() % 4;
        n++;
        switch (direction)
        {
        case LEFT:
            printf("%d Going Left   ", n);
            break;
        case RIGHT:
            printf("%d Going Right   ", n);
            break;
        case DOWN:
            printf("%d Going Down   ", n);
            break;
        case UP:
            printf("%d Going Up    ", n);
            break;
        }
        // TODO_9
        //  prepare the movement message
        m.direction = direction;

        // TODO_10
        // send the movement message
        write(fd, &m, sizeof(message));
    }

    return 0;
}