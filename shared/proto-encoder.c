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

// Convert WindowDataProto to WindowData
void proto_window_matrix_to_window_matrix(WindowMatrixProto *proto, window_matrix *matrix)
{
    matrix->width = proto->width;
    matrix->height = proto->height;
    matrix->cells = malloc(sizeof(layer_cell) * matrix->width * matrix->height);

    for (int i = 0; i < matrix->width * matrix->height; i++)
    {
        convert_layer_cell_proto_to_layer_cell(proto->cells[i], &matrix->cells[i]);
    }
}

// Convert WindowData to WindowDataProto
void window_matrix_to_proto_window_matrix(WindowMatrixProto *proto, window_matrix *matrix)
{
    proto->width = matrix->width;
    proto->height = matrix->height;
    proto->cells = malloc(sizeof(LayerCellProto *) * proto->width * proto->height);

    for (int i = 0; i < proto->width * proto->height; i++)
    {
        proto->cells[i] = malloc(sizeof(LayerCellProto));
        layer_cell_proto__init(proto->cells[i]);
        convert_layer_cell_to_layer_cell_proto(proto->cells[i], &matrix->cells[i]);
    }
}

// Convert WindowDataProto to WindowData
void proto_window_data_to_window_data(WindowDataProto *proto, window_data *data)
{
    proto_window_matrix_to_window_matrix(proto->matrix, data->matrix);
}

// Convert WindowData to WindowDataProto
void window_data_to_proto_window_data(WindowDataProto *proto, window_data *data)
{
    proto->matrix = malloc(sizeof(WindowMatrixProto));
    window_matrix_proto__init(proto->matrix);
    window_matrix_to_proto_window_matrix(proto->matrix, data->matrix);
}

// Convert LayerCellProto to LayerCell
void convert_layer_cell_proto_to_layer_cell(LayerCellProto *proto, layer_cell *cell)
{
    cell->top = proto->top;
    cell->capacity = proto->capacity;
    cell->stack = malloc(sizeof(layer_char) * cell->capacity);

    for (int i = 0; i <= cell->top; i++)
    {
        cell->stack[i].ch = proto->stack[i]->ch[0]; // Assuming ch is a string in Proto and a char in C
        cell->stack[i].client_id = proto->stack[i]->client_id;
        cell->stack[i].position_in_array = proto->stack[i]->position_in_array;
    }
}

// Convert LayerCell to LayerCellProto
void convert_layer_cell_to_layer_cell_proto(LayerCellProto *proto, layer_cell *cell)
{
    proto->top = cell->top;
    proto->capacity = cell->capacity;
    proto->stack = malloc(sizeof(LayerCharProto *) * proto->capacity);

    for (int i = 0; i <= proto->top; i++)
    {
        proto->stack[i] = malloc(sizeof(LayerCharProto));
        layer_char_proto__init(proto->stack[i]);
        proto->stack[i]->ch = malloc(sizeof(char));
        proto->stack[i]->ch[0] = cell->stack[i].ch;
        proto->stack[i]->client_id = cell->stack[i].client_id;
        proto->stack[i]->position_in_array = cell->stack[i].position_in_array;
    }
}

// Convert FieldUpdateProto to FieldUpdate
void proto_field_update_to_field_update(FieldUpdateProto *proto, field_update *field_update)
{
    field_update->size_of_updated_cells = proto->size_of_updated_cells;
    field_update->size_of_scores = proto->size_of_scores;
    field_update->updated_cell_indexes = proto->updated_cell_indexes;
    field_update->scores = proto->scores;

    field_update->updated_cells = malloc(sizeof(layer_cell) * field_update->size_of_updated_cells);
    for (int i = 0; i < field_update->size_of_updated_cells; i++)
    {
        convert_layer_cell_proto_to_layer_cell(proto->updated_cells[i], &field_update->updated_cells[i]);
    }
}

// Convert FieldUpdate to FieldUpdateProto
void field_update_to_proto_field_update(FieldUpdateProto *proto, field_update *field_update)
{
    proto->size_of_updated_cells = field_update->size_of_updated_cells;
    proto->size_of_scores = field_update->size_of_scores;
    proto->updated_cell_indexes = field_update->updated_cell_indexes;
    proto->scores = field_update->scores;
    // Handle updated_cells conversion
    proto->updated_cells = malloc(sizeof(LayerCellProto *) * field_update->size_of_updated_cells);
    for (int i = 0; i < field_update->size_of_updated_cells; i++)
    {
        proto->updated_cells[i] = malloc(sizeof(LayerCellProto));
        layer_cell_proto__init(proto->updated_cells[i]);
        convert_layer_cell_to_layer_cell_proto(proto->updated_cells[i], &field_update->updated_cells[i]);
    }
}