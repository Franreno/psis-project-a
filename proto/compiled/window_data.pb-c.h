/* Generated by the protocol buffer compiler.  DO NOT EDIT! */
/* Generated from: window_data.proto */

#ifndef PROTOBUF_C_window_5fdata_2eproto__INCLUDED
#define PROTOBUF_C_window_5fdata_2eproto__INCLUDED

#include <protobuf-c/protobuf-c.h>

PROTOBUF_C__BEGIN_DECLS

#if PROTOBUF_C_VERSION_NUMBER < 1003000
# error This file was generated by a newer version of protoc-c which is incompatible with your libprotobuf-c headers. Please update your headers.
#elif 1003003 < PROTOBUF_C_MIN_COMPILER_VERSION
# error This file was generated by an older version of protoc-c which is incompatible with your libprotobuf-c headers. Please regenerate this file with a newer version of protoc-c.
#endif


typedef struct _LayerCharProto LayerCharProto;
typedef struct _LayerCellProto LayerCellProto;
typedef struct _WindowMatrixProto WindowMatrixProto;
typedef struct _WindowDataProto WindowDataProto;
typedef struct _ScoresUpdateProto ScoresUpdateProto;
typedef struct _FieldUpdateProto FieldUpdateProto;


/* --- enums --- */


/* --- messages --- */

/*
 * Message for layer_char
 */
struct  _LayerCharProto
{
  ProtobufCMessage base;
  /*
   * Assuming char is represented as a single-character string
   */
  int32_t ch;
  int32_t client_id;
  int32_t position_in_array;
};
#define LAYER_CHAR_PROTO__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&layer_char_proto__descriptor) \
    , 0, 0, 0 }


/*
 * Message for layer_cell
 */
struct  _LayerCellProto
{
  ProtobufCMessage base;
  /*
   * A repeated field to represent the stack of layer_char
   */
  size_t n_stack;
  LayerCharProto **stack;
  /*
   * Index of the top of the stack
   */
  int32_t top;
  /*
   * Capacity of the stack
   */
  int32_t capacity;
};
#define LAYER_CELL_PROTO__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&layer_cell_proto__descriptor) \
    , 0,NULL, 0, 0 }


/*
 * Message for window_matrix
 */
struct  _WindowMatrixProto
{
  ProtobufCMessage base;
  /*
   * A repeated field to represent 1D array matrix of layer_cell
   */
  size_t n_cells;
  LayerCellProto **cells;
  /*
   * Number of rows of the matrix
   */
  int32_t width;
  /*
   * Number of columns of the matrix
   */
  int32_t height;
};
#define WINDOW_MATRIX_PROTO__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&window_matrix_proto__descriptor) \
    , 0,NULL, 0, 0 }


/*
 * Message for window_data
 */
struct  _WindowDataProto
{
  ProtobufCMessage base;
  /*
   * Nested message for window_matrix
   */
  WindowMatrixProto *matrix;
};
#define WINDOW_DATA_PROTO__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&window_data_proto__descriptor) \
    , NULL }


struct  _ScoresUpdateProto
{
  ProtobufCMessage base;
  int32_t score;
  int32_t ch;
};
#define SCORES_UPDATE_PROTO__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&scores_update_proto__descriptor) \
    , 0, 0 }


struct  _FieldUpdateProto
{
  ProtobufCMessage base;
  size_t n_updated_cells;
  LayerCellProto **updated_cells;
  size_t n_updated_cell_indexes;
  int32_t *updated_cell_indexes;
  size_t n_scores;
  ScoresUpdateProto **scores;
  int32_t size_of_updated_cells;
  int32_t size_of_scores;
};
#define FIELD_UPDATE_PROTO__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&field_update_proto__descriptor) \
    , 0,NULL, 0,NULL, 0,NULL, 0, 0 }


/* LayerCharProto methods */
void   layer_char_proto__init
                     (LayerCharProto         *message);
size_t layer_char_proto__get_packed_size
                     (const LayerCharProto   *message);
size_t layer_char_proto__pack
                     (const LayerCharProto   *message,
                      uint8_t             *out);
size_t layer_char_proto__pack_to_buffer
                     (const LayerCharProto   *message,
                      ProtobufCBuffer     *buffer);
LayerCharProto *
       layer_char_proto__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   layer_char_proto__free_unpacked
                     (LayerCharProto *message,
                      ProtobufCAllocator *allocator);
/* LayerCellProto methods */
void   layer_cell_proto__init
                     (LayerCellProto         *message);
