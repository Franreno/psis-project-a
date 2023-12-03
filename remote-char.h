typedef enum direction_t {
    UP,
    DOWN,
    LEFT,
    RIGHT
} direction_t;

typedef struct message {
    int msg_type; // 0 - join - provides char to use; 1 - move - provide the move
    char ch;
    direction_t direction;
} message;

typedef struct user {
    char ch;
    int x;
    int y;
} user;

#define SERVER_SOCKET_IP "ipc:///tmp/server"