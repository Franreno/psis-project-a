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
    // If there are available slots, add the lizard to the array
    if (*(lizard_payload->slot_lizards) <= 0)
    {
        // Reply indicating failure adding the lizard
        zmq_send(lizard_payload->responder, &failure, sizeof(int), 0);
        return;
    }
    // Increment the number of lizards and decrement the available slots
    (*(lizard_payload->num_lizards))++;
    (*(lizard_payload->slot_lizards))--;

    int id = *(lizard_payload->num_lizards) - 1;

    // Initialize the lizard in a random position
    lizard_payload->lizards[id].ch = (char)lizard_payload->recv_message->value;
    lizard_payload->lizards[id].x = rand() % (WINDOW_SIZE - 2) + 1;
    lizard_payload->lizards[id].x = rand() % (WINDOW_SIZE - 2) + 1;
    lizard_payload->lizards[id].y = rand() % (WINDOW_SIZE - 2) + 1;
    lizard_payload->lizards[id].score = 0;

    // Draw the lizard in the random position
    window_draw(lizard_payload->game_window, lizard_payload->lizards[id].x, lizard_payload->lizards[id].y, (lizard_payload->lizards[id].ch) | A_BOLD);

    // Reply indicating position of the lizard in the array
    zmq_send(lizard_payload->responder, &id, sizeof(int), 0);
}

void process_lizard_movement(lizard_mover *lizard_payload)
{
    // Move the specified lizard
    int id = lizard_payload->recv_message->value;
    direction_t direction = lizard_payload->recv_message->direction;

    int new_x = lizard_payload->lizards[id].x;
    int new_y = lizard_payload->lizards[id].y;
    new_position(&new_x, &new_y, direction);

    chtype ch = mvinch(new_x, new_y) & A_CHARTEXT;

    if (ch != ' ' && ch != '.')
    {
        zmq_send(lizard_payload->responder, &success, sizeof(int), 0);
        return;
    }

    // Erase the lizard from the screen
    window_erase(lizard_payload->game_window, lizard_payload->lizards[id].x, lizard_payload->lizards[id].y);

    // Update the lizard position
    lizard_payload->lizards[id].x = new_x;
    lizard_payload->lizards[id].y = new_y;

    // Draw the lizard in the new position
    window_draw(lizard_payload->game_window, lizard_payload->lizards[id].x, lizard_payload->lizards[id].y, (lizard_payload->lizards[id].ch) | A_BOLD);

    // Reply indicating success moving the lizard
    zmq_send(lizard_payload->responder, &success, sizeof(int), 0);
}

void process_lizard_disconnect(lizard_mover *lizard_payload)
{
    int id = lizard_payload->recv_message->value;
    window_erase(lizard_payload->game_window, lizard_payload->lizards[id].x, lizard_payload->lizards[id].y);
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