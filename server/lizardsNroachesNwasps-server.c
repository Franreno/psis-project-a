#include "default_consts.h"
#include "wasp-mover.h"
#include "roach-mover.h"
#include "lizard-mover.h"
#include "logger.h"
#include "window.h"
#include "proto-encoder.h"

/**
 * @brief Print the constants the server is running with
 *
 */
void print_constants()
{
    printf("The server is running with the following parameters:\n");
    printf("WINDOW_SIZE: %d\n", WINDOW_SIZE);
    printf("ROACH_MOVE_CHANCE: %d\n", ROACH_MOVE_CHANCE);
    printf("WASP_MOVE_CHANCE: %d\n", WASP_MOVE_CHANCE);
    printf("ROACH_MOVE_DELAY: %d\n", ROACH_MOVE_DELAY);
    printf("WASP_MOVE_DELAY: %d\n", WASP_MOVE_DELAY);
    printf("MAX_ROACH_SCORE: %d\n", MAX_ROACH_SCORE);
    printf("MAX_ROACHES_GENERATED: %d\n", MAX_ROACHES_GENERATED);
    printf("MAX_WASPS_GENERATED: %d\n", MAX_WASPS_GENERATED);
    printf("MAX_SLOTS_ALLOWED: %d\n", MAX_SLOTS_ALLOWED);
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

    int curve_server = 1;
    zmq_setsockopt(*responder, ZMQ_CURVE_SERVER, &curve_server, sizeof(curve_server));

    if (zmq_setsockopt(*responder, ZMQ_CURVE_SECRETKEY, SERVER_PRIVATE_KEY, 40) != 0)
    {
        printf("Failed to set secret key: %s\n", zmq_strerror(errno));
        zmq_close(*responder);
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
    zmq_setsockopt(*publisher, ZMQ_CURVE_SERVER, &curve_server, sizeof(curve_server));
    if (zmq_setsockopt(*publisher, ZMQ_CURVE_SECRETKEY, SERVER_PRIVATE_KEY, 40) != 0)
    {
        printf("Failed to set secret key: %s\n", zmq_strerror(errno));
        zmq_close(*publisher);
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
 * @brief - Serialize the window matrix, lizard mover, roach mover and wasp mover and send to the
 * connecting display app
 *
 * @param responder  - ZMQ socket
 * @param data - Window data
 * @param roach_mover - Roach mover
 * @param lizard_mover - Lizard mover
 * @param wasp_mover - Wasp mover
 */
void process_display_app_message(void *responder, window_data *data)
{

    // Create proto window data
    WindowDataProto *proto_window_data = malloc(sizeof(WindowDataProto));
    window_data_proto__init(proto_window_data);

    // Convert window data to proto window data
    window_data_to_proto_window_data(proto_window_data, data);

    // Serialize the proto window data
    size_t proto_window_data_size = window_data_proto__get_packed_size(proto_window_data);
    void *proto_window_data_buffer = malloc(proto_window_data_size);

    window_data_proto__pack(proto_window_data, proto_window_data_buffer);

    // Send the serialized data
    zmq_send(responder, proto_window_data_buffer, proto_window_data_size, 0);

    // Free the allocated memory
    free(proto_window_data_buffer);
    window_data_proto__free_unpacked(proto_window_data, NULL);
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
            } while (cell->stack[cell->top].client_id == LIZARD || cell->stack[cell->top].client_id == ROACH || cell->stack[cell->top].client_id == WASP);

            eaten_roach->x = new_x;
            eaten_roach->y = new_y;
            eaten_roach->is_eaten = 0;
            eaten_roach->timestamp = 0;
        }
    }
}

