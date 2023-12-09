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
    int server_reply = 0;
    char new_lizard_char;
    int new_lizard_id;

    // If there are not available slots, refuse to add lizard
    if (*(lizard_payload->slot_lizards) <= 0)
    {
        server_reply = -1;
        // Reply indicating failure adding the lizard due to lack of slots
        if (lizard_payload->should_use_responder)
        {
            zmq_send(lizard_payload->responder, &server_reply, sizeof(int), 0);
        }

        return;
    }

    // Get the character of the new lizard
    new_lizard_char = lizard_payload->recv_message->value;

    // If there is already a lizard with chosen character, refuse to add lizard
    for (int i = 0; i < *(lizard_payload->num_lizards); i++)
    {
        server_reply = -2;
        if (lizard_payload->lizards[i].ch == new_lizard_char)
        {
            // Reply indicating failure adding the lizard due to choice of repeated character
            if (lizard_payload->should_use_responder)
                zmq_send(lizard_payload->responder, &server_reply, sizeof(int), 0);

            return;
        }
    }

    // If there is an empty slot, add the lizard to the array on that slot, if not, add after the last lizard
    for (int i = 0; i < MAX_LIZARDS_ALLOWED; i++)
    {
        if (lizard_payload->lizards[i].ch == -1)
        {
            new_lizard_id = i;
            break;
        }
        else
            new_lizard_id = *(lizard_payload->num_lizards);
    }

    // Increment the number of lizards and decrement the available slots
    (*(lizard_payload->num_lizards))++;
    (*(lizard_payload->slot_lizards))--;

    // Initialize the lizard in a valid random position and with score 0
    lizard_payload->lizards[new_lizard_id].ch = (char)lizard_payload->recv_message->value;
    lizard_payload->lizards[new_lizard_id].x = rand() % (WINDOW_SIZE - 2) + 1; // TODO - CHECK IF POSITION IS VALID
    lizard_payload->lizards[new_lizard_id].y = rand() % (WINDOW_SIZE - 2) + 1; // TODO - CHECK IF POSITION IS VALID
    lizard_payload->lizards[new_lizard_id].previous_direction = rand() % 4;
    lizard_payload->lizards[new_lizard_id].score = 0;

    // Draw the lizard in the random position
    window_draw(lizard_payload->game_window, lizard_payload->lizards[new_lizard_id].x, lizard_payload->lizards[new_lizard_id].y, (lizard_payload->lizards[new_lizard_id].ch) | A_BOLD, LIZARD, new_lizard_id);

    // TODO - DRAW LIZARDS TAIL
    draw_lizard_tail(lizard_payload, new_lizard_id, lizard_payload->lizards[new_lizard_id].previous_direction);

    // Reply to lizard client indicating position of the new lizard in the array
    if (lizard_payload->should_use_responder)
    {
        zmq_send(lizard_payload->responder, &new_lizard_id, sizeof(int), 0);
    }
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
    lizard_draw(lizard_payload, received_id);
}

int calculate_lizard_movement(lizard_mover *lizard_payload, int *new_x, int *new_y)
{
    // Get the lizzard id and direction
    int lizard_id = lizard_payload->recv_message->value;
    direction_t direction = lizard_payload->recv_message->direction;

    // Calculate the new position the lizard wants to move to
    *new_x = lizard_payload->lizards[lizard_id].x;
    *new_y = lizard_payload->lizards[lizard_id].y;
    new_position(new_x, new_y, direction);

    // Get the stack info of the new position
    layer_cell *cell = get_cell(lizard_payload->game_window->matrix, *new_x, *new_y);

    // Check the top element of the stack to see if it's a lizard
    if (cell->stack[cell->top].client_id == LIZARD)
    {
        int id_1 = lizard_id;
        int id_2 = cell->stack[cell->top].position_in_array;

        // If the cell is another lizard, score of both lizards scores should be updated to the sum of the scores devided by 2
        int new_lizard_score = (lizard_payload->lizards[id_1].score + lizard_payload->lizards[id_2].score) / 2;

        // Update the lizards scores
        lizard_payload->lizards[id_1].score = new_lizard_score;
        lizard_payload->lizards[id_2].score = new_lizard_score;

        // Check if the lizards dropped below the maximum score
        if (new_lizard_score < MAX_LIZARD_SCORE)
        {
            // erase the tail just to make sure
            erase_lizard_tail(lizard_payload, id_1, lizard_payload->lizards[id_1].previous_direction);
            erase_lizard_tail(lizard_payload, id_2, lizard_payload->lizards[id_2].previous_direction);

            // Remove the lizards mark of a winner
            lizard_payload->lizards[id_1].is_winner = 0;
            lizard_payload->lizards[id_2].is_winner = 0;

            // Draw the tails again
            draw_lizard_tail(lizard_payload, id_1, lizard_payload->lizards[id_1].previous_direction);
            draw_lizard_tail(lizard_payload, id_2, lizard_payload->lizards[id_2].previous_direction);
        }

        return 0;
    }

    // Check the top element of the stack to see if it's a roach
    if (cell->stack[cell->top].client_id == ROACH)
    {
        char *roaches = (char *)malloc(sizeof(char) * cell->capacity);
        int *roaches_positions = (int *)malloc(sizeof(int) * cell->capacity);
        int num_roaches = 0;
        int new_lizard_score = 0;

        // Iterate through the stack and check for presence of roaches (digits), count all of the roaches and save their positions and values
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
            new_lizard_score += roaches[i] - '0';
        }

        // Update the lizard score
        lizard_payload->lizards[lizard_id].score += new_lizard_score;

        // Check if the lizard has reached the maximum score
        if (lizard_payload->lizards[lizard_id].score >= MAX_LIZARD_SCORE)
        {
            // Mark the lizard as a winner and update the lizard score to the maximum score
            erase_lizard_tail(lizard_payload, lizard_id, lizard_payload->lizards[lizard_id].previous_direction);
            lizard_payload->lizards[lizard_id].is_winner = 1;
            lizard_payload->lizards[lizard_id].score = MAX_LIZARD_SCORE;
            // Erase the old tail and draw the new tail
            draw_lizard_tail(lizard_payload, lizard_id, lizard_payload->lizards[lizard_id].previous_direction);
        }

        // Free the allocated memory
        free(roaches);
        free(roaches_positions);
    }

    return 1;
}

