#include "lizard-mover.h"

void new_lizard_mover(lizard_mover **lizard_payload,
                      message_to_server *recv_message,
                      lizard *lizards,
                      void *responder,
                      int *num_lizards,
                      int *slot_lizards,
                      window_data *game_window)
{
    (*lizard_payload)->num_lizards = num_lizards;
    (*lizard_payload)->slot_lizards = slot_lizards;
    (*lizard_payload)->lizards = lizards;
    (*lizard_payload)->game_window = game_window;
    (*lizard_payload)->responder = responder;
    (*lizard_payload)->recv_message = recv_message;
}

void process_lizard_connect(lizard_mover *lizard_payload)
{
    int failure;

    // If there are not available slots, refuse to add lizard
    if (*(lizard_payload->slot_lizards) <= 0)
    {
        failure = -1;
        // Reply indicating failure adding the lizard
        if (lizard_payload->should_use_responder)
            zmq_send(lizard_payload->responder, &failure, sizeof(int), 0);

        return;
    }

    // If there is already a lizard with chosen character, refuse to add lizard
    char new_lizard_char = lizard_payload->recv_message->value;

    // Check if there is already a lizard with the chosen character
    for (int i = 0; i < *(lizard_payload->num_lizards); i++)
    {
        failure = -2;
        if (lizard_payload->lizards[i].ch == new_lizard_char)
        {
            // Reply indicating failure adding the lizard
            if (lizard_payload->should_use_responder)
                zmq_send(lizard_payload->responder, &failure, sizeof(int), 0);

            return;
        }
    }

    // Get the id of the new lizard
    int id = *(lizard_payload->num_lizards);

    // Increment the number of lizards and decrement the available slots
    (*(lizard_payload->num_lizards))++;
    (*(lizard_payload->slot_lizards))--;

    // Initialize the lizard in a random position and with score 0
    lizard_payload->lizards[id].ch = (char)lizard_payload->recv_message->value;
    lizard_payload->lizards[id].x = rand() % (WINDOW_SIZE - 2) + 1;
    lizard_payload->lizards[id].y = rand() % (WINDOW_SIZE - 2) + 1;
    lizard_payload->lizards[id].score = 0;

    // Draw the lizard in the random position
    window_draw(lizard_payload->game_window, lizard_payload->lizards[id].x, lizard_payload->lizards[id].y, (lizard_payload->lizards[id].ch) | A_BOLD, LIZARD, id);

    // TODO - DRAW LIZARDS TAIL

    // Reply indicating position of the lizard in the array
    if (lizard_payload->should_use_responder)
        zmq_send(lizard_payload->responder, &id, sizeof(int), 0);
}

void process_lizard_inject_connect(lizard_mover *lizard_payload, lizard connected_lizard, int received_id)
{
    // Inject means it was received from the server, if received
    // from the server, we don't need to reply, and don't need to check for slots

    // Increment the number of lizards and decrement the available slots
    (*(lizard_payload->num_lizards))++;
    (*(lizard_payload->slot_lizards))--;

    // Initialize the lizard in a received position
    lizard_payload->lizards[received_id] = connected_lizard;

    // Draw the lizard in the received position
    window_draw(lizard_payload->game_window, lizard_payload->lizards[received_id].x, lizard_payload->lizards[received_id].y, (lizard_payload->lizards[received_id].ch) | A_BOLD, LIZARD, received_id);
}

