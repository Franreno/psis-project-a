#include "default_consts.h"
#include "roach-mover.h"
#include "lizard-mover.h"
#include "logger.h"
#include "window.h"
#include "server_messages.pb-c.h"

/**
 * @brief Print the constants the server is running with
 *
 */
void print_constants()
{
    printf("The server is running with the following parameters:\n");
    printf("WINDOW_SIZE: %d\n", WINDOW_SIZE);
    printf("ROACH_MOVE_CHANCE: %d\n", ROACH_MOVE_CHANCE);
    printf("ROACH_MOVE_DELAY: %d\n", ROACH_MOVE_DELAY);
    printf("MAX_ROACH_SCORE: %d\n", MAX_ROACH_SCORE);
    printf("MAX_ROACHES_GENERATED: %d\n", MAX_ROACHES_GENERATED);
    printf("MAX_ROACHES_ALLOWED: %d\n", MAX_ROACHES_ALLOWED);
    printf("MAX_LIZARDS_ALLOWED: %d\n", MAX_LIZARDS_ALLOWED);
    printf("MAX_LIZARD_SCORE: %d\n", MAX_LIZARD_SCORE);
}

/**
 * @brief Create and connect sockets for clients to connect to
 *
 * @param rep_server_socket_address - REP server socket address
 * @param pub_server_socket_address - PUB server socket address
 * @param context - ZMQ context
 * @param responder - ZMQ socket
 * @param publisher - ZMQ socket
 * @return int - 0 if successful, -1 otherwise
 */
int create_and_connect_sockets(char *rep_server_socket_address, char *pub_server_socket_address, void **context, void **responder, void **publisher)
{
    // Create context
    *context = zmq_ctx_new();
    if (*context == NULL)
    {
        printf("Failed to create context: %s\n", zmq_strerror(errno));
        return -1;
    }

    // Create REQ socket to receive messages from clients
    *responder = zmq_socket(*context, ZMQ_REP);
    if (*responder == NULL)
    {
        printf("Failed to create REP socket: %s\n", zmq_strerror(errno));
        zmq_ctx_destroy(*context);
        return -1;
    }

    // Create PUB socket to send messages to the display app
    *publisher = zmq_socket(*context, ZMQ_PUB);
    if (*publisher == NULL)
    {
        printf("Failed to create PUB socket: %s\n", zmq_strerror(errno));
        zmq_close(*responder);
        zmq_ctx_destroy(*context);
        return -1;
    }

    // Bind to the REP socket
    if (zmq_bind(*responder, rep_server_socket_address) != 0)
    {
        printf("Failed to bind REP socket: %s\n", zmq_strerror(errno));
        zmq_close(*responder);
        zmq_close(*publisher);
        zmq_ctx_destroy(*context);
        return -1;
    }

    // Bind to the PUB socket
    if (zmq_bind(*publisher, pub_server_socket_address) != 0)
    {
        printf("Failed to bind PUB socket: %s\n", zmq_strerror(errno));
        zmq_close(*responder);
        zmq_close(*publisher);
        zmq_ctx_destroy(*context);
        return -1;
    }

    return 0;
}

/**
 * @brief - Serialize the window matrix, lizard mover and roach mover and send to the
 * connecting display app
 *
 * @param responder  - ZMQ socket
 * @param data - Window data
 * @param roach_mover - Roach mover
 * @param lizard_mover - Lizard mover
 */
