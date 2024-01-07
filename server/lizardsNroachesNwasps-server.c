#include "default_consts.h"
#include "wasp-mover.h"
#include "roach-mover.h"
#include "lizard-mover.h"
#include "logger.h"
#include "window.h"
#include "proto-encoder.h"

pthread_mutex_t mutex;

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
int create_and_connect_sockets(char *router_server_socket_address, char *pub_server_socket_address, void **context, void **router_socket, void **publisher)
{
    // Create context
    *context = zmq_ctx_new();
    if (*context == NULL)
    {
        printf("Failed to create context: %s\n", zmq_strerror(errno));
        return -1;
    }

    // Create REQ socket to receive messages from clients
    *router_socket = zmq_socket(*context, ZMQ_ROUTER);
    if (*router_socket == NULL)
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
        zmq_close(*router_socket);
        zmq_ctx_destroy(*context);
        return -1;
    }

    // Bind to the REP socket
    if (zmq_bind(*router_socket, router_server_socket_address) != 0)
    {
        printf("Failed to bind REP socket: %s\n", zmq_strerror(errno));
        zmq_close(*router_socket);
        zmq_close(*publisher);
        zmq_ctx_destroy(*context);
        return -1;
    }

    // Bind to the PUB socket
    if (zmq_bind(*publisher, pub_server_socket_address) != 0)
    {
        printf("Failed to bind PUB socket: %s\n", zmq_strerror(errno));
        zmq_close(*router_socket);
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

typedef struct LizardWorkerArgs
{
    void *context;
    lizard_mover *lizard_payload;
} LizardWorkerArgs;
void *lizard_worker_thread(void *args)
{
    log_write("Starting lizard_worker_thread\n");

    LizardWorkerArgs *lizard_args = (LizardWorkerArgs *)args;
    void *context = lizard_args->context;
    lizard_mover *lizard_payload = lizard_args->lizard_payload;

    void *dealer = zmq_socket(context, ZMQ_REP);
    int rc = zmq_connect(dealer, "inproc://lizard_backend");
    if (rc != 0)
    {
        log_write("Error connecting to lizard_backend: %s\n", zmq_strerror(zmq_errno()));
        exit(EXIT_FAILURE);
    }

    zmq_msg_t identity;
    zmq_msg_t payload;

    while (1)
    {
        log_write("Waiting for message from client\n");

        // zmq_msg_init(&identity);
        // if (zmq_msg_recv(&identity, dealer, 0) == -1)
        // {
        //     log_write("Error receiving identity on lizard_worker: %s\n", zmq_strerror(zmq_errno()));
        //     zmq_msg_close(&identity);
        //     exit(EXIT_FAILURE);
        // }

        zmq_msg_init(&payload);
        if (zmq_msg_recv(&payload, dealer, 0) == -1)
        {
            log_write("Error receiving payload on lizard_worker: %s\n", zmq_strerror(zmq_errno()));
            zmq_msg_close(&identity);
            zmq_msg_close(&payload);
            exit(EXIT_FAILURE);
        }

        log_write("Received message, unpacking\n");

        MessageToServerProto *msg = message_to_server_proto__unpack(NULL, zmq_msg_size(&payload), zmq_msg_data(&payload));
        if (msg == NULL)
        {
            log_write("Error unpacking received message\n");
            zmq_msg_close(&identity);
            zmq_msg_close(&payload);
            exit(EXIT_FAILURE);
        }

        message_to_server *recv_message = malloc(sizeof(message_to_server));
        proto_message_to_server_to_message_to_server(msg, recv_message);

        log_write("Message unpacked, processing\n");

        pthread_mutex_lock(&mutex);

        lizard_payload->responder = dealer;
        lizard_payload->recv_message = recv_message;

        log_write("Processing lizard message\n");
        process_lizard_message(lizard_payload);
        log_write("Processed lizard message\n");

        lizard_payload->identity = NULL;
        lizard_payload->identity_size = 0;

        pthread_mutex_unlock(&mutex);

        log_write("Message processed, closing\n");

        zmq_msg_close(&payload);
        free(recv_message);
        message_to_server_proto__free_unpacked(msg, NULL);
    }

    log_write("Closing lizard_worker_thread\n");
    zmq_close(dealer);
}

typedef struct RestWorkerArgs
{
    void *context;
    roach_mover *roach_payload;
    wasp_mover *wasp_payload;
} RestWorkerArgs;
void *rest_worker_thread(void *args)
{
    // Convert the args to the correct type
    RestWorkerArgs *rest_args = (RestWorkerArgs *)args;
    void *context = rest_args->context;
    roach_mover *roach_payload = rest_args->roach_payload;
    wasp_mover *wasp_payload = rest_args->wasp_payload;

    void *dealer = zmq_socket(context, ZMQ_REP);
    zmq_connect(dealer, "inproc://rest_backend");

    zmq_msg_t identity;
    zmq_msg_t payload;

    while (1)
    {
        // Receive message from one of the clients
        // zmq_msg_init(&identity);
        // // Receive a message part from the socket
        // if (zmq_msg_recv(&identity, dealer, 0) == -1)
        // {
        //     log_write("Error receiving identity on rest_worker: %s\n", zmq_strerror(zmq_errno()));
        //     zmq_msg_close(&identity);
        //     exit(EXIT_FAILURE);
        // }

        zmq_msg_init(&payload);
        // Receive a message part from the socket
        if (zmq_msg_recv(&payload, dealer, 0) == -1)
        {
            log_write("Error receiving payload on rest_worker: %s\n", zmq_strerror(zmq_errno()));
            zmq_msg_close(&identity);
            zmq_msg_close(&payload);
            exit(EXIT_FAILURE);
        }

        // Parse the payload into a message to server struct
        // -- Start of proto decoding --
        MessageToServerProto *msg = message_to_server_proto__unpack(NULL, zmq_msg_size(&payload), zmq_msg_data(&payload));
        if (msg == NULL)
        {
            log_write("Error unpacking received message\n");
            zmq_msg_close(&identity);
            zmq_msg_close(&payload);
            exit(EXIT_FAILURE);
        }

        // -- End of proto decoding --

        // Convert the message to server proto to a message to server struct
        message_to_server *recv_message = malloc(sizeof(message_to_server));
        proto_message_to_server_to_message_to_server(msg, recv_message);

        // Start mutex lock
        pthread_mutex_lock(&mutex);

        roach_payload->responder = dealer;
        roach_payload->recv_message = recv_message;
        wasp_payload->responder = dealer;
        wasp_payload->recv_message = recv_message;

        switch (recv_message->client_id)
        {
        case ROACH:
            // Update the roach mover
            log_write("Updating roach mover\n");
            process_roach_message(roach_payload);
            log_write("Updated roach mover\n");
            break;
        case WASP:
            log_write("Updating wasp mover\n");
            process_wasp_message(wasp_payload);
            log_write("Updated wasp mover\n");
            break;
        case DISPLAY_APP:
            log_write("Updating display app\n");
            process_display_app_message(dealer, roach_payload->game_window);
            log_write("Updated display app\n");
            break;
        default:
            break;
        }

        // End mutex lock
        pthread_mutex_unlock(&mutex);

        // CLose
        zmq_msg_close(&identity);
        zmq_msg_close(&payload);
        free(recv_message);
        message_to_server_proto__free_unpacked(msg, NULL);
    }
}

int extract_client_id_from_message(const char *buffer, size_t buffer_size)
{
    const uint8_t *unsigned_buffer = (const uint8_t *)buffer;

    MessageToServerProto *msg = message_to_server_proto__unpack(NULL, buffer_size, unsigned_buffer);
    if (msg == NULL)
    {
        // Handle error - for example, log and continue
        log_write("Error unpacking received message\n");
        return -1; // Indicate an error
    }

    int client_id = msg->client_id;
    message_to_server_proto__free_unpacked(msg, NULL);
    return client_id;
}

typedef struct ProxyThreadArgs
{
    void *frontend;
    void *backend;
} ProxyThreadArgs;

void *proxy_thread_func(void *args)
{
    ProxyThreadArgs *proxy_args = (ProxyThreadArgs *)args;
    void *frontend = proxy_args->frontend;
    void *backend = proxy_args->backend;

    zmq_proxy(frontend, backend, NULL);

    return NULL;
}

int main(int argc, char *argv[])
{
    pthread_mutex_init(&mutex, NULL);
    // Print the parameters the server is running with
    print_constants();

    // Initialize logger
    log_init("server.log");

    void *context;
    void *publisher;
    void *router_socket;
    void *lizard_backend;
    void *rest_backend;

    char *rep_server_socket_address;
    char *pub_server_socket_address;
    char *router_server_socket_address;

    printf("Usage: ./lizardsNroachesNwasps-server <rep_server_address> <rep_server_port <pub_server_address> <pub_server_port>\n");

    // Check if address and port were provided as command line arguments, if not, use default values
    if (argc != 5)
    {
        printf("Addresses and ports were not provided!\n");

        // Todo remove
        printf("Using default REP server socket address: %s\n", DEFAULT_SERVER_SOCKET_ADDRESS);
        rep_server_socket_address = malloc(strlen(DEFAULT_SERVER_SOCKET_ADDRESS) + 1);
        rep_server_socket_address = DEFAULT_SERVER_SOCKET_ADDRESS;
        // end todo remove

        printf("Using default REP server socket address: %s\n", DEFAULT_SERVER_SOCKET_ADDRESS);
        router_server_socket_address = malloc(strlen(DEFAULT_SERVER_SOCKET_ADDRESS) + 1);
        router_server_socket_address = DEFAULT_SERVER_SOCKET_ADDRESS;

        printf("Using default PUB server socket address: %s\n", DEFAULT_PUBLISH_SERVER_SOCKET_ADDRESS);
        pub_server_socket_address = malloc(strlen(DEFAULT_PUBLISH_SERVER_SOCKET_ADDRESS) + 1);
        pub_server_socket_address = DEFAULT_PUBLISH_SERVER_SOCKET_ADDRESS;
    }
    else
    {
        printf("Using REP address and port: %s %s\n", argv[1], argv[2]);
        char *rep_address = argv[1];
        char *rep_port = argv[2];
        // todo remove
        rep_server_socket_address = malloc(strlen("tcp://") + strlen(rep_address) + strlen(":") + strlen(rep_port) + 1);
        strcpy(rep_server_socket_address, "tcp://");
        strcat(rep_server_socket_address, rep_address);
        strcat(rep_server_socket_address, ":");
        strcat(rep_server_socket_address, rep_port);
        // end todo remove

        router_server_socket_address = malloc(strlen("tcp://") + strlen(rep_address) + strlen(":") + strlen(rep_port) + 1);
        strcpy(router_server_socket_address, "tcp://");
        strcat(router_server_socket_address, rep_address);
        strcat(router_server_socket_address, ":");
        strcat(router_server_socket_address, rep_port);

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
    if (create_and_connect_sockets(router_server_socket_address, pub_server_socket_address, &context, &router_socket, &publisher) != 0)
        return -1;

    // add a backend socket to the context
    lizard_backend = zmq_socket(context, ZMQ_DEALER);
    zmq_bind(lizard_backend, "inproc://lizard_backend");

    rest_backend = zmq_socket(context, ZMQ_DEALER);
    zmq_bind(rest_backend, "inproc://rest_backend");

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
    new_wasp_mover(&wasp_payload, &recv_message, wasps, &num_wasps, &slots_available, game_window);
    wasp_payload->should_use_responder = 1;

    roach_mover *roach_payload = malloc(sizeof(roach_mover));
    log_write("Creating roach mover\n");
    new_roach_mover(&roach_payload, &recv_message, roaches, &num_roaches, &slots_available, game_window);
    roach_payload->should_use_responder = 1;

    lizard_mover *lizard_payload = malloc(sizeof(lizard_mover));
    log_write("Creating lizard mover\n");
    new_lizard_mover(&lizard_payload, &recv_message, lizards, &num_lizards, &slot_lizards, game_window);
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

    zmq_msg_t identity;
    zmq_msg_t payload;
    // - Start definition of threads for client handling -

    pthread_t lizard_workers[4];
    // Create args for the lizard worker threads
    LizardWorkerArgs *lizard_args = malloc(sizeof(LizardWorkerArgs) * 4);
    for (int i = 0; i < 4; i++)
    {
        lizard_args[i].context = context;
        lizard_args[i].lizard_payload = lizard_payload;
        pthread_create(&lizard_workers[i], NULL, lizard_worker_thread, lizard_args);
    }

    pthread_t rest_worker;
    // Create args for the rest worker thread
    RestWorkerArgs *rest_args = malloc(sizeof(RestWorkerArgs));
    rest_args->context = context;
    rest_args->roach_payload = roach_payload;
    rest_args->wasp_payload = wasp_payload;
    pthread_create(&rest_worker, NULL, rest_worker_thread, rest_args);

    pthread_t proxy_thread_lizard;
    pthread_t proxy_thread_rest;

    ProxyThreadArgs *proxy_args_lizard = malloc(sizeof(ProxyThreadArgs));
    proxy_args_lizard->frontend = router_socket;
    proxy_args_lizard->backend = lizard_backend;
    pthread_create(&proxy_thread_lizard, NULL, proxy_thread_func, proxy_args_lizard);

    ProxyThreadArgs *proxy_args_rest = malloc(sizeof(ProxyThreadArgs));
    proxy_args_rest->frontend = router_socket;
    proxy_args_rest->backend = rest_backend;
    pthread_create(&proxy_thread_rest, NULL, proxy_thread_func, proxy_args_rest);

    while (1)
    {
        log_write("Starting main loop\n");

        // Remove the timed out entities
        log_write("Removing timed out entities\n");
        remove_timeout_entities(lizard_payload, roach_payload, wasp_payload); // TODO: Add thread here

        // Receive message from one of the clients
        zmq_msg_init(&identity);
        // Receive a message part from the socket
        log_write("Receiving identity from client\n");
        if (zmq_msg_recv(&identity, router_socket, 0) == -1)
        {
            log_write("Error receiving identity on main: %s\n", zmq_strerror(zmq_errno()));
            zmq_msg_close(&identity);
            exit(EXIT_FAILURE);
        }

        zmq_msg_init(&payload);
        // Receive a message part from the socket
        log_write("Receiving payload from client\n");
        if (zmq_msg_recv(&payload, router_socket, 0) == -1)
        {
            log_write("Error receiving payload on main: %s\n", zmq_strerror(zmq_errno()));
            zmq_msg_close(&identity);
            zmq_msg_close(&payload);
            exit(EXIT_FAILURE);
        }
        log_write("Received payload from client\n");

        log_write("Extracting client id from message\n");
        int client_id = extract_client_id_from_message(zmq_msg_data(&payload), zmq_msg_size(&payload));
        log_write("Client id: %d\n", client_id);

        if (client_id == -1)
        {
            log_write("Error extracting client id from message\n");
            zmq_msg_close(&identity);
            zmq_msg_close(&payload);
            exit(EXIT_FAILURE);
        }

        if (client_id == LIZARD)
        {
            log_write("Received lizard message\n");
            // zmq_msg_send(&identity, lizard_backend, ZMQ_SNDMORE);
            zmq_msg_send(&payload, lizard_backend, 0);
            log_write("Sent lizard message\n");
        }
        else
        {
            log_write("Received rest message\n");
            log_write("Client id: %d\n", client_id);
            // zmq_msg_send(&identity, rest_backend, ZMQ_SNDMORE);
            zmq_msg_send(&payload, rest_backend, 0);
            log_write("Sent rest message\n");
        }

        // Print the scores in the score window
        log_write("Printing scores in score window\n");
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
        log_write("Clearing remaining lines in score window\n");
        for (; j < MAX_LIZARDS_ALLOWED; j++)
        {
            wmove(score_window, j, 0);
            wclrtoeol(score_window);
        }

        // Update the score window
        log_write("Updating score window\n");
        wrefresh(score_window);

        // Respawn the eaten roaches
        log_write("Respawning eaten roaches\n");
        respawn_eaten_roaches(roach_payload, eaten_roaches, &eaten_roaches_count); // TODO: Add thread here

        // Check for updated cells and updated scores
        if (game_window->size_of_updated_cells > 0)
        {
            // Send the updated cells to the clients
            log_write("Sending updated cells to clients\n");
            send_updated_cells(publisher, game_window, lizard_payload);
            log_write("Sent updated cells to clients\n");
        }

        // Close
        log_write("Closing identity and payload messages\n");
        zmq_msg_close(&identity);
        zmq_msg_close(&payload);

        log_write("Ending main loop\n");
    }

    endwin();

    zmq_ctx_destroy(context);
    zmq_close(router_socket);
    zmq_close(lizard_backend);
    zmq_close(rest_backend);

    // Join threads
    for (int i = 0; i < 4; i++)
    {
        pthread_join(lizard_workers[i], NULL);
    }

    pthread_join(rest_worker, NULL);
    // Free the allocated memory
    free(proxy_args_lizard);
    free(proxy_args_rest);

    free(eaten_roaches);
    log_close();
    pthread_mutex_destroy(&mutex);

    return 0;
}
