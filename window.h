#ifndef WINDOW_H
#define WINDOW_H

#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Constants
#define WINDOW_SIZE 30

typedef struct layer_char
{
    char ch;
} layer_char;

typedef struct layer_cell
{
    layer_char *stack; // Dynamic array (stack) of layer_char
    int top;           // Index of the top of the stack
    int capacity;      // Capacity of the stack
} layer_cell;

typedef struct window_matrix
{
    layer_cell *cells; // 1D array matrix of layer_cell
    int width;         // Number of rows of the matrix
    int height;        // Number of columns of the matrix
} window_matrix;

typedef struct window_data
{
    WINDOW *win;
    window_matrix *matrix;
} window_data;

// Function prototypes
void window_init(window_data **game_window, int width, int height);
void window_draw(window_data *game_window, int x, int y, char ch);
void window_erase(window_data *game_window, int x, int y, char ch);
void window_refresh(window_data *game_window);
void window_destroy(window_data *game_window);
// Functions to manipulate the window matrix
#define INDEX(width, x, y) ((y) * (width) + (x))
void init_window_matrix(window_matrix *matrix, int width, int height);
void free_window_matrix(window_matrix *matrix);
int get_char_priority(char ch);
char window_matrix_remove_char(window_matrix *matrix, int x, int y);
void window_matrix_add_char(window_matrix *matrix, int x, int y, char ch);
char window_matrix_peek_below_top_char(window_matrix *matrix, int x, int y);
void serialize_window_matrix(window_matrix *matrix, char **buffer, size_t *buffer_size);
void deserialize_window_matrix(window_matrix *matrix, char *buffer);
void window_init_with_matrix(window_data **data, int width, int height, char *serialized_matrix);
void draw_entire_matrix(window_data *data);
void print_window_matrix(window_matrix *matrix);
void window_matrix_remove_char_from_stack(window_matrix *matrix, int x, int y, char ch);

#endif
