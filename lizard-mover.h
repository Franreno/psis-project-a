#ifndef LIZARD_MOVER_H
#define LIZARD_MOVER_H

#include <stdlib.h>
#include <zmq.h>
#include "util.h"
#include "window.h"
#include "remote-char.h"

typedef struct lizard_mover
{
    message_to_server *recv_message;
    lizard *lizards;
    void *responder;
    int *num_lizards;
    int *slot_lizards;
    window_data *game_window;
} lizard_mover;

void new_lizard_mover(lizard_mover **roach_payload, message_to_server *recv_message, lizard *roaches, void *responder, int *num_roaches, int *slot_roaches, window_data *game_window);
void process_lizard_message(lizard_mover *lizard_payload);
void process_lizard_connect(lizard_mover *lizard_payload);
void process_lizard_movement(lizard_mover *lizard_payload);
void process_lizard_disconnect(lizard_mover *lizard_payload);

#endif