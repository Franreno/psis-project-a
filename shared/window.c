#include "window.h"

/**
 * @brief - Initialize the ncurses window
 *
 * @param data - Pointer to the window_data struct
 * @param width - Width of the window
 * @param height - Height of the window
 */
void window_init(window_data **data, int width, int height)
{
    *data = (window_data *)malloc(sizeof(window_data));

    (*data)->win = newwin(width, height, 0, 0);
    box((*data)->win, 0, 0);
    wrefresh((*data)->win);

    (*data)->matrix = (window_matrix *)malloc(sizeof(window_matrix));
    init_window_matrix((*data)->matrix, width, height);

    // Create buffer for updated cells
    (*data)->updated_cell_indexes = (int *)malloc(sizeof(int) * width * height);
    (*data)->size_of_updated_cells = 0;
}

/**
 * @brief - Initialize the ncurses window with a matrix
 *
 * @param data - Pointer to the window_data struct
 * @param width - Width of the window
 * @param height - Height of the window
 * @param serialized_matrix - Serialized matrix to initialize the window with
 */
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

/**
 * @brief - Initialize the ncurses window matrix
 *
 * @param matrix - Pointer to the window_matrix struct
 * @param width - Width of the window
 * @param height -  Height of the window
 */
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

/**
 * @brief Get the char priority object
 *
 * @param ch - Character to get the priority of
 * @return int - Priority of the character
 */
int get_char_priority(char ch)
{
    if (isalpha(ch))
        return 4;
    if (isdigit(ch))
        return 3;
    if (ch == '#')
        return 2;
    if (ch == '.' || ch == '*')
        return 1;
    return 0;
}

/**
 * @brief - Add a character to the stack at a given position
 *
 * @param matrix - Pointer to the window_matrix struct
 * @param x - x coordinate
 * @param y - y coordinate
 * @param ch - Character to add
 * @param client_id - Client id
 * @param position_in_array - positon of client id in its array
 */
void window_matrix_add_char(window_data *data, int x, int y, char ch, int client_id, int position_in_array)
{
    window_matrix *matrix = data->matrix;
    int index = INDEX(matrix->width, x, y);
    track_cell_update(data, index);
    layer_cell *cell = &matrix->cells[index];
    int priority = get_char_priority(ch);

    // Check if we need to increase the capacity of the stack
    if (cell->top >= cell->capacity - 1)
    {
        // Increase the capacity of the stack
        int new_capacity = (cell->capacity == 0) ? 2 : (cell->capacity * 2);
        layer_char *new_stack = realloc(cell->stack, new_capacity * sizeof(layer_char));
        if (!new_stack)
        {
            exit(EXIT_FAILURE);
        }
        cell->stack = new_stack;
        cell->capacity = new_capacity;
    }

    // If the stack is empty or the new character has the highest priority, add it to the top
    if (cell->top == -1 || get_char_priority(cell->stack[cell->top].ch) <= priority)
    {
        cell->top++;
        cell->stack[cell->top].ch = ch;
        cell->stack[cell->top].client_id = client_id;
        cell->stack[cell->top].position_in_array = position_in_array;
    }
    else
    {
        // Find the correct position to insert the new character
        int insert_pos;
        for (insert_pos = cell->top; insert_pos >= 0; insert_pos--)
        {
            if (get_char_priority(cell->stack[insert_pos].ch) >= priority)
                cell->stack[insert_pos + 1] = cell->stack[insert_pos]; // Shift character up
            else
                break; // Found the insert position
        }
        // Insert the new character
        cell->stack[insert_pos + 1].ch = ch;
        cell->stack[insert_pos + 1].position_in_array = position_in_array;
        cell->stack[insert_pos + 1].client_id = client_id;
        cell->top++;
    }
}

/**
 * @brief - Draw a character at a given position
 *
 * @param matrix - Pointer to the window_matrix struct
 * @param x - x coordinate
 * @param y - y coordinate
 * @param ch - Character to add
 * @param client_id - Client id
 * @param position_in_array - positon of client id in its array
 */
void window_draw(window_data *data, int x, int y, char ch, int client_id, int position_in_array)
{
    // Add the character to the stack with priority
    window_matrix_add_char(data, x, y, ch, client_id, position_in_array);

    // Always draw the top character from the stack at the specified position
    int index = INDEX(data->matrix->width, x, y);
    layer_cell *cell = &data->matrix->cells[index];
    char top_char = (cell->top >= 0) ? cell->stack[cell->top].ch : ' ';

    // Move to the position and draw the top character
    wmove(data->win, x, y);
    waddch(data->win, top_char | A_BOLD);
    wrefresh(data->win);
}

/**
 * @brief - Peek at the character below the top character in the stack at a given position
 *
 * @param matrix - Pointer to the window_matrix struct
 * @param x - x coordinate
 * @param y - y coordinate
 * @return char - Character below the top character in the stack
 */
