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
    int server_reply = 0;
    int new_roach_id;

    // If there are available slots, add the roach to the array
    if (*(roach_payload->slot_roaches) <= 0)
    {
        server_reply = -1;
        // Reply indicating failure adding the roach due to lack of slots
        if (roach_payload->should_use_responder)
        {
            zmq_send(roach_payload->responder, &server_reply, sizeof(int), 0);
        }

        return;
    }

    // Get the id of the new roach
    new_roach_id = *(roach_payload->num_roaches);

    // Increment the number of roaches and decrement the available slots
    (*(roach_payload->num_roaches))++;
    (*(roach_payload->slot_roaches))--;

    // Initialize the roach in a valid random position
    roach_payload->roaches[new_roach_id].ch = roach_payload->recv_message->value;
    roach_payload->roaches[new_roach_id].x = rand() % (WINDOW_SIZE - 2) + 1; // TODO - CHECK IF POSITION IS VALID
    roach_payload->roaches[new_roach_id].y = rand() % (WINDOW_SIZE - 2) + 1; // TODO - CHECK IF POSITION IS VALID

    // Draw the roach in the random position
    window_draw(roach_payload->game_window, roach_payload->roaches[new_roach_id].x, roach_payload->roaches[new_roach_id].y, (roach_payload->roaches[new_roach_id].ch + '0') | A_BOLD, ROACH, new_roach_id);

    // Reply indicating position of the roach in the array
    if (roach_payload->should_use_responder)
    {
        zmq_send(roach_payload->responder, &new_roach_id, sizeof(int), 0);
    }
}

void process_roach_inject_connect(roach_mover *roach_payload, roach connected_roach, int received_id)
{
    // Inject means it was received from the server, if received
    // from the server, we don't need to reply, and don't need to check for slots

    // Increment the number of roaches and decrement the available slots
    (*(roach_payload->num_roaches))++;
    (*(roach_payload->slot_roaches))--;

    // Initialize the roach in a received position
    roach_payload->roaches[received_id] = connected_roach;

    window_draw(roach_payload->game_window, roach_payload->roaches[received_id].x, roach_payload->roaches[received_id].y, (roach_payload->roaches[received_id].ch + 48) | A_BOLD, ROACH, received_id);
}

void process_roach_movement(roach_mover *roach_payload)
{
    int success = 0;

    // Move the specified roach
    int id = roach_payload->recv_message->value;
    direction_t direction = roach_payload->recv_message->direction;

    int new_x = roach_payload->roaches[id].x;
    int new_y = roach_payload->roaches[id].y;
    new_position(&new_x, &new_y, direction);

    if (roach_payload->roaches[id].is_eaten)
    {
        if (roach_payload->should_use_responder)
            zmq_send(roach_payload->responder, &success, sizeof(int), 0);
        return;
    }

    chtype ch = mvinch(new_x, new_y) & A_CHARTEXT;

    if (ch != ' ' && ch != '.')
    {
        if (roach_payload->should_use_responder)
            zmq_send(roach_payload->responder, &success, sizeof(int), 0);
        return;
    }

    // Erase the roach from the screen
    window_erase(roach_payload->game_window, roach_payload->roaches[id].x, roach_payload->roaches[id].y, (roach_payload->roaches[id].ch + 48) | A_BOLD);

    // Update the roach position
    roach_payload->roaches[id].x = new_x;
    roach_payload->roaches[id].y = new_y;

    // Draw the roach in the new position
    window_draw(roach_payload->game_window, roach_payload->roaches[id].x, roach_payload->roaches[id].y, (roach_payload->roaches[id].ch + 48) | A_BOLD, ROACH, id);

    // Reply indicating success moving the roach
    if (roach_payload->should_use_responder)
        zmq_send(roach_payload->responder, &success, sizeof(int), 0);
}

void process_roach_disconnect(roach_mover *roach_payload)
{
    int success = 0;
    int id = roach_payload->recv_message->value;

    window_erase(roach_payload->game_window, roach_payload->roaches[id].x, roach_payload->roaches[id].y, (roach_payload->roaches[id].ch + 48) | A_BOLD);

    zmq_send(roach_payload->responder, &success, sizeof(int), 0);

    (*(roach_payload->num_roaches))--;
    (*(roach_payload->slot_roaches))++;
}

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
    roach_payload->roaches = malloc(sizeof(roach) * MAX_ROACHES_ALLOWED);
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