size_t layer_cell_proto__get_packed_size
                     (const LayerCellProto   *message);
size_t layer_cell_proto__pack
                     (const LayerCellProto   *message,
                      uint8_t             *out);
size_t layer_cell_proto__pack_to_buffer
                     (const LayerCellProto   *message,
                      ProtobufCBuffer     *buffer);
LayerCellProto *
       layer_cell_proto__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   layer_cell_proto__free_unpacked
                     (LayerCellProto *message,
                      ProtobufCAllocator *allocator);
/* WindowMatrixProto methods */
void   window_matrix_proto__init
                     (WindowMatrixProto         *message);
size_t window_matrix_proto__get_packed_size
                     (const WindowMatrixProto   *message);
size_t window_matrix_proto__pack
                     (const WindowMatrixProto   *message,
                      uint8_t             *out);
size_t window_matrix_proto__pack_to_buffer
                     (const WindowMatrixProto   *message,
                      ProtobufCBuffer     *buffer);
WindowMatrixProto *
       window_matrix_proto__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   window_matrix_proto__free_unpacked
                     (WindowMatrixProto *message,
                      ProtobufCAllocator *allocator);
/* WindowDataProto methods */
void   window_data_proto__init
                     (WindowDataProto         *message);
size_t window_data_proto__get_packed_size
                     (const WindowDataProto   *message);
size_t window_data_proto__pack
                     (const WindowDataProto   *message,
                      uint8_t             *out);
size_t window_data_proto__pack_to_buffer
                     (const WindowDataProto   *message,
                      ProtobufCBuffer     *buffer);
WindowDataProto *
       window_data_proto__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   window_data_proto__free_unpacked
                     (WindowDataProto *message,
                      ProtobufCAllocator *allocator);
/* ScoresUpdateProto methods */
void   scores_update_proto__init
                     (ScoresUpdateProto         *message);
size_t scores_update_proto__get_packed_size
                     (const ScoresUpdateProto   *message);
size_t scores_update_proto__pack
                     (const ScoresUpdateProto   *message,
                      uint8_t             *out);
size_t scores_update_proto__pack_to_buffer
                     (const ScoresUpdateProto   *message,
                      ProtobufCBuffer     *buffer);
ScoresUpdateProto *
       scores_update_proto__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   scores_update_proto__free_unpacked
                     (ScoresUpdateProto *message,
                      ProtobufCAllocator *allocator);
/* FieldUpdateProto methods */
void   field_update_proto__init
                     (FieldUpdateProto         *message);
size_t field_update_proto__get_packed_size
                     (const FieldUpdateProto   *message);
size_t field_update_proto__pack
                     (const FieldUpdateProto   *message,
                      uint8_t             *out);
size_t field_update_proto__pack_to_buffer
                     (const FieldUpdateProto   *message,
                      ProtobufCBuffer     *buffer);
FieldUpdateProto *
       field_update_proto__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   field_update_proto__free_unpacked
                     (FieldUpdateProto *message,
                      ProtobufCAllocator *allocator);
/* --- per-message closures --- */

typedef void (*LayerCharProto_Closure)
                 (const LayerCharProto *message,
                  void *closure_data);
typedef void (*LayerCellProto_Closure)
                 (const LayerCellProto *message,
                  void *closure_data);
typedef void (*WindowMatrixProto_Closure)
                 (const WindowMatrixProto *message,
                  void *closure_data);
typedef void (*WindowDataProto_Closure)
                 (const WindowDataProto *message,
                  void *closure_data);
typedef void (*ScoresUpdateProto_Closure)
                 (const ScoresUpdateProto *message,
                  void *closure_data);
typedef void (*FieldUpdateProto_Closure)
                 (const FieldUpdateProto *message,
                  void *closure_data);

/* --- services --- */


/* --- descriptors --- */

extern const ProtobufCMessageDescriptor layer_char_proto__descriptor;
extern const ProtobufCMessageDescriptor layer_cell_proto__descriptor;
extern const ProtobufCMessageDescriptor window_matrix_proto__descriptor;
extern const ProtobufCMessageDescriptor window_data_proto__descriptor;
extern const ProtobufCMessageDescriptor scores_update_proto__descriptor;
extern const ProtobufCMessageDescriptor field_update_proto__descriptor;

PROTOBUF_C__END_DECLS


#endif  /* PROTOBUF_C_window_5fdata_2eproto__INCLUDED */
