#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <stdio.h>

#include <zmq.h>

#include <ncurses.h>

#include "remote-char.h"
#include "logger.h"
#include "window.h"

int success = SUCCESS;
int failure = FAILURE;

typedef struct variable_that_roach_need
{
    message_to_server *recv_message;
    roach *roaches;
    void *responder;
    int *num_roaches;
    int *slot_roaches;
    window_data *game_window;
} variable_that_roach_need;

typedef struct variable_that_lizard_need
{
    message_to_server *recv_message;
    lizard *lizards;
    void *responder;
    int *num_lizards;
    int *slot_lizards;
    window_data *game_window;
} variable_that_lizard_need;

void print_constants()
{
    printf("The server is running with the following parameters:\n");
    printf("WINDOW_SIZE: %d\n", WINDOW_SIZE);
    printf("ROACH_MOVE_CHANCE: %d\n", ROACH_MOVE_CHANCE);
    printf("MAX_ROACH_SCORE: %d\n", MAX_ROACH_SCORE);
    printf("MAX_ROACHES_GENERATED: %d\n", MAX_ROACHES_GENERATED);
    printf("MAX_ROACHES_ALLOWED: %d\n", MAX_ROACHES_ALLOWED);
    printf("MAX_LIZARDS_ALLOWED: %d\n", MAX_LIZARDS_ALLOWED);
}

void new_position(int *x, int *y, direction_t direction)
{
    switch (direction)
    {
    case UP:
        (*x)--;
        if (*x == 0)
            *x = 2;
        break;
    case DOWN:
        (*x)++;
        if (*x == WINDOW_SIZE - 1)
            *x = WINDOW_SIZE - 3;
        break;
    case LEFT:
        (*y)--;
        if (*y == 0)
            *y = 2;
        break;
    case RIGHT:
        (*y)++;
        if (*y == WINDOW_SIZE - 1)
            *y = WINDOW_SIZE - 3;
        break;
    default:
        break;
    }
}

void process_display_app_message(void *responder, window_data *data)
{
    // Serialize the window matrix
    char *serialized_data;
    size_t serialized_size;
    serialize_window_matrix(data->matrix, &serialized_data, &serialized_size);

    if (!serialized_data)
    {
        // Handle memory allocation error
        // Optionally, send an error message back to the client
        int error_code = -1; // Example error code
        zmq_send(responder, &error_code, sizeof(error_code), 0);
        return;
    }

    // Send the serialized data
    zmq_send(responder, &serialized_size, sizeof(serialized_size), ZMQ_SNDMORE);
    zmq_send(responder, serialized_data, serialized_size, 0);

    // Cleanup
    free(serialized_data);
}

void process_lizard_connect(variable_that_lizard_need *lizard_payload)
{
    // If there are available slots, add the lizard to the array
    if (*(lizard_payload->slot_lizards) <= 0)
    {
        log_write("No more lizard slots available\n");
        // Reply indicating failure adding the lizard
        zmq_send(lizard_payload->responder, &failure, sizeof(int), 0);
        return;
    }
    // Increment the number of lizards and decrement the available slots
    (*(lizard_payload->num_lizards))++;
    (*(lizard_payload->slot_lizards))--;

    int id = *(lizard_payload->num_lizards) - 1;

    // Initialize the lizard in a random position
    lizard_payload->lizards[id].ch = (char)lizard_payload->recv_message;
    lizard_payload->lizards[id].x = rand() % (WINDOW_SIZE - 2) + 1;
    lizard_payload->lizards[id].x = rand() % (WINDOW_SIZE - 2) + 1;
    lizard_payload->lizards[id].y = rand() % (WINDOW_SIZE - 2) + 1;
    lizard_payload->lizards[id].score = 0;

    // Draw the lizard in the random position
    window_draw(lizard_payload->game_window, lizard_payload->lizards[id].x, lizard_payload->lizards[id].y, (lizard_payload->lizards[id].ch) | A_BOLD);

    // Reply indicating position of the lizard in the array
    zmq_send(lizard_payload->responder, &id, sizeof(int), 0);
}

