#ifndef WASP_MOVER_H
#define WASP_MOVER_H

#include "util.h"
#include "default_consts.h"
#include "window.h"

typedef struct wasp_mover
{
    message_to_server *recv_message;
    wasp *wasps;
    lizard *lizards;
    void *responder;
    int *num_wasps;
    int *slot_wasps;
    window_data *game_window;

    char should_use_responder;
} wasp_mover;

void new_wasp_mover(wasp_mover **wasp_payload, message_to_server *recv_message, wasp *wasps, int *num_wasps, int *slot_wasps, window_data *game_window);
void process_wasp_message(wasp_mover *wasp_payload);
void process_wasp_connect(wasp_mover *wasp_payload);
int calculate_wasp_movement(wasp_mover *wasp_payload, int *new_x, int *new_y);
void process_wasp_movement(wasp_mover *wasp_payload);
void wasp_move(wasp_mover *wasp_payload, int new_x, int new_y, int wasp_id);
void process_wasp_disconnect(wasp_mover *wasp_payload);
#endif
