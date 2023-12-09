#ifndef REMOTE_CHAR_H
#define REMOTE_CHAR_H

#define DEFAULT_SERVER_SOCKET_ADDRESS "ipc:///tmp/server"
#define DEFAULT_PUBLISH_SERVER_SOCKET_ADDRESS "tcp://*:5556"
#define DEFAULT_SERVER_ADDRESS "localhost"
#define DEFAULT_SERVER_PORT "5555"
#define WINDOW_SIZE 30
#define ROACH_MOVE_CHANCE 50
#define ROACH_MOVE_DELAY 1000000
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
    LIZARD_BODY,
} client_type;

typedef struct message_to_server
{
    int client_id;         // 1 = lizard, 2 = roach, 3 = display-app
    int type;              // 1 = connect, 2 = movement
    int value;             // usage depends on client_id and type
    direction_t direction; // direction to move the roach
    char message_accepted; // 1 = message accepted, 0 = message rejected
} message_to_server;

typedef struct lizard
{
    char ch;
    int x;
    int y;
    int score;
    direction_t previous_direction;
    char is_winner;
} lizard;

typedef struct roach
{
    char ch;
    int x;
    int y;
    char is_eaten;
    time_t timestamp;
} roach;

typedef struct field_update_movement
{
    int num_roaches;
    int num_lizards;
    message_to_server message;
    int new_x;
    int new_y;
    direction_t prev_direction;
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
