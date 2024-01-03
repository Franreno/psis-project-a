#include "roach-mover.h"

/**
 * @brief - Create a new roach mover
 *
 * @param roach_payload - Pointer to the roach mover
 * @param recv_message - Message received from the server
 * @param roaches - Array of roaches
 * @param responder - ZMQ socket
 * @param num_roaches - Number of roaches
 * @param slot_roaches - Number of available slots
 * @param game_window - Game window
 */
void new_roach_mover(roach_mover **roach_payload,
                     message_to_server *recv_message,
                     roach *roaches,
                     void *responder,
                     int *num_roaches,
                     int *slot_roaches,
                     window_data *game_window)
{
    (*roach_payload)->roaches = roaches;
    (*roach_payload)->responder = responder;
    (*roach_payload)->num_roaches = num_roaches;
    (*roach_payload)->slot_roaches = slot_roaches;
    (*roach_payload)->game_window = game_window;
    (*roach_payload)->recv_message = recv_message;
}

/**
 * @brief - Process a roach message
 *
 * @param roach_payload - Pointer to the roach mover
 */
void process_roach_message(roach_mover *roach_payload)
{
    switch (roach_payload->recv_message->type)
    {
    case CONNECT:
        process_roach_connect(roach_payload);
        break;

    case MOVEMENT:
        process_roach_movement(roach_payload);
        break;

    case DISCONNECT:
        process_roach_disconnect(roach_payload);
        break;
    }
}

/**
 * @brief - Proccess a roach connection to the server
 *
 * @param roach_payload - Pointer to the roach mover
 */
void process_roach_connect(roach_mover *roach_payload)
{
    int new_roach_id;

    // If there are not available slots, refuse to add roach
    if (*(roach_payload->slot_roaches) <= 0)
    {
        new_roach_id = -1;
        if (roach_payload->should_use_responder)
            // TODO: ADD PROTO ENCODERs
            zmq_send(roach_payload->responder, &new_roach_id, sizeof(int), 0);

        return;
    }

    // Get the id of the new roach
    new_roach_id = *(roach_payload->num_roaches);

    // Increment the number of roaches and decrement the available slots
    (*(roach_payload->num_roaches))++;
    (*(roach_payload->slot_roaches))--;

    // Initialize the roach
    roach_payload->roaches[new_roach_id].ch = roach_payload->recv_message->value;

    int new_x;
    int new_y;
    layer_cell *cell;

    do
    {
        // Generate a random position
        new_x = rand() % (WINDOW_SIZE - 2) + 1;
        new_y = rand() % (WINDOW_SIZE - 2) + 1;

        // Get the stack info of the new position
        cell = get_cell(roach_payload->game_window->matrix, new_x, new_y);
    } while (cell->stack[cell->top].client_id == LIZARD || cell->stack[cell->top].client_id == ROACH || cell->stack[cell->top].client_id == WASP);

    // Once the position is valid, update the roach position
    roach_payload->roaches[new_roach_id].x = new_x;
    roach_payload->roaches[new_roach_id].y = new_y;

    // Draw the roach in the random position
    window_draw(roach_payload->game_window, roach_payload->roaches[new_roach_id].x, roach_payload->roaches[new_roach_id].y, (roach_payload->roaches[new_roach_id].ch + '0') | A_BOLD, ROACH, new_roach_id);

    // Reply indicating position of the roach in the array
    if (roach_payload->should_use_responder)
        // TODO: ADD PROTO ENCODER
        zmq_send(roach_payload->responder, &new_roach_id, sizeof(int), 0);
}

/**
 * @brief - Process a connection to the display app
 *
 * @param roach_payload - Pointer to the roach mover
 * @param connected_roach - Roach to inject
 * @param received_id - Position of the roach in the array
 */
