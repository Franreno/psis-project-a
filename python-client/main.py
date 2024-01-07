import struct
import zmq
from curses import wrapper
import curses
from consts import *
from server_messages_pb2 import MessageToServerProto
from window_data_pb2 import WindowDataProto

# Constants
LIZARD = 0
DISPLAY_APP = 3
CONNECT = 0
MOVEMENT = 1
DISCONNECT = 2
UP = 0
DOWN = 1
LEFT = 2
RIGHT = 3

def connect_lizard(requester):
    print("Starting connect_lizard function")

    send_message = MessageToServerProto()
    send_message.client_id = LIZARD
    send_message.type = CONNECT
    send_message.value = CONNECT

    print("Attempting to connect lizard")

    # Serialize the message
    print("Serializing the message")
    message_to_server_proto_buffer = send_message.SerializeToString()

    # Send the message
    print("Sending the message")
    try:
        requester.send(message_to_server_proto_buffer, zmq.NOBLOCK)
    except zmq.Again:
        print("Failed to send message! The operation would block")
        return -1

    # Server replies with either failure or the assigned lizard id
    print("Receiving server reply")
    lizard_id_bytes = requester.recv()
    lizard_id = struct.unpack('<i', lizard_id_bytes)[0]  # '>I' for big-endian unsigned int

    if int(lizard_id) < 0:
        print("Failed to connect lizard! No more slots available")
        return -1
    print("Lizard connected with id: %d" % lizard_id)

    print("Ending connect_lizard function")

    return lizard_id

def move_lizard(requester, lizard_id, stdscr):
    stdscr.nodelay(False)
    stop = False
    lizard_score = 0

    send_message = MessageToServerProto()
    send_message.client_id = LIZARD
    send_message.type = MOVEMENT
    send_message.value = lizard_id

    while not stop:
        keypress = stdscr.getch()

        # Check if the character is an arrow key, 'q' or 'Q'
        if keypress == curses.KEY_UP:
            send_message.direction = UP
        elif keypress == curses.KEY_DOWN:
            send_message.direction = DOWN
        elif keypress == curses.KEY_LEFT:
            send_message.direction = LEFT
        elif keypress == curses.KEY_RIGHT:
            send_message.direction = RIGHT
        elif keypress in [ord('q'), ord('Q')]:
            stop = True
            continue

        # Serialize the message
        print("Serializing the message")
        message_to_server_proto_buffer = send_message.SerializeToString()

        # Send the message
        print("Sending the message")
        requester.send(message_to_server_proto_buffer)

        # Server replies with either failure or the assigned lizard id
        server_reply_bytes = requester.recv()
        server_reply = struct.unpack('<i', server_reply_bytes)[0]  # '>I' for big-endian unsigned int
        

        if server_reply == 404:
            curses.endwin()
            print("Lizard %d disconnected from server" % lizard_id)
            return -1
        
        lizard_score = int(server_reply)

        # Clear the screen
        stdscr.clear()

        # Print the lizard's score
        stdscr.addstr(0, 0, "Lizard %c scored %d points" % (chr(lizard_id + 48 + 17), lizard_score))


    curses.endwin()

    print("Ending move_lizard function")
    print("Lizard %d scored %d points" % (lizard_id, lizard_score))

    return 0

def disconnect_lizard(lizard_id, requester):
    print("Starting disconnect_lizard function")

    # Prepare the message to send to the server
    send_message = MessageToServerProto()
    send_message.client_id = LIZARD  # Assuming LIZARD is a defined constant
    send_message.type = DISCONNECT  # Assuming DISCONNECT is a defined constant
    send_message.value = lizard_id

    # Serialize the message
    print("Serializing the message")
    message_to_server_proto_buffer = send_message.SerializeToString()

    # Send the serialized message to the server
    print("Sending disconnect request for lizard ID:", lizard_id)
    requester.send(message_to_server_proto_buffer)

    # Wait for the server's response
    print("Waiting for server response")
    server_reply = requester.recv()

    # Process the server's response
    if server_reply != b'0':  # Assuming a simple byte response indicating success or failure
        print("Failed to disconnect lizard!")
        return -1

    print("Lizard disconnected successfully")
    return 0

def main(stdscr):
    print("Starting main function")

    context = zmq.Context()
    requester = context.socket(zmq.REQ)
    requester.curve_secretkey = CLIENT_SECRET_KEY
    requester.curve_publickey = CLIENT_PUBLIC_KEY
    requester.curve_serverkey = SERVER_PUBLIC_KEY


    subscriber = context.socket(zmq.SUB)
    subscriber.curve_secretkey = CLIENT_SECRET_KEY
    subscriber.curve_publickey = CLIENT_PUBLIC_KEY
    subscriber.curve_serverkey = SERVER_PUBLIC_KEY

    print("Creating and connecting sockets")
    requester.connect(DEFAULT_SERVER_SOCKET_ADDRESS)
    subscriber.connect(DEFAULT_SUBS_SERVER_SOCKET_ADDRESS)

    print("Setting socket options")
    subscriber.setsockopt_string(zmq.SUBSCRIBE, "field_update")

    # Initialize ncurses
    print("Initializing ncurses")
    stdscr.nodelay(True)

    # Create lizard and connect it to the server
    print("Connecting lizard")
    lizard_id = connect_lizard(requester)
    if lizard_id < 0:
        requester.close()
        context.term()
        return -1

    move_lizard(requester, lizard_id, stdscr)

    # Disconnect lizard from the server
    print("Disconnecting lizard")
    disconnect_lizard(lizard_id, requester)

    # Close sockets and terminate context
    print("Closing sockets and terminating context")
    requester.close()
    subscriber.close()
    context.term()

    print("Ending main function")

if __name__ == "__main__":
    wrapper(main)