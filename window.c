#include "window.h"

void window_init(window_data **data, int width, int height)
{
    *data = (window_data *)malloc(sizeof(window_data));

    (*data)->win = newwin(width, height, 0, 0);
    box((*data)->win, 0, 0);
    wrefresh((*data)->win);

    (*data)->matrix = (window_matrix *)malloc(sizeof(window_matrix));
    init_window_matrix((*data)->matrix, width, height);
}

void window_init_with_matrix(window_data **data, int width, int height, char *serialized_matrix)
{
    // Allocate memory for window_data
    *data = (window_data *)malloc(sizeof(window_data));
    if (!*data)
    {
        // Handle allocation failure
        return;
    }

    // Initialize ncurses window
    (*data)->win = newwin(width, height, 0, 0);
    box((*data)->win, 0, 0);
    wrefresh((*data)->win);

    // Allocate memory for window_matrix
    (*data)->matrix = (window_matrix *)malloc(sizeof(window_matrix));
    if (!(*data)->matrix)
    {
        // Handle allocation failure
        delwin((*data)->win);
        free(*data);
        return;
    }

    // Initialize window_matrix dimensions
    (*data)->matrix->width = width;
    (*data)->matrix->height = height;
    (*data)->matrix->cells = malloc(sizeof(layer_cell) * width * height);
    if (!(*data)->matrix->cells)
    {
        // Handle allocation failure
        delwin((*data)->win);
        free((*data)->matrix);
        free(*data);
        return;
    }

    // Deserialize the matrix from the buffer
    deserialize_window_matrix((*data)->matrix, serialized_matrix);
}

void init_window_matrix(window_matrix *matrix, int width, int height)
{
    matrix->width = width;
    matrix->height = height;
    matrix->cells = malloc(sizeof(layer_cell) * width * height);
    for (int i = 0; i < width * height; ++i)
    {
        matrix->cells[i].stack = malloc(sizeof(layer_char) * 1);
        matrix->cells[i].top = -1;
        matrix->cells[i].capacity = 0;
    }
}

// This function pushes a new character onto the stack at a given position
void window_matrix_add_char(window_matrix *matrix, int x, int y, char ch)
{
    int index = INDEX(matrix->width, x, y);
    layer_cell *cell = &matrix->cells[index];

    // Check if we need to increase the capacity of the stack
    if (cell->top == cell->capacity - 1)
    {
        // Increase the capacity of the stack
        int new_capacity = cell->capacity + 1; // Or more, if you expect many layers
        cell->stack = realloc(cell->stack, new_capacity * sizeof(layer_char));
        cell->capacity = new_capacity;
    }

    // Push the new character onto the stack
    cell->top++;
    cell->stack[cell->top].ch = ch;
}

// This function updates the ncurses window to show the top character of the stack at a given position
void window_draw(window_data *data, int x, int y, char ch)
{
    window_matrix_add_char(data->matrix, x, y, ch);
    wmove(data->win, x, y);
    waddch(data->win, ch | A_BOLD);
    wrefresh(data->win);
}

// This function peeks at the character below the top character in the stack at a given position
char window_matrix_peek_below_top_char(window_matrix *matrix, int x, int y)
{
    int index = INDEX(matrix->width, x, y);
    layer_cell *cell = &matrix->cells[index];

    if (cell->top > 0)
    {
        // Peek at the character below the top character
        char ch = cell->stack[cell->top - 1].ch;
        return ch;
    }
    return ' '; // Return a space if the stack is empty or has only one element
}

// This function removes the top character from the stack
char window_matrix_remove_top_char(window_matrix *matrix, int x, int y)
{
    int index = INDEX(matrix->width, x, y);
    layer_cell *cell = &matrix->cells[index];

    if (cell->top >= 0)
    {
        // Pop the top character from the stack
        cell->top--;
        if (cell->top >= 0)
        {
            return cell->stack[cell->top].ch;
        }
    }
    return ' '; // Return a space if the stack is empty
}

// This function erases the top character at a given position and updates the ncurses window to show the next character in the stack
void window_erase(window_data *data, int x, int y)
{
    // Peek at the character below the top character
    char ch_below = window_matrix_peek_below_top_char(data->matrix, x, y);

    // Remove the top character from the stack
    window_matrix_remove_top_char(data->matrix, x, y);

    wmove(data->win, x, y);
    if (ch_below != ' ')
        waddch(data->win, ch_below | A_BOLD);
    else
        waddch(data->win, ' ');

    wrefresh(data->win);
}

void window_refresh(window_data *data)
{
    wrefresh(data->win);
}

void window_destroy(window_data *data)
{
    if (data != NULL)
    {
        if (data->win != NULL)
        {
            delwin(data->win);
        }
        if (data->matrix != NULL)
        {
            free_window_matrix(data->matrix);
            free(data->matrix);
        }
        free(data);
    }
    endwin();
}