void process_roach_inject_connect(roach_mover *roach_payload, roach connected_roach, int received_id)
{
    // Inject means it was received from the server, if received
    // from the server, we don't need to reply, and don't need to check for slots

    // Increment the number of roaches and decrement the available slots
    (*(roach_payload->num_roaches))++;
    (*(roach_payload->slot_roaches))--;

    // Initialize the roach in a received position
    roach_payload->roaches[received_id] = connected_roach;

    roach_move(roach_payload, roach_payload->roaches[received_id].x, roach_payload->roaches[received_id].y, received_id);
}

/**
 * @brief - Calculate the new position of the roach
 *
 * @param roach_payload - Pointer to the roach mover
 * @param new_x - Pointer to the new x position
 * @param new_y - Pointer to the new y position
 * @return int - 1 if the roach can move, 0 otherwise
 */
int calculate_roach_movement(roach_mover *roach_payload, int *new_x, int *new_y)
{
    // Get the roach id
    int roach_id = roach_payload->recv_message->value;

    // Check if the roach is eaten, if it is, don't move the roach
    if (roach_payload->roaches[roach_id].is_eaten)
        return 0;

    // Get the direction the roach wants to move to
    direction_t direction = roach_payload->recv_message->direction;

    // Calculate the new position the roach wants to move to
    *new_x = roach_payload->roaches[roach_id].x;
    *new_y = roach_payload->roaches[roach_id].y;
    new_position(new_x, new_y, direction);

    // Get the stack info of the new position
    layer_cell *cell = get_cell(roach_payload->game_window->matrix, *new_x, *new_y);

    // Check the top element of the stack to see if it's a lizard, if it is don't move the roach
    if (cell->top >= 0 && cell->stack[cell->top].client_id == LIZARD)
        return 0;

    // Otherwise, the roach can move to the new position

    return 1;
}

/**
 * @brief - Draw the tail of the roach if necessary
 *
 * @param roach_payload - Pointer to the roach mover
 * @param roach_id - Roach id
 * @param tail_direction - Direction of the tail
 */
int refresh_eaten_roach_for_display(roach_mover *roach_payload, int new_x, int new_y, char is_eaten)
{
    int roach_id = roach_payload->recv_message->value;
    // get the roach from the array
    roach *roach = &(roach_payload->roaches[roach_id]);

    // Had change
    if (roach->is_eaten == 1 && is_eaten == 0)
    {
        roach->is_eaten = 0;
        roach->x = new_x;
        roach->y = new_y;
        roach_move(roach_payload, new_x, new_y, roach_id);
        return 0;
    }

    return 1;
}

/**
 * @brief - Process the roach movement
 *
 * @param roach_payload  - Pointer to the roach mover
 */
void process_roach_movement(roach_mover *roach_payload)
{
    int success = 0;
    int roach_id = roach_payload->recv_message->value;
    int new_x;
    int new_y;

    if (calculate_roach_movement(roach_payload, &new_x, &new_y))
        roach_move(roach_payload, new_x, new_y, roach_id);

    // Reply indicating success moving the roach
    if (roach_payload->should_use_responder)
        // TODO: ADD PROTO ENCODER
        zmq_send(roach_payload->responder, &success, sizeof(int), 0);
}

/**
 * @brief - Erase and draw the roach in the new position
 *
 * @param roach_payload - Pointer to the roach mover
 * @param roach_id - Roach id
 * @param new_x - New x position
 * @param new_y - New y position
 */
void roach_move(roach_mover *roach_payload, int new_x, int new_y, int roach_id)
{
    // Erase the roach from the screen
    window_erase(roach_payload->game_window, roach_payload->roaches[roach_id].x, roach_payload->roaches[roach_id].y, (roach_payload->roaches[roach_id].ch + '0') | A_BOLD);

    // Update the roach position
    roach_payload->roaches[roach_id].x = new_x;
    roach_payload->roaches[roach_id].y = new_y;

    // Draw the roach in the new position
    window_draw(roach_payload->game_window, roach_payload->roaches[roach_id].x, roach_payload->roaches[roach_id].y, (roach_payload->roaches[roach_id].ch + '0') | A_BOLD, ROACH, roach_id);
}

