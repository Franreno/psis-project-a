#include "wasp-mover.h"

/**
 * @brief - Create a new wasp mover object
 *
 * @param wasp_payload - Pointer to the wasp mover
 * @param recv_message - Message received from the server
 * @param wasps - Array of wasps
 * @param num_wasps - Number of wasps
 * @param slot_wasps - Number of available slots
 * @param game_window - Game window
 */
void new_wasp_mover(wasp_mover **wasp_payload,
                    message_to_server *recv_message,
                    wasp *wasps,
                    int *num_wasps,
                    int *slot_wasps,
                    window_data *game_window)
{
    (*wasp_payload)->wasps = wasps;
    (*wasp_payload)->num_wasps = num_wasps;
    (*wasp_payload)->slot_wasps = slot_wasps;
    (*wasp_payload)->game_window = game_window;
    (*wasp_payload)->recv_message = recv_message;
}

/**
 * @brief - Process a wasp message
 *
 * @param wasp_payload - Pointer to the wasp mover
 */
void process_wasp_message(wasp_mover *wasp_payload)
{
    switch (wasp_payload->recv_message->type)
    {
    case CONNECT:
        process_wasp_connect(wasp_payload);
        break;

    case MOVEMENT:
        process_wasp_movement(wasp_payload);
        break;

    case DISCONNECT:
        process_wasp_disconnect(wasp_payload);
        break;
    }
}

/**
 * @brief - Proccess a wasp connection to the server
 *
 * @param wasp_payload - Pointer to the wasp mover
 */
void process_wasp_connect(wasp_mover *wasp_payload)
{
    int no_slots = -1;
    int new_wasp_id;

    // If there are not available slots, refuse to add wasp
    if (*(wasp_payload->slot_wasps) <= 0)
    {
        if (wasp_payload->should_use_responder)
            zmq_send(wasp_payload->responder, &no_slots, sizeof(int), 0);

        return;
    }

    // If there is an empty slot, add the wasp to the array on that slot, if not, add after the last wasp
    for (int i = 0; i < MAX_SLOTS_ALLOWED; i++)
    {
        if (wasp_payload->wasps[i].ch == (char)-1)
        {
            new_wasp_id = i;
            break;
        }
        else
            new_wasp_id = *(wasp_payload->num_wasps);
    }

    // Increment the number of wasps and decrement the available slots
    (*(wasp_payload->num_wasps))++;
    (*(wasp_payload->slot_wasps))--;

    // Initialize the wasp
    wasp_payload->wasps[new_wasp_id].ch = '#';
    wasp_payload->wasps[new_wasp_id].last_message_time = time(NULL);

    int new_x;
    int new_y;
    layer_cell *cell;

    do
    {
        // Generate a random position
        new_x = rand() % (WINDOW_SIZE - 2) + 1;
        new_y = rand() % (WINDOW_SIZE - 2) + 1;

        // Get the stack info of the new position
        cell = get_cell(wasp_payload->game_window->matrix, new_x, new_y);
    } while (cell->stack[cell->top].client_id == LIZARD || cell->stack[cell->top].client_id == ROACH || cell->stack[cell->top].client_id == WASP);

    // Once the position is valid, update the wasp position
    wasp_payload->wasps[new_wasp_id].x = new_x;
    wasp_payload->wasps[new_wasp_id].y = new_y;

    // Draw the wasp in the random position
    window_draw(wasp_payload->game_window, wasp_payload->wasps[new_wasp_id].x, wasp_payload->wasps[new_wasp_id].y, (wasp_payload->wasps[new_wasp_id].ch) | A_BOLD, WASP, new_wasp_id);

    // Reply indicating position of the wasp in the array
    if (wasp_payload->should_use_responder)
        zmq_send(wasp_payload->responder, &new_wasp_id, sizeof(int), 0);
}

/**
 * @brief - Calculate the wasp movement
 *
 * @param wasp_payload - Pointer to the wasp mover
 * @param new_x - Pointer to the new x position
 * @param new_y - Pointer to the new y position
 * @return int - 1 if the wasp can move, 0 otherwise
 */
int calculate_wasp_movement(wasp_mover *wasp_payload, int *new_x, int *new_y)
{
    // Get the wasp id
    int wasp_id = wasp_payload->recv_message->value;

    // Get the direction the wasp wants to move to
    direction_t direction = wasp_payload->recv_message->direction;

    // Calculate the new position the wasp wants to move to
    *new_x = wasp_payload->wasps[wasp_id].x;
    *new_y = wasp_payload->wasps[wasp_id].y;
    new_position(new_x, new_y, direction);

    // Get the stack info of the new position
    layer_cell *cell = get_cell(wasp_payload->game_window->matrix, *new_x, *new_y);

    // Check the top element of the stack to see if it's a lizard
    if (cell->stack[cell->top].client_id == LIZARD)
    {
        /*
        // SHOULD BE SIMILAR TO THE VERSION IN LIZARD-MOVER.C BUT HERE WE DON'T HAVE ACCESS TO THE LIZARD DATA STRUCTURE

        int lizard_id = cell->stack[cell->top].position_in_array;

        // If the cell is a lizard, the score of the lizard is decreased by the wasp damage
        int new_lizard_score = lizard_payload->lizards[lizard_id].score - WASP_DAMAGE;
        lizard_payload->lizards[lizard_id].score = new_lizard_score;

        // Check if the lizard score dropped below 0
        if (new_lizard_score < 0)
        {
            // Erase the tail
            erase_lizard_tail(lizard_payload, lizard_id, lizard_payload->lizards[lizard_id].previous_direction);
        }
        // Check if the lizard score dropped below the maximum score
        else if (new_lizard_score < MAX_LIZARD_SCORE)
        {
            // Erase the tail
            erase_lizard_tail(lizard_payload, lizard_id, lizard_payload->lizards[lizard_id].previous_direction);

            // Remove the lizards mark of a winner
            lizard_payload->lizards[lizard_id].is_winner = 0;

            // Draw the tail again
            draw_lizard_tail(lizard_payload, lizard_id, lizard_payload->lizards[lizard_id].previous_direction);
        }

        */

        return 0;
    }

    // Check the top element of the stack to see if it's a roach or a wasp
    if (cell->stack[cell->top].client_id == ROACH || cell->stack[cell->top].client_id == WASP)
        return 0;

    // Otherwise, the wasp can move to the new position

    return 1;
}