void free_window_matrix(window_matrix *matrix)
{
    for (int i = 0; i < matrix->width * matrix->height; ++i)
    {
        free(matrix->cells[i].stack);
    }
    free(matrix->cells);
}

void serialize_window_matrix(window_matrix *matrix, char **buffer, size_t *buffer_size)
{
    int cell_count = matrix->width * matrix->height;
    int layer_char_size = sizeof(layer_char);

    // Calculate buffer size
    *buffer_size = sizeof(int) * 2; // for width and height
    for (int i = 0; i < cell_count; i++)
    {
        *buffer_size += sizeof(int) * 2;                             // for capacity and top of each cell
        *buffer_size += matrix->cells[i].capacity * layer_char_size; // for stack elements
    }

    *buffer = malloc(*buffer_size);
    if (!*buffer)
    {
        *buffer_size = 0;
        return;
    }

    char *ptr = *buffer;

    // Serialize width and height
    memcpy(ptr, &matrix->width, sizeof(int));
    ptr += sizeof(int);
    memcpy(ptr, &matrix->height, sizeof(int));
    ptr += sizeof(int);

    // Serialize each cell
    for (int i = 0; i < cell_count; i++)
    {
        layer_cell cell = matrix->cells[i];

        // Serialize capacity and top
        memcpy(ptr, &cell.capacity, sizeof(int));
        ptr += sizeof(int);
        memcpy(ptr, &cell.top, sizeof(int));
        ptr += sizeof(int);

        // Serialize stack elements
        if (cell.capacity > 0)
        {
            for (int j = 0; j <= cell.top; j++)
            {
                memcpy(ptr, &cell.stack[j], layer_char_size);
                ptr += layer_char_size;
            }
        }
    }
}

void deserialize_window_matrix(window_matrix *matrix, char *buffer)
{
    char *ptr = buffer;
    int layer_char_size = sizeof(layer_char);

    // Deserialize width and height
    memcpy(&matrix->width, ptr, sizeof(int));
    ptr += sizeof(int);
    memcpy(&matrix->height, ptr, sizeof(int));
    ptr += sizeof(int);

    // Initialize cells
    matrix->cells = malloc(matrix->width * matrix->height * sizeof(layer_cell));

    // Deserialize each cell
    for (int i = 0; i < matrix->width * matrix->height; i++)
    {
        int capacity, top;

        // Deserialize capacity and top
        memcpy(&capacity, ptr, sizeof(int));
        ptr += sizeof(int);
        memcpy(&top, ptr, sizeof(int));
        ptr += sizeof(int);

        // Allocate stack and deserialize elements if capacity > 0
        if (capacity > 0)
        {
            matrix->cells[i].stack = malloc(capacity * layer_char_size);
            for (int j = 0; j <= top; j++)
            {
                memcpy(&matrix->cells[i].stack[j], ptr, layer_char_size);
                ptr += layer_char_size;
            }
        }
        else
        {
            matrix->cells[i].stack = NULL;
        }

        matrix->cells[i].capacity = capacity;
        matrix->cells[i].top = top;
    }
}

void draw_entire_matrix(window_data *data)
{
    if (data == NULL || data->win == NULL || data->matrix == NULL)
    {
        // Handle error: Invalid data
        return;
    }

    for (int y = 0; y < data->matrix->height; y++)
    {
        for (int x = 0; x < data->matrix->width; x++)
        {
            int index = y * data->matrix->width + x; // Calculate the index in the matrix
            layer_cell cell = data->matrix->cells[index];

            // Only draw if there is at least one character in the cell's stack
            if (cell.top >= 0)
            {
                layer_char top_char = cell.stack[cell.top];
                wmove(data->win, x, y);
                waddch(data->win, top_char.ch);
                wrefresh(data->win);
            }
        }
    }

    // Refresh the window to update the screen
    wrefresh(data->win);
}

void print_window_matrix(window_matrix *matrix)
{
    if (matrix == NULL)
    {
        printf("Matrix is NULL\n");
        return;
    }

    printf("Matrix Width: %d, Height: %d\n", matrix->width, matrix->height);
    for (int y = 0; y < matrix->height; y++)
    {
        for (int x = 0; x < matrix->width; x++)
        {
            int index = y * matrix->width + x;
            layer_cell cell = matrix->cells[index];
            printf("Cell [%d,%d]: Capacity: %d, Top: %d, Characters: ", x, y, cell.capacity, cell.top);
            if (cell.top >= 0)
            {
                for (int j = 0; j <= cell.top; j++)
                {
                    printf("%c ", cell.stack[j].ch);
                }
            }
            printf("\n");
        }
    }
}