char window_matrix_peek_below_top_char(window_matrix *matrix, int x, int y)
{
    int index = INDEX(matrix->width, x, y);
    layer_cell *cell = &matrix->cells[index];

    if (cell->top > 0)
    {
        // Peek at the character below the top character
        char ch = cell->stack[cell->top].ch;
        return ch;
    }
    return ' '; // Return a space if the stack is empty or has only one element
}

/**
 * @brief - Remove the top character from the stack at a given position
 *
 * @param matrix - Pointer to the window_matrix struct
 * @param x - x coordinate
 * @param y - y coordinate
 * @return char - Top character from the stack
 */
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

/**
 * @brief - Remove a character from the stack at a given position
 *
 * @param matrix- Pointer to the window_matrix struct
 * @param x- x coordinate
 * @param y- y coordinate
 * @param ch- Character to remove
 */
void window_matrix_remove_char_from_stack(window_data *data, int x, int y, char ch)
{
    window_matrix *matrix = data->matrix;
    int index = INDEX(matrix->width, x, y);
    track_cell_update(data, index);
    layer_cell *cell = &matrix->cells[index];

    // Check if the character exists in the stack and find its position
    int pos = -1;
    for (int i = 0; i <= cell->top; i++)
    {
        if (cell->stack[i].ch == ch)
        {
            pos = i;
            break;
        }
    }

    // If the character was found, remove it and shift others down
    if (pos != -1)
    {
        for (int i = pos; i < cell->top; i++)
        {
            cell->stack[i] = cell->stack[i + 1];
        }
        cell->top--;
    }
}

/**
 * @brief - Erase a character from the stack at a given position
 *
 * @param data - Pointer to the window_data struct
 * @param x - x coordinate
 * @param y - y coordinate
 * @param ch - Character to erase
 */
void window_erase(window_data *data, int x, int y, char ch)
{
    // Remove the specified character from the stack
    window_matrix_remove_char_from_stack(data, x, y, ch);

    // Determine the index in the matrix for the cell at (x, y)
    int cell_index = INDEX(data->matrix->width, x, y);

    // Determine if there is any character left on the top of the stack
    char new_top_char = ' '; // Default to space if the stack is empty
    if (data->matrix->cells[cell_index].top >= 0)
    {
        new_top_char = data->matrix->cells[cell_index].stack[data->matrix->cells[cell_index].top].ch;
    }

    // Move to the position in the ncurses window
    wmove(data->win, x, y);

    // Draw the new top character or clear the cell
    if (new_top_char != ' ')
        waddch(data->win, new_top_char | A_BOLD);
    else
        waddch(data->win, ' '); // Clear the cell if the stack is empty

    // Refresh the window to reflect changes
    wrefresh(data->win);
}

/**
 * @brief - Refresh the ncurses window
 *
 * @param data - Pointer to the window_data struct
 */
void window_refresh(window_data *data)
{
    wrefresh(data->win);
}

/**
 * @brief - Destroy the ncurses window
 *
 * @param data - Pointer to the window_data struct
 */
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

        // Free the buffer for updated cell indexes
        if (data->updated_cell_indexes)
            free(data->updated_cell_indexes);
    }
    endwin();
}

/**
 * @brief Get the cell object
 *
 * @param matrix  - Pointer to the window_matrix struct
 * @param x - x coordinate
 * @param y - y coordinate
 * @return layer_cell* - Pointer to the cell at (x, y)
 */
layer_cell *get_cell(window_matrix *matrix, int x, int y)
{
    int index = INDEX(matrix->width, x, y);
    return &matrix->cells[index];
}

/**
 * @brief - Free the window matrix
 *
 * @param matrix  - Pointer to the window_matrix struct
 */
void free_window_matrix(window_matrix *matrix)
{
    for (int i = 0; i < matrix->width * matrix->height; ++i)
    {
        free(matrix->cells[i].stack);
    }
    free(matrix->cells);
}

/**
 * @brief - Serialize the window matrix
 *
 * @param matrix  - Pointer to the window_matrix struct
 * @param buffer - Pointer to the buffer
 * @param buffer_size - Pointer to the buffer size
 */
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
/**
 * @brief - Deserialize the window matrix
 *
 * @param matrix - Pointer to the window_matrix struct
 * @param buffer- Pointer to the buffer
 */
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

/**
 * @brief - Draw the entire matrix
 *
 * @param data - Pointer to the window_data struct
 */
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

void erase_cell(window_data *data, int cell_index)
{
    int x = cell_index % data->matrix->width;
    int y = cell_index / data->matrix->width;

    // Move to the position in the ncurses window
    wmove(data->win, x, y);
    waddch(data->win, ' ');

    // Refresh the window to reflect changes
    wrefresh(data->win);
}

void draw_cell(window_data *data, int cell_index, layer_cell *cell)
{
    int x = cell_index % data->matrix->width;
    int y = cell_index / data->matrix->width;

    // Always draw the top character from the stack at the specified position
    char top_char = (cell->top >= 0) ? cell->stack[cell->top].ch : ' ';

    // Move to the position and draw the top character
    wmove(data->win, x, y);
    waddch(data->win, top_char | A_BOLD);
    wrefresh(data->win);
}

