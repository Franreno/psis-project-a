# TODO Proj B

## Changes necessary

### Integration of Asian Wasps into the Game:

[] Implement the functionality for Asian wasps in the game, including their movement and interaction with lizards.

### Server Management Enhancements:

[] Update the server to manage the field with the inclusion of wasps.
[] Ensure the server can handle movement indications from different programs/clients (lizards, cockroaches, wasps).

### Modification of Lizard-Client Application:

[] Merge the applications that display the field with the lizard controllers.
[] Implement the display of the game field and scores within the Lizard-client.

### Development of Roaches-client and Wasps-client Applications:

[] Create applications for controlling cockroaches and wasps with random movements.
[] Ensure these applications can send movement instructions to the server.

### Removal of Display-app:

[] Remove the Display-app from the project.
[] Integrate the Display-app's functionalities into the Lizard-client.

### User Interfaces Update:

[] Update the user interfaces for Lizard-client, Roaches-client, and Wasps-client as required.
[] Implement NCurses interface for the Lizard-client to display the game field and scores.

### Life-cycle Management:

[] Implement the life-cycle processes for cockroaches and wasps, including creation, movement, and destruction.

### Client Disconnect Functionality:

[] Implement the handling of client disconnects, including disconnect messages and timeout management.

### Interoperability and Protocol Handling:

[] Ensure the interoperability between server and clients using Protocol Buffers for message encoding.
[] Consider implementing one of the clients (Wasps-client or Roaches-client) using a different programming language as a bonus.

### Error Treatment and Cheating Prevention:

Implement robust error validation and treatment to handle potential cheating attempts.