/**
 * @brief - Disconnect a roach from the server
 *
 * @param roach_payload - Pointer to the roach mover
 */
void process_roach_disconnect(roach_mover *roach_payload)
{
    int success = 0;
    int roach_id = roach_payload->recv_message->value;

    window_erase(roach_payload->game_window, roach_payload->roaches[roach_id].x, roach_payload->roaches[roach_id].y, (roach_payload->roaches[roach_id].ch + '0') | A_BOLD);

    if (roach_payload->should_use_responder)
        // TODO: ADD PROTO ENCODER
        zmq_send(roach_payload->responder, &success, sizeof(int), 0);

    (*(roach_payload->num_roaches))--;
    (*(roach_payload->slot_roaches))++;
}

/**
 * @brief - Serialize the roach mover
 *
 * @param roach_payload  - Pointer to the roach mover
 * @param buffer - Pointer to the buffer
 * @param buffer_size - Pointer to the buffer size
 */
void serialize_roach_mover(roach_mover *roach_payload, char **buffer, size_t *buffer_size)
{
    // Calculate the size of the buffer
    *buffer_size = sizeof(int) * 2; // num_roaches and slot_roaches

    int num_roaches = *(roach_payload->num_roaches);
    *buffer_size += (sizeof(char) + 2 * sizeof(int)) * num_roaches; // Each roach

    *buffer = malloc(*buffer_size);
    if (!*buffer)
    {
        exit(1);
    }

    char *ptr = *buffer;

    // Serialize num_roaches and slot_roaches
    memcpy(ptr, roach_payload->num_roaches, sizeof(int));
    ptr += sizeof(int);
    memcpy(ptr, roach_payload->slot_roaches, sizeof(int));
    ptr += sizeof(int);

    // Serialize each roach
    for (int i = 0; i < num_roaches; i++)
    {
        memcpy(ptr, &(roach_payload->roaches[i].ch), sizeof(char));
        ptr += sizeof(char);
        memcpy(ptr, &(roach_payload->roaches[i].x), sizeof(int));
        ptr += sizeof(int);
        memcpy(ptr, &(roach_payload->roaches[i].y), sizeof(int));
        ptr += sizeof(int);
    }
}

/**
 * @brief - Deserialize the roach mover
 *
 * @param roach_payload - Pointer to the roach mover
 * @param buffer - Pointer to the buffer
 */
void deserialize_roach_mover(roach_mover *roach_payload, char *buffer)
{
    char *ptr = buffer;

    // Allocate memory for num_roaches and slot_roaches
    roach_payload->num_roaches = malloc(sizeof(int));
    roach_payload->slot_roaches = malloc(sizeof(int));

    if (!roach_payload->num_roaches || !roach_payload->slot_roaches)
    {
        // Free allocated memory if one of them failed
        free(roach_payload->num_roaches);
        free(roach_payload->slot_roaches);
        return;
    }

    // Deserialize num_roaches
    memcpy(roach_payload->num_roaches, ptr, sizeof(int));
    ptr += sizeof(int);

    // Deserialize slot_roaches
    memcpy(roach_payload->slot_roaches, ptr, sizeof(int));
    ptr += sizeof(int);

    // Allocate memory for roaches array
    int num_roaches = *(roach_payload->num_roaches);
    roach_payload->roaches = malloc(sizeof(roach) * MAX_SLOTS_ALLOWED);
    if (!roach_payload->roaches)
    {
        free(roach_payload->num_roaches);
        free(roach_payload->slot_roaches);
        return;
    }

    // Deserialize each roach
    for (int i = 0; i < num_roaches; i++)
    {
        memcpy(&(roach_payload->roaches[i].ch), ptr, sizeof(char));
        ptr += sizeof(char);
        memcpy(&(roach_payload->roaches[i].x), ptr, sizeof(int));
        ptr += sizeof(int);
        memcpy(&(roach_payload->roaches[i].y), ptr, sizeof(int));
        ptr += sizeof(int);
    }
}
