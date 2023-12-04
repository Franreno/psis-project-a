#ifndef REMOTE_CHAR_H
#define REMOTE_CHAR_H

#define FIFO_LOCATION "/tmp/requests-fifo"
#define MAX_USERS 10
#define DEFAULT_SERVER_ADDRESS "localhost"
#define DEFAULT_SERVER_PORT "5555"
#define SUCCESS 1
#define FAILURE -1
#define WINDOW_SIZE 30
#define MAX_ROACHES 8

// TODO_1
// declaration the struct corresponding to the exchanged messages
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

// Message type roach clients use to communicate with the server
typedef struct message_to_server
{
    int client_id;         // 1 = lizard, 2 = roach, 3 = display-app
    int type;              // 1 = connect, 2 = movement
    int value;             // score of roach to connect or id of the roach to move
    direction_t direction; // direction to move the roach
} message_to_server;

typedef struct lizard
{
    char ch;
    int x;
    int y;
} lizard;

typedef struct roach
{
    char ch;
    int x;
    int y;
} roach;

typedef struct cell_state
{
    char ch;
} cell_state;

typedef struct board_state
{
    cell_state cells[WINDOW_SIZE][WINDOW_SIZE];
} board_state;

#endif