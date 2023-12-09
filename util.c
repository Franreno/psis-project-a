#include "util.h"

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

void tail_position_calc(int *x, int *y, direction_t direction, char *overflow)
{
    *overflow = 0;
    switch (direction)
    {
    case UP:
        (*x)++;
        if (*x == WINDOW_SIZE - 1)
            *overflow = 1;
        break;
    case DOWN:
        (*x)--;
        if (*x == 0)
            *overflow = 1;
        break;
    case LEFT:
        (*y)++;
        if (*y == WINDOW_SIZE - 1)
            *overflow = 1;
        break;
    case RIGHT:
        (*y)--;
        if (*y == 0)
            *overflow = 1;
        break;
    default:
        break;
    }
}