void process_display_app_message(void *responder, window_data *data, roach_mover *roach_mover, lizard_mover *lizard_mover)
{
    // Serialize the window matrix
    char *serialized_data;
    size_t serialized_size;
    serialize_window_matrix(data->matrix, &serialized_data, &serialized_size);

    if (!serialized_data)
    {
        int error_code = -1; // Example error code
        zmq_send(responder, &error_code, sizeof(error_code), 0);
        return;
    }

    // Send the serialized data
    zmq_send(responder, &serialized_size, sizeof(serialized_size), ZMQ_SNDMORE);
    zmq_send(responder, serialized_data, serialized_size, ZMQ_SNDMORE);

    // Serialize the roach mover
    char *serialized_roach_mover;
    size_t serialized_roach_mover_size;
    serialize_roach_mover(roach_mover, &serialized_roach_mover, &serialized_roach_mover_size);

    log_write("Serialized roach mover size: %d\n", serialized_roach_mover_size);

    if (serialized_roach_mover == NULL)
    {
        log_write("Failed to serialize roach mover!!!\n");
        int error_code = -1; // Example error code
        zmq_send(responder, &error_code, sizeof(error_code), 0);
        return;
    }

    // Send the serialized data
    zmq_send(responder, &serialized_roach_mover_size, sizeof(serialized_roach_mover_size), ZMQ_SNDMORE);
    zmq_send(responder, serialized_roach_mover, serialized_roach_mover_size, ZMQ_SNDMORE);

    // Serialize the lizard mover
    char *serialized_lizard_mover;
    size_t serialized_lizard_mover_size;
    serialize_lizard_mover(lizard_mover, &serialized_lizard_mover, &serialized_lizard_mover_size);

    log_write("Serialized lizard mover size: %d\n", serialized_lizard_mover_size);

    if (serialized_lizard_mover == NULL)
    {
        log_write("Failed to serialize lizard mover!!!\n");
        int error_code = -1; // Example error code
        zmq_send(responder, &error_code, sizeof(error_code), 0);
        return;
    }

    // Send the serialized data
    zmq_send(responder, &serialized_lizard_mover_size, sizeof(serialized_lizard_mover_size), ZMQ_SNDMORE);
    zmq_send(responder, serialized_lizard_mover, serialized_lizard_mover_size, 0);

    // Cleanup
    free(serialized_data);
    free(serialized_roach_mover);
    free(serialized_lizard_mover);
}

/**
 * @brief - Publishes to the subscriber the move that happeend
 *
 * @param publisher - ZMQ socket
 * @param recv_message- Message received from the client
 * @param roach_payload- Roach mover
 * @param lizard_payload- Lizard mover
 */
void publish_movement(void *publisher, MessageToServer *recv_message, roach_mover *roach_payload, lizard_mover *lizard_payload)
{
    // Create field update message
    FieldUpdateMovement field_update_message = FIELD_UPDATE_MOVEMENT__INIT;
    field_update_message.message = recv_message;
    switch (recv_message->client_id)
    {
    case LIZARD:
        field_update_message.new_x = lizard_payload->lizards[recv_message->value].x;
        field_update_message.new_y = lizard_payload->lizards[recv_message->value].y;
        field_update_message.prev_direction = lizard_payload->lizards[recv_message->value].previous_direction;
        break;
    case ROACH:
        field_update_message.new_x = roach_payload->roaches[recv_message->value].x;
        field_update_message.new_y = roach_payload->roaches[recv_message->value].y;
        field_update_message.is_eaten = roach_payload->roaches[recv_message->value].is_eaten;
        break;
    }

    // Pack
    unsigned int len = field_update_movement__get_packed_size(&field_update_message);
    void *buf = malloc(len);
    field_update_movement__pack(&field_update_message, buf);

    // Send the message topic
    zmq_send(publisher, "field_update_movement", strlen("field_update_movement"), ZMQ_SNDMORE);

    // Send the serialized message
    zmq_send(publisher, buf, len, 0);

    // Free the allocated buffer
    free(buf);
}

void populate_lizard(Lizard **_lizard, lizard __lizard)
{
    *_lizard = malloc(sizeof(Lizard)); // Allocate memory for Lizard
    lizard__init(*_lizard);            // Initialize the Lizard object

    // Populate the Lizard fields
    (*_lizard)->ch = __lizard.ch; // Duplicate the string to avoid pointer issues
    (*_lizard)->x = __lizard.x;
    (*_lizard)->y = __lizard.y;
    (*_lizard)->score = __lizard.score;
    (*_lizard)->previous_direction = __lizard.previous_direction;
    (*_lizard)->is_winner = __lizard.is_winner;
}

