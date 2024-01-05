#include "proto-encoder.h"

// Proto Structures:
// Convert DirectionProto to Direction
void proto_direction_to_direction_t(DirectionProto *proto, direction_t *direction)
{
    switch (*proto)
    {
    case DIRECTION_PROTO__UP:
        *direction = UP;
        break;
    case DIRECTION_PROTO__DOWN:
        *direction = DOWN;
        break;
    case DIRECTION_PROTO__LEFT:
        *direction = LEFT;
        break;
    case DIRECTION_PROTO__RIGHT:
        *direction = RIGHT;
        break;
    default:
        break;
    }
}

// Convert Direction DirectionProto to
void direction_t_to_proto_direction(DirectionProto *proto, direction_t *direction)
{
    switch (*direction)
    {
    case UP:
        *proto = DIRECTION_PROTO__UP;
        break;
    case DOWN:
        *proto = DIRECTION_PROTO__DOWN;
        break;
    case LEFT:
        *proto = DIRECTION_PROTO__LEFT;
        break;
    case RIGHT:
        *proto = DIRECTION_PROTO__RIGHT;
        break;
    default:
        break;
    }
}

// Convert MessageTypeProto to MessageType
void proto_message_type_to_message_type(MessageTypeProto *proto, message_type *message_type)
{
    switch (*proto)
    {
    case MESSAGE_TYPE_PROTO__CONNECT:
        *message_type = CONNECT;
        break;
    case MESSAGE_TYPE_PROTO__DISCONNECT:
        *message_type = DISCONNECT;
        break;
    case MESSAGE_TYPE_PROTO__MOVEMENT:
        *message_type = MOVEMENT;
        break;
    default:
        break;
    }
}

// Convert MessageType to MessageTypeProto
void message_type_to_proto_message_type(MessageTypeProto *proto, message_type *message_type)
{
    switch (*message_type)
    {
    case CONNECT:
        *proto = MESSAGE_TYPE_PROTO__CONNECT;
        break;
    case DISCONNECT:
        *proto = MESSAGE_TYPE_PROTO__DISCONNECT;
        break;
    case MOVEMENT:
        *proto = MESSAGE_TYPE_PROTO__MOVEMENT;
        break;
    default:
        break;
    }
}

// Convert ClientTypeProto to ClientType
void proto_client_type_to_client_type(ClientTypeProto *proto, client_type *client_type)
{
    switch (*proto)
    {
    case CLIENT_TYPE_PROTO__LIZARD:
        *client_type = LIZARD;
        break;
    case CLIENT_TYPE_PROTO__ROACH:
        *client_type = ROACH;
        break;
    case CLIENT_TYPE_PROTO__WASP:
        *client_type = WASP;
        break;
    case CLIENT_TYPE_PROTO__DISPLAY_APP:
        *client_type = DISPLAY_APP;
        break;
    case CLIENT_TYPE_PROTO__LIZARD_BODY:
        *client_type = LIZARD_BODY;
        break;
    default:
        break;
    }
}
// Convert ClientType to ClientTypeProto
void client_type_to_proto_client_type(ClientTypeProto *proto, client_type *client_type)
{
    switch (*client_type)
    {
    case LIZARD:
        *proto = CLIENT_TYPE_PROTO__LIZARD;
        break;
    case ROACH:
        *proto = CLIENT_TYPE_PROTO__ROACH;
        break;
    case WASP:
        *proto = CLIENT_TYPE_PROTO__WASP;
        break;
    case DISPLAY_APP:
        *proto = CLIENT_TYPE_PROTO__DISPLAY_APP;
        break;
    case LIZARD_BODY:
        *proto = CLIENT_TYPE_PROTO__LIZARD_BODY;
        break;
    default:
        break;
    }
}

// Convert MessageToServerProto to MessageToServer
void proto_message_to_server_to_message_to_server(MessageToServerProto *proto, message_to_server *message_to_server)
{
    message_to_server->client_id = proto->client_id;
    message_to_server->type = proto->type;
    message_to_server->value = proto->value;
    proto_direction_to_direction_t(&proto->direction, &message_to_server->direction);
    message_to_server->message_accepted = proto->message_accepted;
}

// Convert MessageToServer to MessageToServerProto
void message_to_server_to_proto_message_to_server(MessageToServerProto *proto, message_to_server *message_to_server)
{
    proto->client_id = message_to_server->client_id;
    proto->type = message_to_server->type;
    proto->value = message_to_server->value;
    direction_t_to_proto_direction(&proto->direction, &message_to_server->direction);
    proto->message_accepted = message_to_server->message_accepted;
}

// Convert LizardProto to Lizard
void proto_lizard_to_lizard(LizardProto *proto, lizard *lizard)
{
    lizard->ch = proto->ch;
    lizard->x = proto->x;
    lizard->y = proto->y;
    lizard->score = proto->score;
    proto_direction_to_direction_t(&proto->previous_direction, &lizard->previous_direction);
    lizard->is_winner = proto->is_winner;
}