int calculate_lizard_movement(lizard_mover *lizard_payload, int *new_x, int *new_y)
{
    // Need to check the stack content of the new position!
    // The stack data should contain the type of element and its position on lizards/roaches array

    // First, get the stack content of the new position
    int id = lizard_payload->recv_message->value;
    direction_t direction = lizard_payload->recv_message->direction;

    *new_x = lizard_payload->lizards[id].x;
    *new_y = lizard_payload->lizards[id].y;
    new_position(new_x, new_y, direction);

    // Get the stack of the new position
    layer_cell *cell = get_cell(lizard_payload->game_window->matrix, *new_x, *new_y);

    // If the stack is empty, just move the lizard
    if (cell->top == -1)
        return 0;

    // If the stack is not empty, check the element

    // First check if there is another lizard in the stack
    // As we have a priority stack, the lizard should always be on top!
    if (isalpha(cell->stack[cell->top].ch))
    {
        // TODO - LIZARD COLISION
        // Lizard shouldnt move
        return 1;
    }
    // Check the stack for presence of roaches (digits), count all of the roaches and save their position and value
    // Array of roaches values
    char *roaches = (char *)malloc(sizeof(char) * cell->capacity);
    int *roaches_positions = (int *)malloc(sizeof(int) * cell->capacity);
    int num_roaches = 0;

    // Iterate through the stack
    for (int i = cell->top; i >= 0; i--)
    {
        // If the element is a roach, save its value and position
        if (cell->stack[i].client_id == ROACH)
        {
            roaches[num_roaches] = cell->stack[i].ch;
            roaches_positions[num_roaches] = cell->stack[i].position_in_array;
            num_roaches++;
        }
    }

    // If there are no roaches, just move the lizard
    if (num_roaches == 0)
    {
        free(roaches);
        free(roaches_positions);
        return 0;
    }

    // If there are roaches, the lizzard should eat them all

    int lizard_update_score = 0;
    // First, mark the roaches as eaten, use the roaches positions
    for (int i = 0; i < num_roaches; i++)
    {
        // Remove the roach from the stack
        window_matrix_remove_char_from_stack(lizard_payload->game_window->matrix, *new_x, *new_y, roaches[i]);

        // Mark the roach as eaten
        lizard_payload->roaches[roaches_positions[i]].is_eaten = 1;
        lizard_payload->roaches[roaches_positions[i]].timestamp = time(NULL);
        // Point to the eaten roaches
        lizard_payload->eaten_roaches[*lizard_payload->amount_eaten_roaches] = &lizard_payload->roaches[roaches_positions[i]];
        (*(lizard_payload->amount_eaten_roaches))++;

        // Increase the lizard score
        lizard_update_score += roaches[i] - '0';
    }

    // Update the lizard score
    lizard_payload->lizards[id].score += lizard_update_score;

    // Check if the lizard has reached the maximum score
    if (lizard_payload->lizards[id].score >= MAX_LIZARD_SCORE)
    {
        // Mark the lizard as a winner
        lizard_payload->lizards[id].is_winner = 1;
    }

    // Return the lizard update score
    free(roaches);
    free(roaches_positions);
    return 0;
}

void process_lizard_movement(lizard_mover *lizard_payload)
{
    // Move the specified lizard
    int id = lizard_payload->recv_message->value;
    int score = lizard_payload->lizards[id].score;

    int new_x;
    int new_y;

    // Calculate lizard movement
    int should_move = calculate_lizard_movement(lizard_payload, &new_x, &new_y);

    if (should_move != 0)
    {
        // Reply indicating failure moving the lizard
        if (lizard_payload->should_use_responder)
            zmq_send(lizard_payload->responder, &success, sizeof(int), 0);
        return;
    }

    // Erase the lizard from the screen
    window_erase(lizard_payload->game_window, lizard_payload->lizards[id].x, lizard_payload->lizards[id].y, (lizard_payload->lizards[id].ch) | A_BOLD);

    // Update the lizard position
    lizard_payload->lizards[id].x = new_x;
    lizard_payload->lizards[id].y = new_y;

    // Draw the lizard in the new position
    window_draw(lizard_payload->game_window, lizard_payload->lizards[id].x, lizard_payload->lizards[id].y, (lizard_payload->lizards[id].ch) | A_BOLD, LIZARD, id);

    // Reply indicating success moving the lizard
    if (lizard_payload->should_use_responder)
    {
        score = lizard_payload->lizards[id].score;
        zmq_send(lizard_payload->responder, &score, sizeof(int), 0);
    }
}

void process_lizard_disconnect(lizard_mover *lizard_payload)
{
    int id = lizard_payload->recv_message->value;
    window_erase(lizard_payload->game_window, lizard_payload->lizards[id].x, lizard_payload->lizards[id].y, (lizard_payload->lizards[id].ch) | A_BOLD);
    if (lizard_payload->should_use_responder)
        zmq_send(lizard_payload->responder, &success, sizeof(int), 0);
    (*(lizard_payload->num_lizards))--;
    (*(lizard_payload->slot_lizards))++;
}