void process_lizard_movement(variable_that_lizard_need *lizard_payload)
{
    // Move the specified lizard
    log_write("Moving lizard %d\n", lizard_payload->recv_message->value);
    int id = lizard_payload->recv_message->value;
    direction_t direction = lizard_payload->recv_message->direction;

    int new_x = lizard_payload->lizards[id].x;
    int new_y = lizard_payload->lizards[id].y;
    new_position(&new_x, &new_y, direction);

    chtype ch = mvinch(new_x, new_y) & A_CHARTEXT;

    if (ch != ' ' && ch != '.')
    {
        log_write("Lizard %d collided with something %c\n", id, ch);
        zmq_send(lizard_payload->responder, &success, sizeof(int), 0);
        return;
    }

    // Erase the lizard from the screen
    window_erase(lizard_payload->game_window, lizard_payload->lizards[id].x, lizard_payload->lizards[id].y);

    // Update the lizard position
    lizard_payload->lizards[id].x = new_x;
    lizard_payload->lizards[id].y = new_y;

    // Draw the lizard in the new position
    window_draw(lizard_payload->game_window, lizard_payload->lizards[id].x, lizard_payload->lizards[id].y, (lizard_payload->lizards[id].ch) | A_BOLD);

    // Reply indicating success moving the lizard
    zmq_send(lizard_payload->responder, &success, sizeof(int), 0);
}

void process_lizard_disconnect(variable_that_lizard_need *lizard_payload)
{
    int id = lizard_payload->recv_message->value;
    window_erase(lizard_payload->game_window, lizard_payload->lizards[id].x, lizard_payload->lizards[id].y);
    zmq_send(lizard_payload->responder, &success, sizeof(int), 0);
    (*(lizard_payload->num_lizards))--;
    (*(lizard_payload->slot_lizards))++;
}

void process_lizard_message(variable_that_lizard_need *lizard_payload)
{
    switch (lizard_payload->recv_message->type)
    {
        case CONNECT:
            log_write("Processing lizard connect message\n");
            process_lizard_connect(lizard_payload);
            break;

        case MOVEMENT:
            log_write("Processing lizard movement message\n");
            process_lizard_movement(lizard_payload);
            break;

        case DISCONNECT:
            log_write("Processing lizard disconnect message\n");
            process_lizard_disconnect(lizard_payload);
            break;
    }
}

void process_roach_connect(variable_that_roach_need *roach_payload)
{
    // If there are available slots, add the roach to the array
    if (*(roach_payload->slot_roaches) <= 0)
    {
        log_write("No more roach slots available\n");
        // Reply indicating failure adding the roach
        zmq_send(roach_payload->responder, &failure, sizeof(int), 0);
        return;
    }
    // Increment the number of roaches and decrement the available slots
    (*(roach_payload->num_roaches))++;
    (*(roach_payload->slot_roaches))--;

    int id = *(roach_payload->num_roaches) - 1;

    // Initialize the roach in a random position
    roach_payload->roaches[id].ch = roach_payload->recv_message->value;
    roach_payload->roaches[id].x = rand() % (WINDOW_SIZE - 2) + 1;
    roach_payload->roaches[id].y = rand() % (WINDOW_SIZE - 2) + 1;

    // Draw the roach in the random position
    window_draw(roach_payload->game_window, roach_payload->roaches[id].x, roach_payload->roaches[id].y, (roach_payload->roaches[id].ch + 48) | A_BOLD);

    // Reply indicating position of the roach in the array
    zmq_send(roach_payload->responder, &id, sizeof(int), 0);
}

void process_roach_movement(variable_that_roach_need *roach_payload)
{
    // Move the specified roach
    log_write("Moving roach %d\n", roach_payload->recv_message->value);
    int id = roach_payload->recv_message->value;
    direction_t direction = roach_payload->recv_message->direction;

    int new_x = roach_payload->roaches[id].x;
    int new_y = roach_payload->roaches[id].y;
    new_position(&new_x, &new_y, direction);

    chtype ch = mvinch(new_x, new_y) & A_CHARTEXT;

    if (ch != ' ' && ch != '.')
    {
        log_write("Roach %d collided with something %c\n", id, ch);
        zmq_send(roach_payload->responder, &success, sizeof(int), 0);
        return;
    }

    // Erase the roach from the screen
    window_erase(roach_payload->game_window, roach_payload->roaches[id].x, roach_payload->roaches[id].y);

    // Update the roach position
    roach_payload->roaches[id].x = new_x;
    roach_payload->roaches[id].y = new_y;

    // Draw the roach in the new position
    window_draw(roach_payload->game_window, roach_payload->roaches[id].x, roach_payload->roaches[id].y, (roach_payload->roaches[id].ch + 48) | A_BOLD);

    // Reply indicating success moving the roach
    zmq_send(roach_payload->responder, &success, sizeof(int), 0);
}

void process_roach_disconnect(variable_that_roach_need *roach_payload)
{
    int id = roach_payload->recv_message->value;
    window_erase(roach_payload->game_window, roach_payload->roaches[id].x, roach_payload->roaches[id].y);
    zmq_send(roach_payload->responder, &success, sizeof(int), 0);
    (*(roach_payload->num_roaches))--;
    (*(roach_payload->slot_roaches))++;
}