void populate_roach(Roach **_roach, roach __roach)
{
    *_roach = malloc(sizeof(Roach)); // Allocate memory for Roach
    roach__init(*_roach);            // Initialize the Roach object

    // Populate the Roach fields
    (*_roach)->ch = __roach.ch;
    (*_roach)->x = __roach.x;
    (*_roach)->y = __roach.y;
    (*_roach)->is_eaten = __roach.is_eaten;
    (*_roach)->timestamp = __roach.timestamp;
}

/**
 * @brief - Publishes to the subscriber the connect that happeend
 *
 * @param publisher - ZMQ socket
 * @param recv_message - Message received from the client
 * @param roach_payload - Roach mover
 * @param lizard_payload - Lizard mover
 */
void publish_connect(void *publisher, MessageToServer *recv_message, roach_mover *roach_payload, lizard_mover *lizard_payload)
{
    // Assume FieldUpdateConnect is the name of your Protobuf message
    FieldUpdateConnect field_update_message = FIELD_UPDATE_CONNECT__INIT;

    // Populate the Protobuf message
    field_update_message.client_id = recv_message->client_id;
    int id = 0;
    switch (recv_message->client_id)
    {
    case LIZARD:
        Lizard *proto_lizard;
        populate_lizard(&proto_lizard, lizard_payload->lizards[id]);
        id = *(lizard_payload->num_lizards) - 1;
        field_update_message.position_in_array = id;
        field_update_message.connected_lizard = proto_lizard;
        break;
    case ROACH:
        Roach *proto_roach;
        populate_roach(&proto_roach, roach_payload->roaches[id]);
        id = *(roach_payload->num_roaches) - 1;
        field_update_message.position_in_array = id;
        field_update_message.connected_roach = proto_roach;
        break;
    }

    // Serialize the message
    unsigned int len = field_update_connect__get_packed_size(&field_update_message);
    void *buf = malloc(len);
    field_update_connect__pack(&field_update_message, buf);

    // Send the message topic
    zmq_send(publisher, "field_update_connect", strlen("field_update_connect"), ZMQ_SNDMORE);

    // Send the serialized message
    zmq_send(publisher, buf, len, 0);

    // Free the allocated buffer
    free(buf);
}

/**
 * @brief - Publishes to the subscriber the disconnection of a client
 *
 * @param publisher  - ZMQ socket
 * @param recv_message - Message received from the client
 */
void publish_disconnect(void *publisher, MessageToServer *recv_message)
{
    // Create field update message
    FieldUpdateDisconnect field_update_message = FIELD_UPDATE_DISCONNECT__INIT;
    field_update_message.message = recv_message;
    field_update_message.client_id = recv_message->client_id;
    field_update_message.position_in_array = recv_message->value;

    // Serialize the message
    unsigned int len = field_update_disconnect__get_packed_size(&field_update_message);
    void *buf = malloc(len);
    field_update_disconnect__pack(&field_update_message, buf);

    zmq_send(publisher, "field_update_disconnect", strlen("field_update_disconnect"), ZMQ_SNDMORE);

    // Send the serialized message
    zmq_send(publisher, buf, len, 0);

    free(buf);
}

/**
 * @brief - Respawn the eaten roaches after 5 seconds
 *
 * @param roach_payload - Roach mover
 * @param eaten_roaches  - Array of eaten roaches
 * @param amount_eaten_roaches - Amount of eaten roaches
 */
void respawn_eaten_roaches(roach_mover *roach_payload, roach **eaten_roaches, int *amount_eaten_roaches)
{
    time_t current_time = time(NULL);

    roach *eaten_roach;
    for (int i = 0; i < *amount_eaten_roaches; i++)
    {
        eaten_roach = eaten_roaches[i];

        if (eaten_roach->is_eaten && difftime(current_time, eaten_roach->timestamp) >= 5)
        {
            int new_x;
            int new_y;
            layer_cell *cell;

            do
            {
                // Generate a random position
                new_x = rand() % (WINDOW_SIZE - 2) + 1;
                new_y = rand() % (WINDOW_SIZE - 2) + 1;

                // Get the stack info of the new position
                cell = get_cell(roach_payload->game_window->matrix, new_x, new_y);
            } while (cell->stack[cell->top].client_id == LIZARD || cell->stack[cell->top].client_id == ROACH);

            eaten_roach->x = new_x;
            eaten_roach->y = new_y;
            eaten_roach->is_eaten = 0;
            eaten_roach->timestamp = 0;
        }
    }
}

