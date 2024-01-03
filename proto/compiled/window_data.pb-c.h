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


typedef struct _LayerChar LayerChar;
typedef struct _LayerCell LayerCell;
typedef struct _WindowMatrix WindowMatrix;
typedef struct _WindowData WindowData;


/* --- enums --- */


/* --- messages --- */

/*
 * Message for layer_char
 */
struct  _LayerChar
{
  ProtobufCMessage base;
  /*
   * Assuming char is represented as a single-character string
   */
  char *ch;
  int32_t client_id;
  int32_t position_in_array;
};
#define LAYER_CHAR__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&layer_char__descriptor) \
    , (char *)protobuf_c_empty_string, 0, 0 }


/*
 * Message for layer_cell
 */
struct  _LayerCell
{
  ProtobufCMessage base;
  /*
   * A repeated field to represent the stack of layer_char
   */
  size_t n_stack;
  LayerChar **stack;
  /*
   * Index of the top of the stack
   */
  int32_t top;
  /*
   * Capacity of the stack
   */
  int32_t capacity;
};
#define LAYER_CELL__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&layer_cell__descriptor) \
    , 0,NULL, 0, 0 }


/*
 * Message for window_matrix
 */
struct  _WindowMatrix
{
  ProtobufCMessage base;
  /*
   * A repeated field to represent 1D array matrix of layer_cell
   */
  size_t n_cells;
  LayerCell **cells;
  /*
   * Number of rows of the matrix
   */
  int32_t width;
  /*
   * Number of columns of the matrix
   */
  int32_t height;
};
#define WINDOW_MATRIX__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&window_matrix__descriptor) \
    , 0,NULL, 0, 0 }


/*
 * Message for window_data
 */
struct  _WindowData
{
  ProtobufCMessage base;
  /*
   * Nested message for window_matrix
   */
  WindowMatrix *matrix;
};
#define WINDOW_DATA__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&window_data__descriptor) \
    , NULL }


/* LayerChar methods */
void   layer_char__init
                     (LayerChar         *message);
size_t layer_char__get_packed_size
                     (const LayerChar   *message);
size_t layer_char__pack
                     (const LayerChar   *message,
                      uint8_t             *out);
size_t layer_char__pack_to_buffer
                     (const LayerChar   *message,
                      ProtobufCBuffer     *buffer);
LayerChar *
       layer_char__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   layer_char__free_unpacked
                     (LayerChar *message,
                      ProtobufCAllocator *allocator);
/* LayerCell methods */
void   layer_cell__init
                     (LayerCell         *message);
size_t layer_cell__get_packed_size
                     (const LayerCell   *message);
size_t layer_cell__pack
                     (const LayerCell   *message,
                      uint8_t             *out);
size_t layer_cell__pack_to_buffer
                     (const LayerCell   *message,
                      ProtobufCBuffer     *buffer);
LayerCell *
       layer_cell__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   layer_cell__free_unpacked
                     (LayerCell *message,
                      ProtobufCAllocator *allocator);
/* WindowMatrix methods */
void   window_matrix__init
                     (WindowMatrix         *message);
size_t window_matrix__get_packed_size
                     (const WindowMatrix   *message);
size_t window_matrix__pack
                     (const WindowMatrix   *message,
                      uint8_t             *out);
size_t window_matrix__pack_to_buffer
                     (const WindowMatrix   *message,
                      ProtobufCBuffer     *buffer);
WindowMatrix *
       window_matrix__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   window_matrix__free_unpacked
                     (WindowMatrix *message,
                      ProtobufCAllocator *allocator);
/* WindowData methods */
void   window_data__init
                     (WindowData         *message);
size_t window_data__get_packed_size
                     (const WindowData   *message);
size_t window_data__pack
                     (const WindowData   *message,
                      uint8_t             *out);
size_t window_data__pack_to_buffer
                     (const WindowData   *message,
                      ProtobufCBuffer     *buffer);
WindowData *
       window_data__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   window_data__free_unpacked
                     (WindowData *message,
                      ProtobufCAllocator *allocator);
/* --- per-message closures --- */

typedef void (*LayerChar_Closure)
                 (const LayerChar *message,
                  void *closure_data);
typedef void (*LayerCell_Closure)
                 (const LayerCell *message,
                  void *closure_data);
typedef void (*WindowMatrix_Closure)
                 (const WindowMatrix *message,
                  void *closure_data);
typedef void (*WindowData_Closure)
                 (const WindowData *message,
                  void *closure_data);

/* --- services --- */


/* --- descriptors --- */

extern const ProtobufCMessageDescriptor layer_char__descriptor;
extern const ProtobufCMessageDescriptor layer_cell__descriptor;
extern const ProtobufCMessageDescriptor window_matrix__descriptor;
extern const ProtobufCMessageDescriptor window_data__descriptor;

PROTOBUF_C__END_DECLS


#endif  /* PROTOBUF_C_window_5fdata_2eproto__INCLUDED */