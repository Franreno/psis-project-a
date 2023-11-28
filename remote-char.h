// TODO_1
// declaration the struct corresponding to the exchanged messages
typedef enum direction_t
{
    UP,
    DOWN,
    LEFT,
    RIGHT
} direction_t;

typedef struct message
{
    int msg_type; // 0 - join - provides char to use; 1 - move - provide the move
    char ch;
    direction_t direction;
} message;

typedef struct user
{
    char ch;
    int x;
    int y;
} user;

// TODO_2
// declaration of the FIFO location

#define FIFO_LOCATION "/tmp/requests-fifo"