int main(int argc, char *argv[])
{
    // Print the parameters the server is running with
    print_constants();

    // Initialize logger
    log_init("server.log");

    void *context;
    void *responder;
    void *publisher;
    char *rep_server_socket_address;
    char *pub_server_socket_address;

    printf("Usage: ./lizardsNroaches-server <rep_server_address> <rep_server_port <pub_server_address> <pub_server_port>\n");

    // Check if address and port were provided as command line arguments, if not, use default values
    if (argc != 5)
    {
        printf("Addresses and ports were not provided!\n");
        printf("Using default REP server socket address: %s\n", DEFAULT_SERVER_SOCKET_ADDRESS);
        rep_server_socket_address = malloc(strlen(DEFAULT_SERVER_SOCKET_ADDRESS) + 1);
        rep_server_socket_address = DEFAULT_SERVER_SOCKET_ADDRESS;
        printf("Using default PUB server socket address: %s\n", DEFAULT_PUBLISH_SERVER_SOCKET_ADDRESS);
        pub_server_socket_address = malloc(strlen(DEFAULT_PUBLISH_SERVER_SOCKET_ADDRESS) + 1);
        pub_server_socket_address = DEFAULT_PUBLISH_SERVER_SOCKET_ADDRESS;
    }
    else
    {
        printf("Using REP address and port: %s %s\n", argv[1], argv[2]);
        char *rep_address = argv[1];
        char *rep_port = argv[2];
        rep_server_socket_address = malloc(strlen("tcp://") + strlen(rep_address) + strlen(":") + strlen(rep_port) + 1);
        strcpy(rep_server_socket_address, "tcp://");
        strcat(rep_server_socket_address, rep_address);
        strcat(rep_server_socket_address, ":");
        strcat(rep_server_socket_address, rep_port);

        printf("Using PUB address and port: %s %s\n", argv[3], argv[4]);
        char *pub_address = argv[3];
        char *pub_port = argv[4];
        pub_server_socket_address = malloc(strlen("tcp://") + strlen(pub_address) + strlen(":") + strlen(pub_port) + 1);
        strcpy(pub_server_socket_address, "tcp://");
        strcat(pub_server_socket_address, rep_address);
        strcat(pub_server_socket_address, ":");
        strcat(pub_server_socket_address, rep_port);
    }

    // Create and connect sockets for clients to connect to
    if (create_and_connect_sockets(rep_server_socket_address, pub_server_socket_address, &context, &responder, &publisher) != 0)
        return -1;

    // Initialize ncurses
    initscr();
    cbreak();
    keypad(stdscr, TRUE);
    noecho();

    zmq_msg_t message;
    zmq_msg_init(&message);

    // Creates a window and draws a border
    window_data *game_window;
    window_init(&game_window, WINDOW_SIZE, WINDOW_SIZE);

    // Create a new window for the scores
    WINDOW *score_window = newwin(MAX_LIZARDS_ALLOWED, 50, 0, WINDOW_SIZE + 2);

    // Initialize variables used for roach tracking
    int num_roaches = 0;
    int slot_roaches = MAX_ROACHES_ALLOWED;
    roach roaches[MAX_ROACHES_ALLOWED];

    // Initialize variables used for lizard tracking
    int num_lizards = 0;
    int slot_lizards = MAX_LIZARDS_ALLOWED;
    lizard lizards[MAX_LIZARDS_ALLOWED];

    MessageToServer *recv_message;

    roach_mover *roach_payload = malloc(sizeof(roach_mover));
    log_write("Creating roach mover\n");
    new_roach_mover(&roach_payload, recv_message, roaches, responder, &num_roaches, &slot_roaches, game_window);
    roach_payload->should_use_responder = 1;

    lizard_mover *lizard_payload = malloc(sizeof(lizard_mover));
    log_write("Creating lizard mover\n");
    new_lizard_mover(&lizard_payload, recv_message, lizards, responder, &num_lizards, &slot_lizards, game_window);
    lizard_payload->should_use_responder = 1;

    roach_payload->lizards = lizards;
    lizard_payload->roaches = roaches;

    // Create pointer to eaten roaches
    roach **eaten_roaches = (roach **)malloc(sizeof(roach *) * MAX_ROACHES_ALLOWED);
    int eaten_roaches_count = 0;

    roach_payload->eaten_roaches = eaten_roaches;
    roach_payload->amount_eaten_roaches = &eaten_roaches_count;
    lizard_payload->eaten_roaches = eaten_roaches;
    lizard_payload->amount_eaten_roaches = &eaten_roaches_count;

    while (1)
    {
        // Receive a message part from the socket
        if (zmq_msg_recv(&message, responder, 0) == -1)
        {
            // Handle error
            log_write("Error receiving message: %s\n", zmq_strerror(zmq_errno()));
            zmq_msg_close(&message);
            continue;
        }

        // Get the size of the received message
        size_t msg_size = zmq_msg_size(&message);
        void *msg_data = zmq_msg_data(&message);

        // Unpack the Protobuf message
        recv_message = message_to_server__unpack(NULL, msg_size, msg_data);
        if (recv_message == NULL)
        {
            // Handle error - for example, log and continue
            log_write("Error unpacking received message\n");
            zmq_msg_close(&message);
            continue;
        }

        // Receive message from one of the clients
        zmq_recv(responder, &recv_message, sizeof(MessageToServer), 0);
        log_write("Received message from client %d\n", recv_message->client_id);

        respawn_eaten_roaches(roach_payload, eaten_roaches, &eaten_roaches_count);

        // Print the scores in the score window
        int i, j;
        for (i = 0, j = 0; j < num_lizards; i++)
        {
            if (lizards[i].ch == -1)
                continue;

            // j is the line number and only increments when a lizard is printed
            mvwprintw(score_window, j, 0, "Lizard id %c: Score %d", (char)lizards[i].ch, lizards[i].score);
            j++;

            // Clear the rest of the line
            wclrtoeol(score_window);
        }

        // Clear the remaining lines
        for (; j < MAX_LIZARDS_ALLOWED; j++)
        {
            wmove(score_window, j, 0);
            wclrtoeol(score_window);
        }

        // Update the score window
        wrefresh(score_window);

        recv_message->message_accepted = 1;

        switch (recv_message->client_id)
        {
        case LIZARD:
            log_write("Processing lizard message\n");
            process_lizard_message(lizard_payload);
            log_write("Processed lizard message\n");
            break;
        case ROACH:
            log_write("Processing roach message\n");
            process_roach_message(roach_payload);
            break;
        case DISPLAY_APP:
            log_write("Processing display app message\n");
            process_display_app_message(responder, game_window, roach_payload, lizard_payload);
            log_write("Processed display app message\n");
            break;
        default:
            break;
        }

        respawn_eaten_roaches(roach_payload, eaten_roaches, &eaten_roaches_count);

        if (recv_message->client_id == DISPLAY_APP || recv_message->message_accepted == 0)
            continue;

        // Publish the message to the display app
        switch (recv_message->type)
        {
        case CONNECT:
            log_write("Publishing connect\n");
            publish_connect(publisher, recv_message, roach_payload, lizard_payload);
            break;
        case MOVEMENT:
            log_write("Publishing movement\n");
            publish_movement(publisher, recv_message, roach_payload, lizard_payload);
            break;
        case DISCONNECT:
            log_write("Publishing disconnect\n");
            publish_disconnect(publisher, recv_message);
            break;
        }

        // Free the Protobuf message
        message_to_server__free_unpacked(recv_message, NULL);
    }

    endwin();

    // Close the ZMQ message
    zmq_msg_close(&message);
    zmq_close(responder);
    zmq_ctx_destroy(context);

    free(eaten_roaches);
    log_close();

    return 0;
}
