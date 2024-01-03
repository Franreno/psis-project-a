#ifndef UTIL_H
#define UTIL_H

#include "default_consts.h"
#include "server_messages.pb-c.h"

extern void new_position(int *x, int *y, direction_t direction);
extern void tail_position_calc(int *x, int *y, direction_t direction, char *overflow);
extern Direction convert_direction_to_protobuf_direction_enum(direction_t direction);
extern direction_t convert_protobuf_direction_enum_to_direction_t(Direction direction);
extern void convert_proto_message_to_server_to_message_to_server(message_to_server *new_msg, MessageToServer *msg);
extern void convert_message_to_server_to_proto_message_to_server(MessageToServer *new_msg, message_to_server *msg);
#endif
