#define FIFO_LOCATION "/tmp/requests-fifo"
#define MAX_USERS 10

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

// message update to send position of users
typedef struct window_update
{
    int amount_of_users;
    user users[MAX_USERS];
} window_update;
