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

Direction convert_direction_to_protobuf_direction_enum(direction_t direction)
{
    switch (direction)
    {
    case UP:
        return DIRECTION__UP;
    case DOWN:
        return DIRECTION__DOWN;
    case LEFT:
        return DIRECTION__LEFT;
    case RIGHT:
        return DIRECTION__RIGHT;
    }

    return DIRECTION__UP;
}

direction_t convert_protobuf_direction_enum_to_direction_t(Direction direction)
{
    switch (direction)
    {
    case DIRECTION__UP:
        return UP;
    case DIRECTION__DOWN:
        return DOWN;
    case DIRECTION__LEFT:
        return LEFT;
    case DIRECTION__RIGHT:
        return RIGHT;
    }

    return UP;
}

void convert_proto_message_to_server_to_message_to_server(message_to_server *new_msg, MessageToServer *msg)
{
    new_msg->client_id = msg->client_id;
    new_msg->type = msg->type;
    new_msg->value = msg->value;
    new_msg->direction = convert_protobuf_direction_enum_to_direction_t(msg->direction);
    new_msg->message_accepted = msg->message_accepted;
}

void convert_message_to_server_to_proto_message_to_server(MessageToServer *new_msg, message_to_server *msg)
{
    new_msg->client_id = msg->client_id;
    new_msg->type = msg->type;
    new_msg->value = msg->value;
    new_msg->direction = convert_direction_to_protobuf_direction_enum(msg->direction);
    new_msg->message_accepted = msg->message_accepted;
}