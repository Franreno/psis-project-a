#ifndef __PROTO_ENCODER_H__
#define __PROTO_ENCODER_H__

#include "default_consts.h"
#include "server_messages.pb-c.h"
#include "window_data.pb-c.h"

void proto_direction_to_direction_t(DirectionProto *proto, direction_t *direction);
void direction_t_to_proto_direction(DirectionProto *proto, direction_t *direction);
void proto_message_type_to_message_type(MessageTypeProto *proto, message_type *message_type);
void message_type_to_proto_message_type(MessageTypeProto *proto, message_type *message_type);
void proto_client_type_to_client_type(ClientTypeProto *proto, client_type *client_type);
void client_type_to_proto_client_type(ClientTypeProto *proto, client_type *client_type);
void proto_message_to_server_to_message_to_server(MessageToServerProto *proto, message_to_server *message_to_server);
void message_to_server_to_proto_message_to_server(MessageToServerProto *proto, message_to_server *message_to_server);
void proto_lizard_to_lizard(LizardProto *proto, lizard *lizard);
void lizard_to_proto_lizard(LizardProto *proto, lizard *lizard);
void proto_roach_to_roach(RoachProto *proto, roach *roach);
void roach_to_proto_roach(RoachProto *proto, roach *roach);
void proto_wasp_to_wasp(WaspProto *proto, wasp *wasp);
void wasp_to_proto_wasp(WaspProto *proto, wasp *wasp);
void proto_field_update_movement_to_field_update_movement(FieldUpdateMovementProto *proto, field_update_movement *field_update_movement);
void field_update_movement_to_proto_field_update_movement(FieldUpdateMovementProto *proto, field_update_movement *field_update_movement);
void proto_field_update_connect_to_field_update_connect(FieldUpdateConnectProto *proto, field_update_connect *field_update_connect);
void field_update_connect_to_proto_field_update_connect(FieldUpdateConnectProto *proto, field_update_connect *field_update_connect);
void proto_field_update_disconnect_to_field_update_disconnect(FieldUpdateDisconnectProto *proto, field_update_disconnect *field_update_disconnect);
void field_update_disconnect_to_proto_field_update_disconnect(FieldUpdateDisconnectProto *proto, field_update_disconnect *field_update_disconnect);

#endif // __PROTO_ENCODER_H__