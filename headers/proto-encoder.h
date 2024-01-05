#ifndef __PROTO_ENCODER_H__
#define __PROTO_ENCODER_H__

#include "default_consts.h"
#include "server_messages.pb-c.h"
#include "window_data.pb-c.h"
#include "window.h"

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
void proto_window_matrix_to_window_matrix(WindowMatrixProto *proto, window_matrix *matrix);
void window_matrix_to_proto_window_matrix(WindowMatrixProto *proto, window_matrix *matrix);
void proto_window_data_to_window_data(WindowDataProto *proto, window_data *data);
void window_data_to_proto_window_data(WindowDataProto *proto, window_data *data);
void convert_layer_cell_proto_to_layer_cell(LayerCellProto *proto, layer_cell *cell);
void convert_layer_cell_to_layer_cell_proto(LayerCellProto *proto, layer_cell *cell);
void proto_field_update_to_field_update(FieldUpdateProto *proto, field_update *field_update);
void field_update_to_proto_field_update(FieldUpdateProto *proto, field_update *field_update);

#endif // __PROTO_ENCODER_H__