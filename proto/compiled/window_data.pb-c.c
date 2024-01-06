/* Generated by the protocol buffer compiler.  DO NOT EDIT! */
/* Generated from: window_data.proto */

/* Do not generate deprecated warnings for self */
#ifndef PROTOBUF_C__NO_DEPRECATED
#define PROTOBUF_C__NO_DEPRECATED
#endif

#include "window_data.pb-c.h"
void   layer_char_proto__init
                     (LayerCharProto         *message)
{
  static const LayerCharProto init_value = LAYER_CHAR_PROTO__INIT;
  *message = init_value;
}
size_t layer_char_proto__get_packed_size
                     (const LayerCharProto *message)
{
  assert(message->base.descriptor == &layer_char_proto__descriptor);
  return protobuf_c_message_get_packed_size ((const ProtobufCMessage*)(message));
}
size_t layer_char_proto__pack
                     (const LayerCharProto *message,
                      uint8_t       *out)
{
  assert(message->base.descriptor == &layer_char_proto__descriptor);
  return protobuf_c_message_pack ((const ProtobufCMessage*)message, out);
}
size_t layer_char_proto__pack_to_buffer
                     (const LayerCharProto *message,
                      ProtobufCBuffer *buffer)
{
  assert(message->base.descriptor == &layer_char_proto__descriptor);
  return protobuf_c_message_pack_to_buffer ((const ProtobufCMessage*)message, buffer);
}
LayerCharProto *
       layer_char_proto__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data)
{
  return (LayerCharProto *)
     protobuf_c_message_unpack (&layer_char_proto__descriptor,
                                allocator, len, data);
}
void   layer_char_proto__free_unpacked
                     (LayerCharProto *message,
                      ProtobufCAllocator *allocator)
{
  if(!message)
    return;
  assert(message->base.descriptor == &layer_char_proto__descriptor);
  protobuf_c_message_free_unpacked ((ProtobufCMessage*)message, allocator);
}
void   layer_cell_proto__init
                     (LayerCellProto         *message)
{
  static const LayerCellProto init_value = LAYER_CELL_PROTO__INIT;
  *message = init_value;
}
size_t layer_cell_proto__get_packed_size
                     (const LayerCellProto *message)
{
  assert(message->base.descriptor == &layer_cell_proto__descriptor);
  return protobuf_c_message_get_packed_size ((const ProtobufCMessage*)(message));
}
size_t layer_cell_proto__pack
                     (const LayerCellProto *message,
                      uint8_t       *out)
{
  assert(message->base.descriptor == &layer_cell_proto__descriptor);
  return protobuf_c_message_pack ((const ProtobufCMessage*)message, out);
}
size_t layer_cell_proto__pack_to_buffer
                     (const LayerCellProto *message,
                      ProtobufCBuffer *buffer)
{
  assert(message->base.descriptor == &layer_cell_proto__descriptor);
  return protobuf_c_message_pack_to_buffer ((const ProtobufCMessage*)message, buffer);
}
LayerCellProto *
       layer_cell_proto__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data)
{
  return (LayerCellProto *)
     protobuf_c_message_unpack (&layer_cell_proto__descriptor,
                                allocator, len, data);
}
void   layer_cell_proto__free_unpacked
                     (LayerCellProto *message,
                      ProtobufCAllocator *allocator)
{
  if(!message)
    return;
  assert(message->base.descriptor == &layer_cell_proto__descriptor);
  protobuf_c_message_free_unpacked ((ProtobufCMessage*)message, allocator);
}
void   window_matrix_proto__init
                     (WindowMatrixProto         *message)
{
  static const WindowMatrixProto init_value = WINDOW_MATRIX_PROTO__INIT;
  *message = init_value;
}
size_t window_matrix_proto__get_packed_size
                     (const WindowMatrixProto *message)
{
  assert(message->base.descriptor == &window_matrix_proto__descriptor);
  return protobuf_c_message_get_packed_size ((const ProtobufCMessage*)(message));
}
size_t window_matrix_proto__pack
                     (const WindowMatrixProto *message,
                      uint8_t       *out)
{
  assert(message->base.descriptor == &window_matrix_proto__descriptor);
  return protobuf_c_message_pack ((const ProtobufCMessage*)message, out);
}
size_t window_matrix_proto__pack_to_buffer
                     (const WindowMatrixProto *message,
                      ProtobufCBuffer *buffer)
{
  assert(message->base.descriptor == &window_matrix_proto__descriptor);
  return protobuf_c_message_pack_to_buffer ((const ProtobufCMessage*)message, buffer);
}
WindowMatrixProto *
       window_matrix_proto__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data)
{
  return (WindowMatrixProto *)
     protobuf_c_message_unpack (&window_matrix_proto__descriptor,
                                allocator, len, data);
}
void   window_matrix_proto__free_unpacked
                     (WindowMatrixProto *message,
                      ProtobufCAllocator *allocator)
{
  if(!message)
    return;
  assert(message->base.descriptor == &window_matrix_proto__descriptor);
  protobuf_c_message_free_unpacked ((ProtobufCMessage*)message, allocator);
}
void   window_data_proto__init
                     (WindowDataProto         *message)
{
  static const WindowDataProto init_value = WINDOW_DATA_PROTO__INIT;
  *message = init_value;
}
size_t window_data_proto__get_packed_size
                     (const WindowDataProto *message)
{
  assert(message->base.descriptor == &window_data_proto__descriptor);
  return protobuf_c_message_get_packed_size ((const ProtobufCMessage*)(message));
}
size_t window_data_proto__pack
                     (const WindowDataProto *message,
                      uint8_t       *out)
{
  assert(message->base.descriptor == &window_data_proto__descriptor);
  return protobuf_c_message_pack ((const ProtobufCMessage*)message, out);
}
size_t window_data_proto__pack_to_buffer
                     (const WindowDataProto *message,
                      ProtobufCBuffer *buffer)
{
  assert(message->base.descriptor == &window_data_proto__descriptor);
  return protobuf_c_message_pack_to_buffer ((const ProtobufCMessage*)message, buffer);
}
WindowDataProto *
       window_data_proto__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data)
{
  return (WindowDataProto *)
     protobuf_c_message_unpack (&window_data_proto__descriptor,
                                allocator, len, data);
}
void   window_data_proto__free_unpacked
                     (WindowDataProto *message,
                      ProtobufCAllocator *allocator)
{
  if(!message)
    return;
  assert(message->base.descriptor == &window_data_proto__descriptor);
  protobuf_c_message_free_unpacked ((ProtobufCMessage*)message, allocator);
}
void   scores_update_proto__init
                     (ScoresUpdateProto         *message)
{
  static const ScoresUpdateProto init_value = SCORES_UPDATE_PROTO__INIT;
  *message = init_value;
}
size_t scores_update_proto__get_packed_size
                     (const ScoresUpdateProto *message)
{
  assert(message->base.descriptor == &scores_update_proto__descriptor);
  return protobuf_c_message_get_packed_size ((const ProtobufCMessage*)(message));
}
size_t scores_update_proto__pack
                     (const ScoresUpdateProto *message,
                      uint8_t       *out)
{
  assert(message->base.descriptor == &scores_update_proto__descriptor);
  return protobuf_c_message_pack ((const ProtobufCMessage*)message, out);
}
size_t scores_update_proto__pack_to_buffer
                     (const ScoresUpdateProto *message,
                      ProtobufCBuffer *buffer)
{
  assert(message->base.descriptor == &scores_update_proto__descriptor);
  return protobuf_c_message_pack_to_buffer ((const ProtobufCMessage*)message, buffer);
}
ScoresUpdateProto *
       scores_update_proto__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data)
{
  return (ScoresUpdateProto *)
     protobuf_c_message_unpack (&scores_update_proto__descriptor,
                                allocator, len, data);
}
void   scores_update_proto__free_unpacked
                     (ScoresUpdateProto *message,
                      ProtobufCAllocator *allocator)
{
  if(!message)
    return;
  assert(message->base.descriptor == &scores_update_proto__descriptor);
  protobuf_c_message_free_unpacked ((ProtobufCMessage*)message, allocator);
}
void   field_update_proto__init
                     (FieldUpdateProto         *message)
{
  static const FieldUpdateProto init_value = FIELD_UPDATE_PROTO__INIT;
  *message = init_value;
}
size_t field_update_proto__get_packed_size
                     (const FieldUpdateProto *message)
{
  assert(message->base.descriptor == &field_update_proto__descriptor);
  return protobuf_c_message_get_packed_size ((const ProtobufCMessage*)(message));
}
size_t field_update_proto__pack
                     (const FieldUpdateProto *message,
                      uint8_t       *out)
{
  assert(message->base.descriptor == &field_update_proto__descriptor);
  return protobuf_c_message_pack ((const ProtobufCMessage*)message, out);
}
size_t field_update_proto__pack_to_buffer
                     (const FieldUpdateProto *message,
                      ProtobufCBuffer *buffer)
{
  assert(message->base.descriptor == &field_update_proto__descriptor);
  return protobuf_c_message_pack_to_buffer ((const ProtobufCMessage*)message, buffer);
}
FieldUpdateProto *
       field_update_proto__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data)
{
  return (FieldUpdateProto *)
     protobuf_c_message_unpack (&field_update_proto__descriptor,
                                allocator, len, data);
}
void   field_update_proto__free_unpacked
                     (FieldUpdateProto *message,
                      ProtobufCAllocator *allocator)
{
  if(!message)
    return;
  assert(message->base.descriptor == &field_update_proto__descriptor);
  protobuf_c_message_free_unpacked ((ProtobufCMessage*)message, allocator);
}
static const ProtobufCFieldDescriptor layer_char_proto__field_descriptors[3] =
{
  {
    "ch",
    1,
    PROTOBUF_C_LABEL_NONE,
    PROTOBUF_C_TYPE_INT32,
    0,   /* quantifier_offset */
    offsetof(LayerCharProto, ch),
    NULL,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "client_id",
    2,
    PROTOBUF_C_LABEL_NONE,
    PROTOBUF_C_TYPE_INT32,
    0,   /* quantifier_offset */
    offsetof(LayerCharProto, client_id),
    NULL,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "position_in_array",
    3,
    PROTOBUF_C_LABEL_NONE,
    PROTOBUF_C_TYPE_INT32,
    0,   /* quantifier_offset */
    offsetof(LayerCharProto, position_in_array),
    NULL,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
};
static const unsigned layer_char_proto__field_indices_by_name[] = {
  0,   /* field[0] = ch */
  1,   /* field[1] = client_id */
  2,   /* field[2] = position_in_array */
};
static const ProtobufCIntRange layer_char_proto__number_ranges[1 + 1] =
{
  { 1, 0 },
  { 0, 3 }
};
const ProtobufCMessageDescriptor layer_char_proto__descriptor =
{
  PROTOBUF_C__MESSAGE_DESCRIPTOR_MAGIC,
  "LayerCharProto",
  "LayerCharProto",
  "LayerCharProto",
  "",
  sizeof(LayerCharProto),
  3,
  layer_char_proto__field_descriptors,
  layer_char_proto__field_indices_by_name,
  1,  layer_char_proto__number_ranges,
  (ProtobufCMessageInit) layer_char_proto__init,
  NULL,NULL,NULL    /* reserved[123] */
};
static const ProtobufCFieldDescriptor layer_cell_proto__field_descriptors[3] =
{
  {
    "stack",
    1,
    PROTOBUF_C_LABEL_REPEATED,
    PROTOBUF_C_TYPE_MESSAGE,
    offsetof(LayerCellProto, n_stack),
    offsetof(LayerCellProto, stack),
    &layer_char_proto__descriptor,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "top",
    2,
    PROTOBUF_C_LABEL_NONE,
    PROTOBUF_C_TYPE_INT32,
    0,   /* quantifier_offset */
    offsetof(LayerCellProto, top),
    NULL,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "capacity",
    3,
    PROTOBUF_C_LABEL_NONE,
    PROTOBUF_C_TYPE_INT32,
    0,   /* quantifier_offset */
    offsetof(LayerCellProto, capacity),
    NULL,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
};
static const unsigned layer_cell_proto__field_indices_by_name[] = {
  2,   /* field[2] = capacity */
  0,   /* field[0] = stack */
  1,   /* field[1] = top */
};
static const ProtobufCIntRange layer_cell_proto__number_ranges[1 + 1] =
{
  { 1, 0 },
  { 0, 3 }
};
const ProtobufCMessageDescriptor layer_cell_proto__descriptor =
{
  PROTOBUF_C__MESSAGE_DESCRIPTOR_MAGIC,
  "LayerCellProto",
  "LayerCellProto",
  "LayerCellProto",
  "",
  sizeof(LayerCellProto),
  3,
  layer_cell_proto__field_descriptors,
  layer_cell_proto__field_indices_by_name,
  1,  layer_cell_proto__number_ranges,
  (ProtobufCMessageInit) layer_cell_proto__init,
  NULL,NULL,NULL    /* reserved[123] */
};
static const ProtobufCFieldDescriptor window_matrix_proto__field_descriptors[3] =
{
  {
    "cells",
    1,
    PROTOBUF_C_LABEL_REPEATED,
    PROTOBUF_C_TYPE_MESSAGE,
    offsetof(WindowMatrixProto, n_cells),
    offsetof(WindowMatrixProto, cells),
    &layer_cell_proto__descriptor,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "width",
    2,
    PROTOBUF_C_LABEL_NONE,
    PROTOBUF_C_TYPE_INT32,
    0,   /* quantifier_offset */
    offsetof(WindowMatrixProto, width),
    NULL,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "height",
    3,
    PROTOBUF_C_LABEL_NONE,
    PROTOBUF_C_TYPE_INT32,
    0,   /* quantifier_offset */
    offsetof(WindowMatrixProto, height),
    NULL,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
};
static const unsigned window_matrix_proto__field_indices_by_name[] = {
  0,   /* field[0] = cells */
  2,   /* field[2] = height */
  1,   /* field[1] = width */
};
static const ProtobufCIntRange window_matrix_proto__number_ranges[1 + 1] =
{
  { 1, 0 },
  { 0, 3 }
};
const ProtobufCMessageDescriptor window_matrix_proto__descriptor =
{
  PROTOBUF_C__MESSAGE_DESCRIPTOR_MAGIC,
  "WindowMatrixProto",
  "WindowMatrixProto",
  "WindowMatrixProto",
  "",
  sizeof(WindowMatrixProto),
  3,
  window_matrix_proto__field_descriptors,
  window_matrix_proto__field_indices_by_name,
  1,  window_matrix_proto__number_ranges,
  (ProtobufCMessageInit) window_matrix_proto__init,
  NULL,NULL,NULL    /* reserved[123] */
};
static const ProtobufCFieldDescriptor window_data_proto__field_descriptors[1] =
{
  {
    "matrix",
    1,
    PROTOBUF_C_LABEL_NONE,
    PROTOBUF_C_TYPE_MESSAGE,
    0,   /* quantifier_offset */
    offsetof(WindowDataProto, matrix),
    &window_matrix_proto__descriptor,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
};
static const unsigned window_data_proto__field_indices_by_name[] = {
  0,   /* field[0] = matrix */
};
static const ProtobufCIntRange window_data_proto__number_ranges[1 + 1] =
{
  { 1, 0 },
  { 0, 1 }
};
const ProtobufCMessageDescriptor window_data_proto__descriptor =
{
  PROTOBUF_C__MESSAGE_DESCRIPTOR_MAGIC,
  "WindowDataProto",
  "WindowDataProto",
  "WindowDataProto",
  "",
  sizeof(WindowDataProto),
  1,
  window_data_proto__field_descriptors,
  window_data_proto__field_indices_by_name,
  1,  window_data_proto__number_ranges,
  (ProtobufCMessageInit) window_data_proto__init,
  NULL,NULL,NULL    /* reserved[123] */
};
static const ProtobufCFieldDescriptor scores_update_proto__field_descriptors[2] =
{
  {
    "score",
    1,
    PROTOBUF_C_LABEL_NONE,
    PROTOBUF_C_TYPE_INT32,
    0,   /* quantifier_offset */
    offsetof(ScoresUpdateProto, score),
    NULL,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "ch",
    2,
    PROTOBUF_C_LABEL_NONE,
    PROTOBUF_C_TYPE_INT32,
    0,   /* quantifier_offset */
    offsetof(ScoresUpdateProto, ch),
    NULL,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
};
static const unsigned scores_update_proto__field_indices_by_name[] = {
  1,   /* field[1] = ch */
  0,   /* field[0] = score */
};
static const ProtobufCIntRange scores_update_proto__number_ranges[1 + 1] =
{
  { 1, 0 },
  { 0, 2 }
};
const ProtobufCMessageDescriptor scores_update_proto__descriptor =
{
  PROTOBUF_C__MESSAGE_DESCRIPTOR_MAGIC,
  "ScoresUpdateProto",
  "ScoresUpdateProto",
  "ScoresUpdateProto",
  "",
  sizeof(ScoresUpdateProto),
  2,
  scores_update_proto__field_descriptors,
  scores_update_proto__field_indices_by_name,
  1,  scores_update_proto__number_ranges,
  (ProtobufCMessageInit) scores_update_proto__init,
  NULL,NULL,NULL    /* reserved[123] */
};
static const ProtobufCFieldDescriptor field_update_proto__field_descriptors[5] =
{
  {
    "updated_cells",
    1,
    PROTOBUF_C_LABEL_REPEATED,
    PROTOBUF_C_TYPE_MESSAGE,
    offsetof(FieldUpdateProto, n_updated_cells),
    offsetof(FieldUpdateProto, updated_cells),
    &layer_cell_proto__descriptor,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "updated_cell_indexes",
    2,
    PROTOBUF_C_LABEL_REPEATED,
    PROTOBUF_C_TYPE_INT32,
    offsetof(FieldUpdateProto, n_updated_cell_indexes),
    offsetof(FieldUpdateProto, updated_cell_indexes),
    NULL,
    NULL,
    0 | PROTOBUF_C_FIELD_FLAG_PACKED,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "scores",
    3,
    PROTOBUF_C_LABEL_REPEATED,
    PROTOBUF_C_TYPE_MESSAGE,
    offsetof(FieldUpdateProto, n_scores),
    offsetof(FieldUpdateProto, scores),
    &scores_update_proto__descriptor,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "size_of_updated_cells",
    4,
    PROTOBUF_C_LABEL_NONE,
    PROTOBUF_C_TYPE_INT32,
    0,   /* quantifier_offset */
    offsetof(FieldUpdateProto, size_of_updated_cells),
    NULL,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "size_of_scores",
    5,
    PROTOBUF_C_LABEL_NONE,
    PROTOBUF_C_TYPE_INT32,
    0,   /* quantifier_offset */
    offsetof(FieldUpdateProto, size_of_scores),
    NULL,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
};
static const unsigned field_update_proto__field_indices_by_name[] = {
  2,   /* field[2] = scores */
  4,   /* field[4] = size_of_scores */
  3,   /* field[3] = size_of_updated_cells */
  1,   /* field[1] = updated_cell_indexes */
  0,   /* field[0] = updated_cells */
};
static const ProtobufCIntRange field_update_proto__number_ranges[1 + 1] =
{
  { 1, 0 },
  { 0, 5 }
};
const ProtobufCMessageDescriptor field_update_proto__descriptor =
{
  PROTOBUF_C__MESSAGE_DESCRIPTOR_MAGIC,
  "FieldUpdateProto",
  "FieldUpdateProto",
  "FieldUpdateProto",
  "",
  sizeof(FieldUpdateProto),
  5,
  field_update_proto__field_descriptors,
  field_update_proto__field_indices_by_name,
  1,  field_update_proto__number_ranges,
  (ProtobufCMessageInit) field_update_proto__init,
  NULL,NULL,NULL    /* reserved[123] */
};