void draw_lizard_tail(lizard_mover *lizard_payload, int lizard_id, direction_t tail_direction)
{
    lizard *lizard = &lizard_payload->lizards[lizard_id];
    char char_to_draw = lizard->is_winner ? '*' : '.';

    // Draw the lizard tail
    int tail_x = lizard->x;
    int tail_y = lizard->y;
    char overflow = 0;

    for (int i = 0; i < 5; i++)
    {
        // Calculate the new position and check if it's valid
        tail_position_calc(&tail_x, &tail_y, tail_direction, &overflow);
        if (overflow)
            break;

        // Draw the tail
        window_draw(lizard_payload->game_window, tail_x, tail_y, char_to_draw, LIZARD_BODY, lizard_id);
    }
}

void erase_lizard_tail(lizard_mover *lizard_payload, int lizard_id, direction_t tail_direction)
{
    lizard *lizard = &lizard_payload->lizards[lizard_id];
    char char_to_erase = lizard->is_winner ? '*' : '.';

    // Draw the lizard tail
    int tail_x = lizard->x;
    int tail_y = lizard->y;
    char overflow = 0;

    for (int i = 0; i < 5; i++)
    {
        // Calculate the new position and check if it's valid
        tail_position_calc(&tail_x, &tail_y, tail_direction, &overflow);
        if (overflow)
            break;

        // erase the tail
        window_erase(lizard_payload->game_window, tail_x, tail_y, char_to_erase | A_BOLD);
    }
}

void process_lizard_movement(lizard_mover *lizard_payload)
{
    int lizard_id = lizard_payload->recv_message->value;
    int new_x;
    int new_y;

    // If the lizard movement is calculated as valid, move the lizard
    if (calculate_lizard_movement(lizard_payload, &new_x, &new_y))
    {
        lizard_payload->recv_message->message_accepted = 1;
        lizard_move(lizard_payload, lizard_id, new_x, new_y);
        // Update the previous direction
        lizard_payload->lizards[lizard_id].previous_direction = lizard_payload->recv_message->direction;
    }
    else
    {
        lizard_payload->recv_message->message_accepted = 0;
    }

    // Reply with the lizard score
    if (lizard_payload->should_use_responder)
        zmq_send(lizard_payload->responder, &lizard_payload->lizards[lizard_id].score, sizeof(int), 0);
}

void lizard_draw(lizard_mover *lizard_payload, int lizard_id)
{
    // Draw the lizard in the new position
    window_draw(lizard_payload->game_window, lizard_payload->lizards[lizard_id].x, lizard_payload->lizards[lizard_id].y, (lizard_payload->lizards[lizard_id].ch) | A_BOLD, LIZARD, lizard_id);
    draw_lizard_tail(lizard_payload, lizard_id, lizard_payload->recv_message->direction);
}

void lizard_erase(lizard_mover *lizard_payload, int lizard_id)
{
    // Erase the lizard from the screen
    window_erase(lizard_payload->game_window, lizard_payload->lizards[lizard_id].x, lizard_payload->lizards[lizard_id].y, (lizard_payload->lizards[lizard_id].ch) | A_BOLD);
    erase_lizard_tail(lizard_payload, lizard_id, lizard_payload->lizards[lizard_id].previous_direction);
}

void lizard_move(lizard_mover *lizard_payload, int lizard_id, int new_x, int new_y)
{
    lizard_erase(lizard_payload, lizard_id);

    // Update the lizard position
    lizard_payload->lizards[lizard_id].x = new_x;
    lizard_payload->lizards[lizard_id].y = new_y;

    lizard_draw(lizard_payload, lizard_id);
}

void process_lizard_disconnect(lizard_mover *lizard_payload)
{
    int lizard_id = lizard_payload->recv_message->value;
    int success = 0;

    // Erase the lizard from the screen
    lizard_erase(lizard_payload, lizard_id);

    // Set the lizard character to -1 to indicate it's not in use
    lizard_payload->lizards[lizard_id].ch = -1;
    lizard_payload->lizards[lizard_id].x = -1;
    lizard_payload->lizards[lizard_id].y = -1;
    lizard_payload->lizards[lizard_id].score = 0;
    lizard_payload->lizards[lizard_id].is_winner = 0;

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
    *buffer_size += (sizeof(char) + 4 * sizeof(int)) * num_lizards; // Each lizard

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
        memcpy(ptr, &(lizard_payload->lizards[i].previous_direction), sizeof(int));
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
        memcpy(&(lizard_payload->lizards[i].previous_direction), ptr, sizeof(int));
        ptr += sizeof(int);
    }
}
