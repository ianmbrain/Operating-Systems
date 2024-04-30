/** 
    @file client.c
    @author Ian M Brain (imbrain)
    This program acts as a client for the server.c client.c application.
    Allows users to interact with the server to play the peg-jumping game.
    Allows movement commands, undo commands, and show commands.
    Interacts with server.c through a message queue.
*/

#include "common.h"
#include <mqueue.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Print out an error message and exit.
/** Citing Help from other Assignments
 * The code for this method is based on the fail() method of server.c that was provided in this project.
*/
static void fail( char const *message ) {
  fprintf( stderr, "%s\n", message );
  exit( 1 );
}

/**
 * Main functionality of the client.
 * Sends messages to the server through message queue.
 * Receives responses from the server through same queue.
 * @param argc number of arguments passed
 * @param argv array of passed arguments
 * @return program exit status
*/
int main( int argc, char *argv[] ) {
    /** Citing Help from other Assignments
     * The code for this message queue is based on the message queue code from server.c that was provided in this project.
     * Prepare structure indicating maximum queue and message sizes.
    */
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 1;
    attr.mq_msgsize = MESSAGE_LIMIT;

    // Client message to send to the server.
    char clientSend[ 8 + 1 ] = "";
    // Client message received from the server.
    char clientReceive[ MESSAGE_LIMIT ] = "";

    // Open the client and server queues.
    mqd_t clientQueue = mq_open( CLIENT_QUEUE, O_RDONLY, 0600, &attr );
    mqd_t serverQueue = mq_open( SERVER_QUEUE, O_WRONLY, 0600, &attr );
    
    // Print an error message and exit as failure if either message queue cannot be opened.
    if ( clientQueue == -1 || serverQueue == -1 )
        fail( "Can't open a message queue" );

    // Print error and exit if command line arguments are invalid.
    if ( argc < 2 || argc > 4 ) 
        fail( "error" );

    // Right, left, up, or down move command.
    if ( strcmp( argv[ 1 ], "left" ) == 0 || strcmp( argv[ 1 ], "right" ) == 0 || strcmp( argv[ 1 ], "up" ) == 0 || strcmp( argv[ 1 ], "down" ) == 0 ) {
        // Fail if there are an incorrect number of command line arguments.
        if ( argc != 4 ) 
            fail( "error" );

        // Create the client request.
        strcat( clientSend, argv[ 1 ] );
        strcat( clientSend, " " );
        strcat( clientSend, argv[ 2 ] );
        strcat( clientSend, " " );
        strcat( clientSend, argv[ 3 ] );
        strcat( clientSend, "\0" );

        // Send the client request to the server.
        mq_send( serverQueue, clientSend, strlen( clientSend ), 0 );

        // Receive the message from the server.
        int len = mq_receive( clientQueue, clientReceive, sizeof( clientReceive ), NULL );
        // Fail if the server message is not received.
        if ( len <= 0 )
            fail( "Unable to receive server message." );

        // Print the server success or error message.
        printf( "%s\n", clientReceive );

        return EXIT_SUCCESS;
    }
    // Perform the undo command that undoes the most recent move.
    else if ( strcmp( argv[ 1 ], "undo" ) == 0 ) {
        // Send the client request to the server.
        mq_send( serverQueue, argv[ 1 ], strlen( argv[ 1 ] ), 0 );

        // Receive the message from the server.
        int len = mq_receive( clientQueue, clientReceive, sizeof( clientReceive ), NULL );
        // Fail if the server message is not received.
        if ( len <= 0 )
            fail( "Unable to receive server message." );

        // Print the server success or error message.
        printf( "%s\n", clientReceive );
    }
    // Perform the show command that prints out the puzzle board.
    else if ( strcmp( argv[ 1 ], "show" ) == 0 ) {
        // Send the client request to the server.
        mq_send( serverQueue, argv[ 1 ], strlen( argv[ 1 ] ), 0 );

        // Receive the message from the server.
        int len = mq_receive( clientQueue, clientReceive, sizeof( clientReceive ), NULL );
        // Fail if the server message is not received.
        if ( len < 0 )
            fail( "Unable to receive server message." );

        // Print the server success or error message.
        printf( "%s", clientReceive );
    }
    // Print error and exit the program if the command is invalid.
    else {
        fail( "error" );
    }

    // Close the client queue when after finishing
    mq_close( clientQueue );
}