/**
 * @brief - Process the wasp movement
 *
 * @param wasp_payload - Pointer to the wasp mover
 */
void process_wasp_movement(wasp_mover *wasp_payload)
{
    int wasp_id = wasp_payload->recv_message->value;
    int wasp_not_found = 404;
    int success = 0;
    int new_x;
    int new_y;

    // Verify id the wasp id is within the range of the array
    if (wasp_id < 0 || wasp_id >= MAX_SLOTS_ALLOWED)
    {
        if (wasp_payload->should_use_responder)
            zmq_send(wasp_payload->responder, &wasp_not_found, sizeof(int), 0);

        return;
    }

    // Verify if the wasp is still in use
    if (wasp_payload->wasps[wasp_id].ch == (char)-1)
    {
        if (wasp_payload->should_use_responder)
            zmq_send(wasp_payload->responder, &wasp_not_found, sizeof(int), 0);

        return;
    }

    // Update the last message time
    wasp_payload->wasps[wasp_id].last_message_time = time(NULL);

    // If the wasp movement is calculated as valid, move the wasp
    if (calculate_wasp_movement(wasp_payload, &new_x, &new_y))
        wasp_move(wasp_payload, new_x, new_y, wasp_id);

    // Reply indicating success moving the wasp
    if (wasp_payload->should_use_responder)
        zmq_send(wasp_payload->responder, &success, sizeof(int), 0);
}

/**
 * @brief - Erase and draw the wasp in the new position
 *
 * @param wasp_payload - Pointer to the wasp mover
 * @param wasp_id - Wasp id
 * @param new_x - New x position
 * @param new_y - New y position
 */
void wasp_move(wasp_mover *wasp_payload, int new_x, int new_y, int wasp_id)
{
    // Erase the wasp from the screen
    window_erase(wasp_payload->game_window, wasp_payload->wasps[wasp_id].x, wasp_payload->wasps[wasp_id].y, (wasp_payload->wasps[wasp_id].ch) | A_BOLD);

    // Update the wasp position
    wasp_payload->wasps[wasp_id].x = new_x;
    wasp_payload->wasps[wasp_id].y = new_y;

    // Draw the wasp in the new position
    window_draw(wasp_payload->game_window, wasp_payload->wasps[wasp_id].x, wasp_payload->wasps[wasp_id].y, (wasp_payload->wasps[wasp_id].ch) | A_BOLD, WASP, wasp_id);
}

/**
 * @brief - Disconnect a wasp from the server
 *
 * @param wasp_payload - Pointer to the wasp mover
 */
void process_wasp_disconnect(wasp_mover *wasp_payload)
{
    int success = 0;
    int wasp_not_found = 404;
    int wasp_id = wasp_payload->recv_message->value;

    // Verify if the wasp id is within the range of the array
    if (wasp_id < 0 || wasp_id >= MAX_SLOTS_ALLOWED)
    {
        if (wasp_payload->should_use_responder)
            zmq_send(wasp_payload->responder, &wasp_not_found, sizeof(int), 0);

        return;
    }

    // Verify if the wasp is still in use
    if (wasp_payload->wasps[wasp_id].ch == (char)-1)
    {
        if (wasp_payload->should_use_responder)
            zmq_send(wasp_payload->responder, &success, sizeof(int), 0);

        return;
    }

    // Erase the wasp from the screen
    window_erase(wasp_payload->game_window, wasp_payload->wasps[wasp_id].x, wasp_payload->wasps[wasp_id].y, (wasp_payload->wasps[wasp_id].ch) | A_BOLD);

    // Set the wasp character to -1 to indicate it's not in use
    wasp_payload->wasps[wasp_id].ch = -1;
    wasp_payload->wasps[wasp_id].x = -1;
    wasp_payload->wasps[wasp_id].y = -1;
    wasp_payload->wasps[wasp_id].last_message_time = -1;

    if (wasp_payload->should_use_responder)
        zmq_send(wasp_payload->responder, &success, sizeof(int), 0);

    (*(wasp_payload->num_wasps))--;
    (*(wasp_payload->slot_wasps))++;
}
