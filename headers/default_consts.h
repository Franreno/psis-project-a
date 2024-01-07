#ifndef REMOTE_CHAR_H
#define REMOTE_CHAR_H

#define DEFAULT_SERVER_SOCKET_ADDRESS "tcp://127.0.0.1:8080"
#define DEFAULT_PUBLISH_SERVER_SOCKET_ADDRESS "tcp://127.0.0.1:5556"
#define DEFAULT_SUBS_SERVER_SOCKET_ADDRESS "tcp://127.0.0.1:5556"
#define DEFAULT_SERVER_ADDRESS "localhost"
#define DEFAULT_SERVER_PORT "5555"
#define WINDOW_SIZE 30
#define CLIENT_TIMEOUT_SECONDS 30
#define ROACH_MOVE_CHANCE 50
#define WASP_MOVE_CHANCE 25
#define ROACH_MOVE_DELAY 1000000
#define WASP_MOVE_DELAY 2000000
#define MAX_ROACH_SCORE 5
#define MAX_ROACHES_GENERATED 10
#define MAX_WASPS_GENERATED 10
#define MAX_SLOTS_ALLOWED (WINDOW_SIZE * WINDOW_SIZE / 3)
#define MAX_LIZARDS_ALLOWED 10
#define MAX_LIZARD_SCORE 50
#define WASP_DAMAGE 10

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <signal.h>
#include <zmq.h>
#include <ncurses.h>
#include <pthread.h>

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
    WASP,
    DISPLAY_APP,
    LIZARD_BODY,
} client_type;

typedef struct message_to_server
{
    int client_id;         // 1 = lizard, 2 = roach, 3 = wasp, 4 = display app, 5 = lizard body
    int type;              // 1 = connect, 2 = movement
    int value;             // usage depends on client_id and type
    direction_t direction; // direction of movement
} message_to_server;

typedef struct lizard
{
    char ch;
    int x;
    int y;
    int score;
    char is_winner;
    direction_t previous_direction;
    time_t last_message_time;
} lizard;

typedef struct roach
{
    char ch;
    int x;
    int y;
    char is_eaten;
    time_t timestamp;
    time_t last_message_time;
} roach;

typedef struct wasp
{
    char ch;
    int x;
    int y;
    time_t last_message_time;
} wasp;

typedef struct field_update_movement
{
    int num_roaches;
    int num_lizards;
    message_to_server message;
    int new_x;
    int new_y;
    direction_t prev_direction;
    char is_eaten;
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
