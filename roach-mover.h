#ifndef ROACH_MOVER_H
#define ROACH_MOVER_H

#include <stdlib.h>
#include <zmq.h>
#include "util.h"
#include "remote-char.h"
#include "window.h"

typedef struct roach_mover
{
    message_to_server *recv_message;
    roach *roaches;
    void *responder;
    int *num_roaches;
    int *slot_roaches;
    window_data *game_window;

    char should_use_responder;
} roach_mover;

void new_roach_mover(roach_mover **roach_payload, message_to_server *recv_message, roach *roaches, void *responder, int *num_roaches, int *slot_roaches, window_data *game_window);
void process_roach_message(roach_mover *roach_payload);
void process_roach_connect(roach_mover *roach_payload);
void process_roach_inject_connect(roach_mover *roach_payload, roach connected_roach, int received_id);
void process_roach_movement(roach_mover *roach_payload);
void process_roach_disconnect(roach_mover *roach_payload);
void serialize_roach_mover(roach_mover *roach_payload, char **buffer, size_t *buffer_size);
void deserialize_roach_mover(roach_mover *roach_payload, char *buffer);

#endif