void remove_timeout_entities(lizard_mover *lizard_payload, roach_mover *roach_payload, wasp_mover *wasp_payload)
{
    int lizard_id;
    int roach_id;
    int wasp_id;
    time_t current_time = time(NULL);

    // Remove the timed out lizards
    for (int i = 0; i < MAX_LIZARDS_ALLOWED; i++)
    {
        lizard_id = i;

        if (lizard_payload->lizards[lizard_id].ch == (char)-1)
            continue;

        if (difftime(current_time, lizard_payload->lizards[lizard_id].last_message_time) >= CLIENT_TIMEOUT_SECONDS)
        {
            // Erase the lizard from the screen
            lizard_erase(lizard_payload, i);

            // Set the lizard character to -1 to indicate it's no longer in use
            lizard_payload->lizards[lizard_id].ch = -1;
            lizard_payload->lizards[lizard_id].x = -1;
            lizard_payload->lizards[lizard_id].y = -1;
            lizard_payload->lizards[lizard_id].score = -1;
            lizard_payload->lizards[lizard_id].is_winner = -1;
            lizard_payload->lizards[lizard_id].previous_direction = -1;
            lizard_payload->lizards[lizard_id].last_message_time = -1;

            (*(lizard_payload->num_lizards))--;
            (*(lizard_payload->slot_lizards))++;
        }
    }

    // Remove the timed out roaches
    for (int i = 0; i < MAX_SLOTS_ALLOWED; i++)
    {
        roach_id = i;

        if (roach_payload->roaches[roach_id].ch == (char)-1)
            continue;

        if (difftime(current_time, roach_payload->roaches[roach_id].last_message_time) >= CLIENT_TIMEOUT_SECONDS)
        {
            // Erase the roach from the screen
            window_erase(roach_payload->game_window, roach_payload->roaches[roach_id].x, roach_payload->roaches[roach_id].y, (roach_payload->roaches[roach_id].ch + '0') | A_BOLD);

            // Set the roach character to -1 to indicate it's no longer in use
            roach_payload->roaches[roach_id].ch = -1;
            roach_payload->roaches[roach_id].x = -1;
            roach_payload->roaches[roach_id].y = -1;
            roach_payload->roaches[roach_id].is_eaten = -1;
            roach_payload->roaches[roach_id].timestamp = -1;
            roach_payload->roaches[roach_id].last_message_time = -1;

            (*(roach_payload->num_roaches))--;
            (*(roach_payload->slot_roaches))++;
        }
    }

    // Remove the timed out wasps
    for (int i = 0; i < MAX_SLOTS_ALLOWED; i++)
    {
        wasp_id = i;

        if (wasp_payload->wasps[wasp_id].ch == (char)-1)
            continue;

        if (difftime(current_time, wasp_payload->wasps[wasp_id].last_message_time) >= CLIENT_TIMEOUT_SECONDS)
        {
            // Erase the wasp from the screen
            window_erase(wasp_payload->game_window, wasp_payload->wasps[wasp_id].x, wasp_payload->wasps[wasp_id].y, (wasp_payload->wasps[wasp_id].ch) | A_BOLD);

            // Set the wasp character to -1 to indicate it's no longer in use
            wasp_payload->wasps[wasp_id].ch = -1;
            wasp_payload->wasps[wasp_id].x = -1;
            wasp_payload->wasps[wasp_id].y = -1;
            wasp_payload->wasps[wasp_id].last_message_time = -1;

            (*(wasp_payload->num_wasps))--;
            (*(wasp_payload->slot_wasps))++;
        }
    }
}

void send_updated_cells(void *publisher, window_data *game_window, lizard_mover *lizard_payload)
{

    log_write("Sending updated cells to clients\n");
    log_write("Size of updated cells: %d\n", game_window->size_of_updated_cells);
    // Create field update message
    field_update field_update_message;
    field_update_message.updated_cell_indexes = game_window->updated_cell_indexes;
    field_update_message.size_of_updated_cells = game_window->size_of_updated_cells;

    // Copy the updated cells to a new array in the field update message
    // Allocate memory for the updated cells
    field_update_message.updated_cells = (layer_cell *)malloc(sizeof(layer_cell) * game_window->size_of_updated_cells);

    // Copy the updated cells from the updated_cell_indexes to the updated_cells array
    for (int i = 0; i < game_window->size_of_updated_cells; i++)
    {
        field_update_message.updated_cells[i] = game_window->matrix->cells[game_window->updated_cell_indexes[i]];
    }

    field_update_message.size_of_scores = *lizard_payload->num_lizards;
    field_update_message.scores = (scores_update *)malloc(sizeof(scores_update) * field_update_message.size_of_scores);

    // Copy the scores from the lizard mover to the field update message
    for (int i = 0; i < field_update_message.size_of_scores; i++)
    {
        field_update_message.scores[i].score = lizard_payload->lizards[i].score;
        field_update_message.scores[i].ch = lizard_payload->lizards[i].ch;
    }

    // --- Start of proto encoding ---
    FieldUpdateProto *proto_field_update = malloc(sizeof(FieldUpdateProto));
    field_update_proto__init(proto_field_update);

    field_update_to_proto_field_update(proto_field_update, &field_update_message);

    size_t proto_field_update_size = field_update_proto__get_packed_size(proto_field_update);
    void *proto_field_update_buffer = malloc(proto_field_update_size);

    field_update_proto__pack(proto_field_update, proto_field_update_buffer);

    // --- End of proto encoding ---

    // Send the serialized data
    zmq_send(publisher, "field_update", strlen("field_update"), ZMQ_SNDMORE);
    zmq_send(publisher, proto_field_update_buffer, proto_field_update_size, 0);

    // Free the allocated memory
    free(proto_field_update_buffer);
    field_update_proto__free_unpacked(proto_field_update, NULL);

    // Clear the updated cells
    game_window->size_of_updated_cells = 0;
    memset(game_window->updated_cell_indexes, 0, sizeof(int) * game_window->size_of_updated_cells);
}

