// Name for the queue of messages going to the server.
#define SERVER_QUEUE "/imbrain-server-queue"

// Name for the queue of messages going to the current client.
#define CLIENT_QUEUE "/imbrain-client-queue"

// Maximum length for a message in the queue
// (Long enough to hold any server request or response)
#define MESSAGE_LIMIT 1024

// Height and width of the playing area.
#define GRID_SIZE 8

// Length of the undo history.
#define UNDO_SIZE 3

// Direction options for the move.  You're not required to use these
// now, but you will need them for a later assignment.
#define RIGHT_DIR 0
#define DOWN_DIR 1
#define LEFT_DIR 2
#define UP_DIR 3
