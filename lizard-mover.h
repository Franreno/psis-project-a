#ifndef LIZARD_MOVER_H
#define LIZARD_MOVER_H

#include "util.h"

typedef struct lizard_mover
{
    message_to_server *recv_message;
    lizard *lizards;
    roach *roaches;
    void *responder;
    int *num_lizards;
    int *slot_lizards;
    window_data *game_window;
    roach **eaten_roaches;
    int *amount_eaten_roaches;
    char should_use_responder;
} lizard_mover;

void new_lizard_mover(lizard_mover **lizard_payload, message_to_server *recv_message, lizard *lizards, void *responder, int *num_lizards, int *slot_lizards, window_data *game_window);
void process_lizard_connect(lizard_mover *lizard_payload);
void process_lizard_inject_connect(lizard_mover *lizard_payload, lizard connected_lizard, int received_id);
int calculate_lizard_movement(lizard_mover *lizard_payload, int *new_x, int *new_y);
void draw_lizard_tail(lizard_mover *lizard_payload, int lizard_id, direction_t tail_direction);
void erase_lizard_tail(lizard_mover *lizard_payload, int lizard_id, direction_t tail_direction);
void process_lizard_movement(lizard_mover *lizard_payload);
void lizard_draw(lizard_mover *lizard_payload, int lizard_id);
void lizard_erase(lizard_mover *lizard_payload, int lizard_id);
void lizard_move(lizard_mover *lizard_payload, int lizard_id, int new_x, int new_y);
void process_lizard_disconnect(lizard_mover *lizard_payload);
void process_lizard_message(lizard_mover *lizard_payload);
void serialize_lizard_mover(lizard_mover *lizard_payload, char **buffer, size_t *buffer_size);
void deserialize_lizard_mover(lizard_mover *lizard_payload, char *buffer);
void move_lizard_on_screen(lizard_mover *lizard_payload, int *new_x, int *new_y, int lizard_id);

#endif