typedef struct RemoveTimeoutEntitiesArgs
{
    lizard_mover *lizard_payload;
    roach_mover *roach_payload;
    wasp_mover *wasp_payload;
} remove_timeout_entities_args;

// Threads for removing timed out entities
void *remove_timeout_entities_thread(void *args)
{
    remove_timeout_entities_args *arguments = (remove_timeout_entities_args *)args;

    // Run this function every second
    while (1)
    {
        sleep(1);
        remove_timeout_entities(arguments->lizard_payload, arguments->roach_payload, arguments->wasp_payload);
    }

    return NULL;
}

typedef struct RespawnEatenRoachesArgs
{
    roach_mover *roach_payload;
    roach **eaten_roaches;
    int *amount_eaten_roaches;
} respawn_eaten_roaches_args;
// Threads for respawning eaten roaches
void *respawn_eaten_roaches_thread(void *args)
{
    respawn_eaten_roaches_args *arguments = (respawn_eaten_roaches_args *)args;

    // Run this function every second
    while (1)
    {
        sleep(1);
        respawn_eaten_roaches(arguments->roach_payload, arguments->eaten_roaches, arguments->amount_eaten_roaches);
    }

    return NULL;
}

typedef struct SendUpdatedCellsArgs
{
    void *publisher;
    window_data *game_window;
    lizard_mover *lizard_payload;
} send_updated_cells_args;