// Convert Lizard to LizardProto
void lizard_to_proto_lizard(LizardProto *proto, lizard *lizard)
{
    proto->ch = lizard->ch;
    proto->x = lizard->x;
    proto->y = lizard->y;
    proto->score = lizard->score;
    direction_t_to_proto_direction(&proto->previous_direction, &lizard->previous_direction);
    proto->is_winner = lizard->is_winner;
}

// Convert RoachProto to Roach
void proto_roach_to_roach(RoachProto *proto, roach *roach)
{
    roach->ch = proto->ch;
    roach->x = proto->x;
    roach->y = proto->y;
    roach->is_eaten = proto->is_eaten;
    roach->timestamp = proto->timestamp;
}

// Convert Roach to RoachProto
void roach_to_proto_roach(RoachProto *proto, roach *roach)
{
    proto->ch = roach->ch;
    proto->x = roach->x;
    proto->y = roach->y;
    proto->is_eaten = roach->is_eaten;
    proto->timestamp = roach->timestamp;
}

// Convert WaspProto to Wasp
void proto_wasp_to_wasp(WaspProto *proto, wasp *wasp)
{
    wasp->ch = proto->ch;
    wasp->x = proto->x;
    wasp->y = proto->y;
}

// Convert Wasp to WaspProto
void wasp_to_proto_wasp(WaspProto *proto, wasp *wasp)
{
    proto->ch = wasp->ch;
    proto->x = wasp->x;
    proto->y = wasp->y;
}

// Convert FieldUpdateMovementProto to FieldUpdateMovement
void proto_field_update_movement_to_field_update_movement(FieldUpdateMovementProto *proto, field_update_movement *field_update_movement)
{
    field_update_movement->num_roaches = proto->num_roaches;
    field_update_movement->num_lizards = proto->num_lizards;
    proto_message_to_server_to_message_to_server(proto->message, &field_update_movement->message);
    field_update_movement->new_x = proto->new_x;
    field_update_movement->new_y = proto->new_y;
    proto_direction_to_direction_t(&proto->prev_direction, &field_update_movement->prev_direction);
    field_update_movement->is_eaten = proto->is_eaten;
}

// Convert FieldUpdateMovement to FieldUpdateMovementProto
void field_update_movement_to_proto_field_update_movement(FieldUpdateMovementProto *proto, field_update_movement *field_update_movement)
{
    proto->num_roaches = field_update_movement->num_roaches;
    proto->num_lizards = field_update_movement->num_lizards;
    message_to_server_to_proto_message_to_server(proto->message, &field_update_movement->message);
    proto->new_x = field_update_movement->new_x;
    proto->new_y = field_update_movement->new_y;
    direction_t_to_proto_direction(&proto->prev_direction, &field_update_movement->prev_direction);
    proto->is_eaten = field_update_movement->is_eaten;
}

// Convert FieldUpdateConnectProto to FieldUpdateConnect
void proto_field_update_connect_to_field_update_connect(FieldUpdateConnectProto *proto, field_update_connect *field_update_connect)
{
    field_update_connect->client_id = proto->client_id;
    field_update_connect->position_in_array = proto->position_in_array;
    proto_lizard_to_lizard(proto->connected_lizard, &field_update_connect->connected_lizard);
    proto_roach_to_roach(proto->connected_roach, &field_update_connect->connected_roach);
    proto_message_to_server_to_message_to_server(proto->message, &field_update_connect->message);
}

// Convert FieldUpdateConnect to FieldUpdateConnectProto
void field_update_connect_to_proto_field_update_connect(FieldUpdateConnectProto *proto, field_update_connect *field_update_connect)
{
    proto->client_id = field_update_connect->client_id;
    proto->position_in_array = field_update_connect->position_in_array;
    lizard_to_proto_lizard(proto->connected_lizard, &field_update_connect->connected_lizard);
    roach_to_proto_roach(proto->connected_roach, &field_update_connect->connected_roach);
    message_to_server_to_proto_message_to_server(proto->message, &field_update_connect->message);
}

// Convert FieldUpdateDisconnectProto to FieldUpdateDisconnect
void proto_field_update_disconnect_to_field_update_disconnect(FieldUpdateDisconnectProto *proto, field_update_disconnect *field_update_disconnect)
{
    field_update_disconnect->client_id = proto->client_id;
    field_update_disconnect->position_in_array = proto->position_in_array;
    proto_message_to_server_to_message_to_server(proto->message, &field_update_disconnect->message);
}

// Convert FieldUpdateDisconnect to FieldUpdateDisconnectProto
void field_update_disconnect_to_proto_field_update_disconnect(FieldUpdateDisconnectProto *proto, field_update_disconnect *field_update_disconnect)
{
    proto->client_id = field_update_disconnect->client_id;
    proto->position_in_array = field_update_disconnect->position_in_array;
    message_to_server_to_proto_message_to_server(proto->message, &field_update_disconnect->message);
}

// Convert RoachMoverMessageProto to RoachMoverMessage

// Convert LizardMoverMessageProto to LizardMoverMessage

// Convert WaspMoverMessageProto to WaspMoverMessage
