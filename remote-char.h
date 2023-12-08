#ifndef REMOTE_CHAR_H
#define REMOTE_CHAR_H

#define SUCCESS 1
#define FAILURE -1
#define FIFO_LOCATION "/tmp/requests-fifo"
#define DEFAULT_SERVER_ADDRESS "localhost"
#define DEFAULT_SERVER_PORT "5555"
#define WINDOW_SIZE 30
#define ROACH_MOVE_CHANCE 50
#define MAX_ROACH_SCORE 5
#define MAX_ROACHES_GENERATED 10
#define MAX_ROACHES_ALLOWED (WINDOW_SIZE * WINDOW_SIZE / 3)
#define MAX_LIZARDS_ALLOWED 10
#define MAX_LIZARD_SCORE 50

#include <time.h>

typedef enum direction_t
{
    UP,
    DOWN,
    LEFT,
    RIGHT,
} direction_t;

typedef enum message_type
{
    CONNECT,
    MOVEMENT,
    DISCONNECT,
} message_type;

typedef enum client_type
{
    LIZARD,
    ROACH,
    DISPLAY_APP,
} client_type;

// Message type clients use to communicate with the server
typedef struct message_to_server
{
    int client_id;         // 1 = lizard, 2 = roach, 3 = display-app
    int type;              // 1 = connect, 2 = movement
    int value;             // usage depends on client_id and type
    direction_t direction; // direction to move the roach
} message_to_server;

typedef struct lizard
{
    char ch;
    int x;
    int y;
    int score;
    int prev_x;
    int prev_y;
    char is_winner;
} lizard;

typedef struct roach
{
    char ch;
    int x;
    int y;
    // If roach was eaten
    char is_eaten;
    // Timestamp of when the roach was eaten
    time_t timestamp;
} roach;

typedef struct field_update_movement
{
    int num_roaches;
    int num_lizards;
    message_to_server message;
} field_update_movement;

typedef struct field_update_connect
{
    int client_id;
    int position_in_array;
    lizard connected_lizard;
    roach connected_roach;
    message_to_server message;
} field_update_connect;

typedef struct field_update_disconnect
{
    int client_id;
    int position_in_array;
    message_to_server message;
} field_update_disconnect;

#endif
