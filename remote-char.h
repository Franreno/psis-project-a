typedef enum direction_t {
    UP,
    DOWN,
    LEFT,
    RIGHT,
} direction_t;

// Message type roach clients use to communicate with the server
typedef struct message_to_server {
    int client_id; // 1 = lizard, 2 = roach, 3 = display-app
    int type; // 1 = connect, 2 = movement
    int value; // score of roach to connect or id of the roach to move
    direction_t direction; // direction to move the roach
} message_to_server;

typedef struct lizard {
    char ch;
    int x;
    int y;
} lizard;

typedef struct roach {
    char ch;
    int x;
    int y;
} roach;

#define DEFAULT_SERVER_ADDRESS "localhost"
#define DEFAULT_SERVER_PORT "5555"
#define SUCCESS 1
#define FAILURE -1