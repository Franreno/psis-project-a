#include "util.h"

/**
 * @brief - Calculates the new position based on the direction
 *
 * @param x - Pointer to the x coordinate
 * @param y - Pointer to the y coordinate
 * @param direction - Direction to move
 */
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

/**
 * @brief - Calculates the new position based on the direction and handles overflow
 *
 * @param x - Pointer to the x coordinate
 * @param y - Pointer to the y coordinate
 * @param direction - Direction to move
 * @param overflow - Pointer to the overflow variable
 */
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