// Threads for sending updated cells
void *send_updated_cells_thread(void *args)
{
    send_updated_cells_args *arguments = (send_updated_cells_args *)args;

    // Run this function every 10 milliseconds
    while (1)
    {
        usleep(10000);
        if (arguments->game_window->size_of_updated_cells > 0)
            send_updated_cells(arguments->publisher, arguments->game_window, arguments->lizard_payload);
    }

    return NULL;
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

    printf("Usage: ./lizardsNroachesNwasps-server <rep_server_address> <rep_server_port <pub_server_address> <pub_server_port>\n");

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

    // Creates a window and draws a border
    window_data *game_window;
    window_init(&game_window, WINDOW_SIZE, WINDOW_SIZE);

    // Create a new window for the scores
    WINDOW *score_window = newwin(MAX_LIZARDS_ALLOWED, 50, 0, WINDOW_SIZE + 2);

    // Initialize variables used for entity tracking
    int num_wasps = 0;
    int num_roaches = 0;
    int num_lizards = 0;
    int slots_available = MAX_SLOTS_ALLOWED;
    int slot_lizards = MAX_LIZARDS_ALLOWED;
    wasp wasps[MAX_SLOTS_ALLOWED];
    roach roaches[MAX_SLOTS_ALLOWED];
    lizard lizards[MAX_LIZARDS_ALLOWED];

    // Initialize the wasps array
    for (int i = 0; i < MAX_SLOTS_ALLOWED; i++)
    {
        wasps[i].ch = -1;
        wasps[i].x = -1;
        wasps[i].y = -1;
        wasps[i].last_message_time = -1;
    }

    // Initialize the roaches array
    for (int i = 0; i < MAX_SLOTS_ALLOWED; i++)
    {
        roaches[i].ch = -1;
        roaches[i].x = -1;
        roaches[i].y = -1;
        roaches[i].is_eaten = -1;
        roaches[i].timestamp = -1;
        roaches[i].last_message_time = -1;
    }

    // Initialize the lizards array
    for (int i = 0; i < MAX_LIZARDS_ALLOWED; i++)
    {
        lizards[i].ch = -1;
        lizards[i].x = -1;
        lizards[i].y = -1;
        lizards[i].score = -1;
        lizards[i].is_winner = -1;
        lizards[i].previous_direction = -1;
        lizards[i].last_message_time = -1;
    }

    message_to_server recv_message;

    wasp_mover *wasp_payload = malloc(sizeof(wasp_mover));
    log_write("Creating wasp mover\n");
    new_wasp_mover(&wasp_payload, &recv_message, wasps, responder, &num_wasps, &slots_available, game_window);
    wasp_payload->should_use_responder = 1;

    roach_mover *roach_payload = malloc(sizeof(roach_mover));
    log_write("Creating roach mover\n");
    new_roach_mover(&roach_payload, &recv_message, roaches, responder, &num_roaches, &slots_available, game_window);
    roach_payload->should_use_responder = 1;

    lizard_mover *lizard_payload = malloc(sizeof(lizard_mover));
    log_write("Creating lizard mover\n");
    new_lizard_mover(&lizard_payload, &recv_message, lizards, responder, &num_lizards, &slot_lizards, game_window);
    lizard_payload->should_use_responder = 1;

    wasp_payload->wasps = wasps;
    roach_payload->lizards = lizards;
    lizard_payload->roaches = roaches;

    // Create pointer to eaten roaches
    roach **eaten_roaches = (roach **)malloc(sizeof(roach *) * MAX_SLOTS_ALLOWED);
    int eaten_roaches_count = 0;

    roach_payload->eaten_roaches = eaten_roaches;
    roach_payload->amount_eaten_roaches = &eaten_roaches_count;
    lizard_payload->eaten_roaches = eaten_roaches;
    lizard_payload->amount_eaten_roaches = &eaten_roaches_count;

    zmq_msg_t message;

    pthread_t remove_timeout_entities_thread_t;
    pthread_t respawn_eaten_roaches_thread_t;
    pthread_t send_updated_cells_thread_t;

    remove_timeout_entities_args remove_timeout_entities_arguments;
    remove_timeout_entities_arguments.lizard_payload = lizard_payload;
    remove_timeout_entities_arguments.roach_payload = roach_payload;
    remove_timeout_entities_arguments.wasp_payload = wasp_payload;

    respawn_eaten_roaches_args respawn_eaten_roaches_arguments;
    respawn_eaten_roaches_arguments.roach_payload = roach_payload;
    respawn_eaten_roaches_arguments.eaten_roaches = eaten_roaches;
    respawn_eaten_roaches_arguments.amount_eaten_roaches = &eaten_roaches_count;

    send_updated_cells_args send_updated_cells_arguments;
    send_updated_cells_arguments.publisher = publisher;
    send_updated_cells_arguments.game_window = game_window;
    send_updated_cells_arguments.lizard_payload = lizard_payload;

    // Create the threads for removing timed out entities and respawning eaten roaches
    pthread_create(&remove_timeout_entities_thread_t, NULL, remove_timeout_entities_thread, &remove_timeout_entities_arguments);

    pthread_create(&respawn_eaten_roaches_thread_t, NULL, respawn_eaten_roaches_thread, &respawn_eaten_roaches_arguments);

    pthread_create(&send_updated_cells_thread_t, NULL, send_updated_cells_thread, &send_updated_cells_arguments);

    while (1)
    {
        log_write("Waiting for message from client\n");
        // Receive message from one of the clients
        zmq_msg_init(&message);
        // Receive a message part from the socket
        if (zmq_msg_recv(&message, responder, 0) == -1)
        {
            // Handle error - for example, log and break/continue
            log_write("Error receiving message: %s\n", zmq_strerror(zmq_errno()));
            zmq_msg_close(&message);
            exit(EXIT_FAILURE);
        }
        log_write("Received message from client\n");
        // Get the size of the received message
        size_t msg_size = zmq_msg_size(&message);
        void *msg_data = zmq_msg_data(&message);

        // Unpack the Protobuf message
        MessageToServerProto *msg = message_to_server_proto__unpack(NULL, msg_size, msg_data);
        if (msg == NULL)
        {
            // Handle error - for example, log and continue
            log_write("Error unpacking received message\n");
            zmq_msg_close(&message);
            exit(EXIT_FAILURE);
        }

        // Convert the message to the server to the message to the server
        proto_message_to_server_to_message_to_server(msg, &recv_message);

        // Free the message to the server proto
        message_to_server_proto__free_unpacked(msg, NULL);
        zmq_msg_close(&message);

        log_write("Received message from client %d\n", recv_message.client_id);

        // Respawn the eaten roaches
        respawn_eaten_roaches(roach_payload, eaten_roaches, &eaten_roaches_count);

        // Print the scores in the score window
        int i, j;
        for (i = 0, j = 0; j < num_lizards; i++)
        {
            if (lizards[i].ch == (char)-1)
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

        switch (recv_message.client_id)
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
        case WASP:
            log_write("Processing wasp message\n");
            process_wasp_message(wasp_payload);
            break;
        // This will still remain to process the display app message from the lizard-client
        case DISPLAY_APP:
            log_write("Processing display app message\n");
            process_display_app_message(responder, game_window);
            log_write("Processed display app message\n");
            break;
        default:
            break;
        }
    }

    endwin();

    zmq_close(responder);
    zmq_ctx_destroy(context);

    // Free the allocated memory

    free(wasp_payload);
    free(roach_payload);
    free(lizard_payload);

    // Join the threads

    pthread_join(remove_timeout_entities_thread_t, NULL);
    pthread_join(respawn_eaten_roaches_thread_t, NULL);
    pthread_join(send_updated_cells_thread_t, NULL);

    free(eaten_roaches);
    log_close();

    return 0;
}