void draw_updated_matrix(window_data *data, layer_cell *updated_cells, int *updated_cell_indexes, int size_of_updated_cells)
{
    for (int i = 0; i < size_of_updated_cells; i++)
    {
        int index_of_this_cell = updated_cell_indexes[i];

        layer_cell *cell = &updated_cells[i];

        // Erase old state of the cell
        erase_cell(data, index_of_this_cell);

        // Draw new state of the cell
        draw_cell(data, index_of_this_cell, cell);
    }
}

/**
 * @brief Updates the cells of a matrix in a window.
 *
 * This function iterates over a list of updated cells and their corresponding indexes in the matrix.
 * For each updated cell, it validates the index, frees the existing stack in the cell to be updated,
 * allocates new memory for the stack, copies the new stack data, and updates other properties of the cell.
 *
 * @param data The window data containing the matrix to be updated.
 * @param updated_cells An array of cells with updated data.
 * @param updated_cell_indexes An array of indexes in the matrix corresponding to the updated cells.
 * @param size_of_updated_cells The number of cells that have been updated.
 */
void update_matrix_cells(window_data *data, layer_cell *updated_cells, int *updated_cell_indexes, int size_of_updated_cells)
{
    for (int i = 0; i < size_of_updated_cells; i++)
    {
        int index_of_this_cell = updated_cell_indexes[i];

        // Validate index
        if (index_of_this_cell < 0 || index_of_this_cell >= data->matrix->width * data->matrix->height)
            continue;

        layer_cell *matrix_cell_with_new_data = &updated_cells[i];
        layer_cell *matrix_cell_to_be_updated = &data->matrix->cells[index_of_this_cell];

        // Free existing stack in the cell to be updated
        free(matrix_cell_to_be_updated->stack);

        // Allocate new memory for the stack and copy the new stack data
        matrix_cell_to_be_updated->stack = malloc(matrix_cell_with_new_data->capacity * sizeof(layer_char));
        if (matrix_cell_with_new_data->stack && matrix_cell_to_be_updated->stack)
            memcpy(matrix_cell_to_be_updated->stack, matrix_cell_with_new_data->stack,
                   matrix_cell_with_new_data->capacity * sizeof(layer_char));

        // Copy other properties of the cell
        matrix_cell_to_be_updated->capacity = matrix_cell_with_new_data->capacity;
        matrix_cell_to_be_updated->top = matrix_cell_with_new_data->top;
    }
}

/**
 * @brief Tracks the indexes of the cells that have been updated.
 *
 * This function checks if the cell index already exists in the array of updated cell indexes.
 * If it does, it does nothing. If it doesn't, it adds the cell index to the array.
 *
 * @param data The window data containing the matrix to be updated.
 * @param cell_index The index of the cell to be tracked.
 */
void track_cell_update(window_data *data, int cell_index)
{
    // Check if the cell index already exists in the array
    for (int i = 0; i < data->size_of_updated_cells; i++)
    {
        if (data->updated_cell_indexes[i] == cell_index)
        {
            // Index already tracked, no need to add it again
            return;
        }
    }

    // Check if there's enough space to add a new updated cell index
    if (data->size_of_updated_cells >= data->matrix->width * data->matrix->height)
    {
    }

    // Add the cell index to the tracking array
    data->updated_cell_indexes[data->size_of_updated_cells] = cell_index;
    data->size_of_updated_cells++;
}

void create_updated_layer_cells(window_data *data, layer_cell *updated_cells, int *updated_cell_indexes, int size_of_updated_cells)
{
    for (int i = 0; i < size_of_updated_cells; i++)
    {
        int index_of_this_cell = updated_cell_indexes[i];

        // Validate index
        if (index_of_this_cell < 0 || index_of_this_cell >= data->matrix->width * data->matrix->height)
            continue;

        layer_cell *matrix_cell_with_new_data = &updated_cells[i];
        layer_cell *matrix_cell_to_be_updated = &data->matrix->cells[index_of_this_cell];

        // Free existing stack in the cell to be updated
        free(matrix_cell_to_be_updated->stack);

        // Allocate new memory for the stack and copy the new stack data
        matrix_cell_to_be_updated->stack = malloc(matrix_cell_with_new_data->capacity * sizeof(layer_char));
        if (matrix_cell_with_new_data->stack && matrix_cell_to_be_updated->stack)
            memcpy(matrix_cell_to_be_updated->stack, matrix_cell_with_new_data->stack,
                   matrix_cell_with_new_data->capacity * sizeof(layer_char));

        // Copy other properties of the cell
        matrix_cell_to_be_updated->capacity = matrix_cell_with_new_data->capacity;
        matrix_cell_to_be_updated->top = matrix_cell_with_new_data->top;
    }
}