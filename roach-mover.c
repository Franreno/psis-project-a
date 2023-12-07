#include "roach-mover.h"

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

void process_roach_connect(roach_mover *roach_payload)
{
    // If there are available slots, add the roach to the array
    if (*(roach_payload->slot_roaches) <= 0)
    {
        // Reply indicating failure adding the roach
        zmq_send(roach_payload->responder, &failure, sizeof(int), 0);
        return;
    }
    // Increment the number of roaches and decrement the available slots
    (*(roach_payload->num_roaches))++;
    (*(roach_payload->slot_roaches))--;

    int id = *(roach_payload->num_roaches) - 1;

    // Initialize the roach in a random position
    roach_payload->roaches[id].ch = roach_payload->recv_message->value;
    roach_payload->roaches[id].x = rand() % (WINDOW_SIZE - 2) + 1;
    roach_payload->roaches[id].y = rand() % (WINDOW_SIZE - 2) + 1;

    // Draw the roach in the random position
    window_draw(roach_payload->game_window, roach_payload->roaches[id].x, roach_payload->roaches[id].y, (roach_payload->roaches[id].ch + 48) | A_BOLD);

    // Reply indicating position of the roach in the array
    zmq_send(roach_payload->responder, &id, sizeof(int), 0);
}

void process_roach_movement(roach_mover *roach_payload)
{
    // Move the specified roach
    int id = roach_payload->recv_message->value;
    direction_t direction = roach_payload->recv_message->direction;

    int new_x = roach_payload->roaches[id].x;
    int new_y = roach_payload->roaches[id].y;
    new_position(&new_x, &new_y, direction);

    chtype ch = mvinch(new_x, new_y) & A_CHARTEXT;

    if (ch != ' ' && ch != '.')
    {
        zmq_send(roach_payload->responder, &success, sizeof(int), 0);
        return;
    }

    // Erase the roach from the screen
    window_erase(roach_payload->game_window, roach_payload->roaches[id].x, roach_payload->roaches[id].y);

    // Update the roach position
    roach_payload->roaches[id].x = new_x;
    roach_payload->roaches[id].y = new_y;

    // Draw the roach in the new position
    window_draw(roach_payload->game_window, roach_payload->roaches[id].x, roach_payload->roaches[id].y, (roach_payload->roaches[id].ch + 48) | A_BOLD);

    // Reply indicating success moving the roach
    zmq_send(roach_payload->responder, &success, sizeof(int), 0);
}

void process_roach_disconnect(roach_mover *roach_payload)
{
    int id = roach_payload->recv_message->value;
    window_erase(roach_payload->game_window, roach_payload->roaches[id].x, roach_payload->roaches[id].y);
    zmq_send(roach_payload->responder, &success, sizeof(int), 0);
    (*(roach_payload->num_roaches))--;
    (*(roach_payload->slot_roaches))++;
}