void process_roach_message(variable_that_roach_need *roach_payload)
{
    switch (roach_payload->recv_message->type)
    {
        case CONNECT:
            log_write("Processing roach connect message\n");
            process_roach_connect(roach_payload);
            break;

        case MOVEMENT:
            log_write("Processing roach movement message\n");
            process_roach_movement(roach_payload);
            break;

        case DISCONNECT:
            log_write("Processing roach disconnect message\n");
            process_roach_disconnect(roach_payload);
            break;
    }
}

int main()
{
    // Print the parameters the server is running with
    print_constants(); // NOT PRINTING ??

    // Initialize logger
    log_init("server.log");

    // Create socket address
    char *address = DEFAULT_SERVER_ADDRESS;
    char *port = DEFAULT_SERVER_PORT;
    char *server_socket_address = malloc(sizeof(char) * (strlen("tcp://") + strlen(address) + strlen(":") + strlen(port) + 1));
    strcpy(server_socket_address, "tcp://");
    strcat(server_socket_address, address);
    strcat(server_socket_address, ":");
    strcat(server_socket_address, port);
    server_socket_address = "ipc:///tmp/server"; // WORKAROUND
    printf("Connecting to server at %s\n", server_socket_address);

    // Create context
    void *context = zmq_ctx_new();
    if (context == NULL)
    {
        printf("Failed to create context: %s\n", zmq_strerror(errno));
        return 1;
    }

    // Create REP socket to receive messages from clients
    void *responder = zmq_socket(context, ZMQ_REP);
    if (responder == NULL)
    {
        printf("Failed to create REP socket: %s\n", zmq_strerror(errno));
        zmq_ctx_destroy(context);
        return 1;
    }
    // Create a publisher socket to send messages to the display app
    void *publisher = zmq_socket(context, ZMQ_PUB);
    if (publisher == NULL)
    {
        printf("Failed to create PUB socket: %s\n", zmq_strerror(errno));
        zmq_close(responder);
        zmq_ctx_destroy(context);
        return 1;
    }

    // Bind to the REP socket
    if (zmq_bind(responder, server_socket_address) != 0)
    {
        printf("Failed to bind REP socket: %s\n", zmq_strerror(errno));
        zmq_close(responder);
        zmq_close(publisher);
        zmq_ctx_destroy(context);
        return 1;
    }

    // Initialize ncurses
    initscr();
    cbreak();
    keypad(stdscr, TRUE);
    noecho();

    // Creates a window and draws a border
    window_data *game_window;
    window_init(&game_window, WINDOW_SIZE, WINDOW_SIZE);

    // Initialize variables used for roach tracking
    int num_roaches = 0;
    int slot_roaches = MAX_ROACHES_ALLOWED;
    roach roaches[MAX_ROACHES_ALLOWED];

    // Initialize variables used for lizard tracking
    int num_lizards = 0;
    int slot_lizards = MAX_LIZARDS_ALLOWED;
    lizard lizards[MAX_LIZARDS_ALLOWED];

    message_to_server recv_message;

    // Seed random number generator
    srand(time(NULL));

    variable_that_roach_need roach_payload;
    roach_payload.num_roaches = &num_roaches;
    roach_payload.slot_roaches = &slot_roaches;
    roach_payload.roaches = roaches;
    roach_payload.game_window = game_window;
    roach_payload.responder = responder;
    roach_payload.recv_message = &recv_message;

    variable_that_lizard_need lizard_payload;
    lizard_payload.num_lizards = &num_lizards;
    lizard_payload.slot_lizards = &slot_lizards;
    lizard_payload.lizards = lizards;
    lizard_payload.game_window = game_window;
    lizard_payload.responder = responder;
    lizard_payload.recv_message = &recv_message;

    while (1)
    {
        // Receive message from one of the clients
        zmq_recv(responder, &recv_message, sizeof(message_to_server), 0);
        log_write("Received message from client %d\n", recv_message.client_id);

        switch (recv_message.client_id)
        {
            case LIZARD:
                log_write("Processing lizard message\n");
                process_lizard_message(&lizard_payload);
                break;
            case ROACH:
                log_write("Processing roach message\n");
                process_roach_message(&roach_payload);
                break;
            case DISPLAY_APP:
                log_write("Processing display app message\n");
                process_display_app_message(responder, game_window);
                break;
            default:
                break;
        }

        // Publish the message to the display app
    }

    endwin();
    zmq_close(responder);
    zmq_ctx_destroy(context);
    log_close();

    return 0;
}