void process_lizard_message(lizard_mover *lizard_payload)
{
    switch (lizard_payload->recv_message->type)
    {
    case CONNECT:
        process_lizard_connect(lizard_payload);
        break;

    case MOVEMENT:
        process_lizard_movement(lizard_payload);
        break;

    case DISCONNECT:
        process_lizard_disconnect(lizard_payload);
        break;
    }
}

void serialize_lizard_mover(lizard_mover *lizard_payload, char **buffer, size_t *buffer_size)
{
    // Calculate the size of the buffer
    *buffer_size = sizeof(int) * 2; // num_lizards and slot_lizards

    int num_lizards = *(lizard_payload->num_lizards);
    *buffer_size += (sizeof(char) + 5 * sizeof(int)) * num_lizards; // Each lizard

    *buffer = malloc(*buffer_size);
    if (!*buffer)
    {
        exit(1);
    }

    char *ptr = *buffer;

    // Serialize num_lizards and slot_lizards
    memcpy(ptr, lizard_payload->num_lizards, sizeof(int));
    ptr += sizeof(int);
    memcpy(ptr, lizard_payload->slot_lizards, sizeof(int));
    ptr += sizeof(int);

    // Serialize each lizard
    for (int i = 0; i < num_lizards; i++)
    {
        memcpy(ptr, &(lizard_payload->lizards[i].ch), sizeof(char));
        ptr += sizeof(char);
        memcpy(ptr, &(lizard_payload->lizards[i].x), sizeof(int));
        ptr += sizeof(int);
        memcpy(ptr, &(lizard_payload->lizards[i].y), sizeof(int));
        ptr += sizeof(int);
        memcpy(ptr, &(lizard_payload->lizards[i].score), sizeof(int));
        ptr += sizeof(int);
        memcpy(ptr, &(lizard_payload->lizards[i].prev_x), sizeof(int));
        ptr += sizeof(int);
        memcpy(ptr, &(lizard_payload->lizards[i].prev_y), sizeof(int));
        ptr += sizeof(int);
    }
}

void deserialize_lizard_mover(lizard_mover *lizard_payload, char *buffer)
{
    char *ptr = buffer;

    // Allocate memory for num_lizards and slot_lizards
    lizard_payload->num_lizards = malloc(sizeof(int));
    lizard_payload->slot_lizards = malloc(sizeof(int));

    if (!lizard_payload->num_lizards || !lizard_payload->slot_lizards)
    {
        // Free allocated memory if one of them failed
        free(lizard_payload->num_lizards);
        free(lizard_payload->slot_lizards);
        return;
    }

    // Deserialize num_lizards
    memcpy(lizard_payload->num_lizards, ptr, sizeof(int));
    ptr += sizeof(int);

    // Deserialize slot_lizards
    memcpy(lizard_payload->slot_lizards, ptr, sizeof(int));
    ptr += sizeof(int);

    // Allocate memory for lizards array
    int num_lizards = *(lizard_payload->num_lizards);
    lizard_payload->lizards = malloc(sizeof(lizard) * MAX_LIZARDS_ALLOWED);
    if (!lizard_payload->lizards)
    {
        free(lizard_payload->num_lizards);
        free(lizard_payload->slot_lizards);
        return;
    }

    // Deserialize each lizard
    for (int i = 0; i < num_lizards; i++)
    {
        memcpy(&(lizard_payload->lizards[i].ch), ptr, sizeof(char));
        ptr += sizeof(char);
        memcpy(&(lizard_payload->lizards[i].x), ptr, sizeof(int));
        ptr += sizeof(int);
        memcpy(&(lizard_payload->lizards[i].y), ptr, sizeof(int));
        ptr += sizeof(int);
        memcpy(&(lizard_payload->lizards[i].score), ptr, sizeof(int));
        ptr += sizeof(int);
        memcpy(&(lizard_payload->lizards[i].prev_x), ptr, sizeof(int));
        ptr += sizeof(int);
        memcpy(&(lizard_payload->lizards[i].prev_y), ptr, sizeof(int));
        ptr += sizeof(int);
    }
}
