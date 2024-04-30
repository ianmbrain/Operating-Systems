/** 
    @file server.c
    @author Ian M Brain (imbrain)
    This program acts as a server for the server.c client.c application.
    Allows users to play a peg-jumping puzzle.
    Reads in puzzle board from provided file and performs client.c operations on it.
    Interacts with client.c through a message queue.
*/

#include "common.h"
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <signal.h>
#include <errno.h>
#include <string.h>

// Print out an error message and exit.
static void fail( char const *message ) {
  fprintf( stderr, "%s\n", message );
  exit( 1 );
}

// Flag for telling the server to stop running because of a sigint.
// This is safer than trying to print in the signal handler.
static volatile int running = 1;

/** Citing Help from the Course Slides
 * The code for this signal handler function is based on handler in lecture three we discussed on 1/18/24.
 * Handler method for determining if ctrl-c is inputted.
 * @param sig signal integer
*/
void ctrlCHander( int sig ) {
  running = 0;
}

/**
 * Main functionality of the server.
 * Reads in puzzle board from provided file.
 * Communicates with client through message queue to perform commands.
 * @param argc number of arguments passed
 * @param argv array of passed arguments
 * @return program exit status
*/
int main( int argc, char *argv[] ) {
  // Remove both queues, in case, last time, this program terminated
  // abnormally with some queued messages still queued.
  mq_unlink( SERVER_QUEUE );
  mq_unlink( CLIENT_QUEUE );

  // Eight by eight array representing the puzzle board.
  char puzzleBoard[ GRID_SIZE ][ GRID_SIZE + 1 ];

  // Array containing most recent three moves.
  char undoHistory[ UNDO_SIZE ][ MESSAGE_LIMIT ] = { };

  // The command line aruments are invalid if there are not two arguments.
  if ( argc != 2 )
    fail( "usage: server <puzzle-file>" );

  // Open the specified file.
  int fd = open( argv[ 1 ], O_RDONLY, 0600 );

  // Print error message and exit unsuccessfully if the file cannot be opened.
  if ( fd == -1 ) {
    char errorMessage[ MESSAGE_LIMIT + 20 ] = "";
    strcat( errorMessage, "Invalid input file: " );
    strcat( errorMessage, argv[ 1 ] );

    fail( errorMessage );
  }

  char readLine[ GRID_SIZE ] = "";

  int charsRead = 0;

  // Read the lines from the input file.
  for( int i = 0; i < 8; i++ ) {
    // Read in each of the characters into the buffer including the new line character.
    charsRead = read( fd, readLine, 9 );

    // Exit as failure if the file line does not contain nine characters
    if ( charsRead != ( GRID_SIZE + 1 ) ) {
      char errorMessage[ MESSAGE_LIMIT + 20 ] = "";
      strcat( errorMessage, "Invalid input file: " );
      strcat( errorMessage, argv[ 1 ] );

      fail( errorMessage );
    }

    // Exit as failure if the last character in the line is not a new line character.
    if ( readLine[ GRID_SIZE ] != '\n' ) {
      char errorMessage[ MESSAGE_LIMIT + 20 ] = "";
      strcat( errorMessage, "Invalid input file: " );
      strcat( errorMessage, argv[ 1 ] );

      fail( errorMessage );
    }

    for ( int j = 0; j < GRID_SIZE; j++ ) {
      // Exit as failure if the character is not a valid character.
      if ( readLine[ j ] != '.' && readLine[ j ] != 'o' && readLine[ j ] != '#' ) {
        char errorMessage[ MESSAGE_LIMIT + 20 ] = "";
        strcat( errorMessage, "Invalid input file: " );
        strcat( errorMessage, argv[ 1 ] );

        fail( errorMessage );
      }

      // Copy just the characters from the line into the puzzle board.
      puzzleBoard[ i ][ j ] = readLine[ j ];
    }

    puzzleBoard[ i ][ GRID_SIZE ] = '\n';
  }

  // Read the last line of the input file
  charsRead = read( fd, readLine, 1 );

  // Exit as failure if the last line contains any characters.
  if ( charsRead != 0 ) {
    char errorMessage[ MESSAGE_LIMIT + 20 ] = "";
    strcat( errorMessage, "Invalid input file: " );
    strcat( errorMessage, argv[ 1 ] );

    fail( errorMessage );
  }

  // Prepare structure indicating maximum queue and message sizes.
  struct mq_attr attr;
  attr.mq_flags = 0;
  attr.mq_maxmsg = 1;
  attr.mq_msgsize = MESSAGE_LIMIT;

  // Make both the server and client message queues.
  mqd_t serverQueue = mq_open( SERVER_QUEUE, O_RDONLY | O_CREAT, 0600, &attr );
  mqd_t clientQueue = mq_open( CLIENT_QUEUE, O_WRONLY | O_CREAT, 0600, &attr );
  if ( serverQueue == -1 || clientQueue == -1 )
    fail( "Can't create the needed message queues" );

  /** Citing Help from the Course Slides
   * The code for this signal handler operation is based on handler in lecture three we discussed on 1/18/24.
   * Set up the signal hander and link it with the associated method.
  */
  struct sigaction act;

  act.sa_handler = ctrlCHander;
  sigemptyset( &( act.sa_mask ) );
  act.sa_flags = 0;

  // Repeatedly read and process client messages.
  while ( running ) {
    // Signal action handler for the ctrl-c signal.
    sigaction( SIGINT, &act, 0 );

    // Message received from the client.
    char serverReceive[ MESSAGE_LIMIT ] = "";
    // Error message to send when there is an invalid command.
    char errorMessage[ 6 ] = "";
    strcpy( errorMessage, "error" );
    // Success message to send when there is a valid command.
    char successMessage[ 8 ] = "";
    strcpy( successMessage, "success" );

    // Receive the message sent from the client.
    int len = mq_receive( serverQueue, serverReceive, sizeof( serverReceive ), NULL );

    // Print the puzzle board and exit the server if the user passes ctrl-c.
    if ( len == -1 && errno == EINTR  ) {
      // String that the board message will be contained in.
      char boardMessage[ 73 ] = "";

      // Copy the puzzle board into the message
      strncpy( boardMessage, puzzleBoard[ 0 ], 9 );
      for ( int i = 1; i < GRID_SIZE; i++ ) {
        strncat( boardMessage, puzzleBoard[ i ], 9 );
      }

      // Ensure null terminator is at the end of the board message.
      strcat( boardMessage, "\0" );

      // Print out the puzzle board and exit the program.
      fprintf( stderr, "\n%s", boardMessage );
      exit( 0 );
    }

    // Fail and return error message if the server did not receive a message.
    if ( len < 0 ) {
      fail( "Unable to receive client message." );
    }

    // Perform the right move if the the right command is inputted.
    if( serverReceive[ 0 ] == 'r' ) {
      // Row of the peg to move.
      int row = serverReceive[ 6 ] - '0';
      // Column of the peg to move.
      int column = serverReceive[ 8 ] - '0';

      // A right command is invalid if the column is more than five or less than zero.
      if ( column > 5 || column < 0 ) {
        mq_send( clientQueue, errorMessage, strlen( errorMessage ), 0 );
      }
      // A right command is invalid if the index to the right is not a peg or the index after that is not empty
      else if ( puzzleBoard[ row ][ column ] != 'o' || puzzleBoard[ row ][ column + 1 ] != 'o' || puzzleBoard[ row ][ column + 2 ] != '.' ) {
        mq_send( clientQueue, errorMessage, strlen( errorMessage ), 0 );
      }
      // Otherwise, perform the valid move and udpate the undo history with the successful move.
      else {
        // Perform the move.
        puzzleBoard[ row ][ column ] = '.';
        puzzleBoard[ row ][ column + 1 ] = '.';
        puzzleBoard[ row ][ column + 2 ] = 'o';

        // Update the undo history to include the most recent move.
        strcpy( undoHistory[ 2 ], undoHistory[ 1 ] );
        strcpy( undoHistory[ 1 ], undoHistory[ 0 ] );
        strcpy( undoHistory[ 0 ], serverReceive );

        // Send a success message.
        mq_send( clientQueue, successMessage, strlen( successMessage ), 0 );
      }
    }
    // Perform the left move if the the left command is inputted.
    else if( serverReceive[ 0 ] == 'l' ) {
      // Row of the peg to move.
      int row = serverReceive[ 5 ] - '0';
      // Column of the peg to move.
      int column = serverReceive[ 7 ] - '0';

      // Left command cannot is invalid if the column is less than two or more than seven.
      if ( column < 2 || column > 7 ) {
        mq_send( clientQueue, errorMessage, strlen( errorMessage ), 0 );
      }
      // Left command is invalid if the index to the left is not a peg or the index after that is not empty
      else if ( puzzleBoard[ row ][ column ] != 'o' || puzzleBoard[ row ][ column - 1 ] != 'o' || puzzleBoard[ row ][ column - 2 ] != '.' ) {
        mq_send( clientQueue, errorMessage, strlen( errorMessage ), 0 );
      }
      // Otherwise, perform the valid move and udpate the undo history with the successful move.
      else {
        // Perform the move.
        puzzleBoard[ row ][ column ] = '.';
        puzzleBoard[ row ][ column - 1 ] = '.';
        puzzleBoard[ row ][ column - 2 ] = 'o';

        // Update the undo history to include the most recent move.
        strcpy( undoHistory[ 2 ], undoHistory[ 1 ] );
        strcpy( undoHistory[ 1 ], undoHistory[ 0 ] );
        strcpy( undoHistory[ 0 ], serverReceive );

        // Send a success message.
        mq_send( clientQueue, successMessage, strlen( successMessage ), 0 );
      }
    }
    // Perform the up move if the the up command is inputted.
    else if( serverReceive[ 0 ] == 'u' && serverReceive[ 1 ] == 'p') {
      // Row of the peg to move.
      int row = serverReceive[ 3 ] - '0';
      // Column of the peg to move.
      int column = serverReceive[ 5 ] - '0';

      // Up command is invalid if the row is less than two or more than seven.
      if ( row < 2 || row > 7 ) {
        mq_send( clientQueue, errorMessage, strlen( errorMessage ), 0 );
      }
      // Up command is invalid if the index above the peg is not a peg or the index after that is not empty
      else if ( puzzleBoard[ row ][ column ] != 'o' || puzzleBoard[ row - 1 ][ column ] != 'o' || puzzleBoard[ row - 2 ][ column ] != '.' ) {
        mq_send( clientQueue, errorMessage, strlen( errorMessage ), 0 );
      }
      // Otherwise, perform the valid move and udpate the undo history with the successful move.
      else {
        // Perform the move.
        puzzleBoard[ row ][ column ] = '.';
        puzzleBoard[ row - 1 ][ column ] = '.';
        puzzleBoard[ row - 2 ][ column ] = 'o';

        // Update the undo history to include the most recent move.
        strcpy( undoHistory[ 2 ], undoHistory[ 1 ] );
        strcpy( undoHistory[ 1 ], undoHistory[ 0 ] );
        strcpy( undoHistory[ 0 ], serverReceive );

        // Send a success message.
        mq_send( clientQueue, successMessage, strlen( successMessage ), 0 );
      }
    }
    // Perform the down move if the the up command is inputted.
    else if( serverReceive[ 0 ] == 'd' ) {
      // Row of the peg to move.
      int row = serverReceive[ 5 ] - '0';
      // Column of the peg to move.
      int column = serverReceive[ 7 ] - '0';

      // Dwon command is invalid if the row is more than five or less than zero.
      if ( row > 5 || row < 0 ) {
        mq_send( clientQueue, errorMessage, strlen( errorMessage ), 0 );
      }
      // Down command is invalid if the index to the right is not a peg or the index after that is not empty.
      else if ( puzzleBoard[ row ][ column ] != 'o' || puzzleBoard[ row + 1 ][ column ] != 'o' || puzzleBoard[ row + 2 ][ column ] != '.' ) {
        mq_send( clientQueue, errorMessage, strlen( errorMessage ), 0 );
      }
      // Otherwise, perform the valid move and udpate the undo history with the successful move.
      else {
        // Perform the move.
        puzzleBoard[ row ][ column ] = '.';
        puzzleBoard[ row + 1 ][ column ] = '.';
        puzzleBoard[ row + 2 ][ column ] = 'o';

        // Update the undo history to include the most recent move.
        strcpy( undoHistory[ 2 ], undoHistory[ 1 ] );
        strcpy( undoHistory[ 1 ], undoHistory[ 0 ] );
        strcpy( undoHistory[ 0 ], serverReceive );

        // Send a success message.
        mq_send( clientQueue, successMessage, strlen( successMessage ), 0 );
      } 
    }
    // Perform the undo move if the the undo command is inputted.
    else if( serverReceive[ 0 ] == 'u' && serverReceive[ 1 ] == 'n') {
      // Send error if no move has been made or the undo history is empty.
      if ( strlen( undoHistory[ 0 ] ) <= 1 )
        mq_send( clientQueue, errorMessage, strlen( errorMessage ), 0 );
      // Otherwise undo the move and send a success message.
      else {
        // Undo a right command.
        if( undoHistory[ 0 ][ 0 ] == 'r' ) {
          // Row of the peg to undo.
          int row = undoHistory[ 0 ][ 6 ] - '0';
          // Column of the peg to undo.
          int column = undoHistory[ 0 ][ 8 ] - '0';

          // Perform the opposite of the most recent move in the undo history.
          puzzleBoard[ row ][ column ] = 'o';
          puzzleBoard[ row ][ column + 1 ] = 'o';
          puzzleBoard[ row ][ column + 2 ] = '.';
        }
        // Undo a left command.
        else if( undoHistory[ 0 ][ 0 ] == 'l' ) {
          // Row of the peg to undo.
          int row = undoHistory[ 0 ][ 5 ] - '0';
          // Column of the peg to undo.
          int column = undoHistory[ 0 ][ 7 ] - '0';

          // Perform the opposite of the most recent move in the undo history.
          puzzleBoard[ row ][ column ] = 'o';
          puzzleBoard[ row ][ column - 1 ] = 'o';
          puzzleBoard[ row ][ column - 2 ] = '.';
        }
        // Undo an up command.
        else if( undoHistory[ 0 ][ 0 ] == 'u' ) {
          // Row of the peg to undo.
          int row = undoHistory[ 0 ][ 3 ] - '0';
          // Column of the peg to undo.
          int column = undoHistory[ 0 ][ 5 ] - '0';

          // Perform the opposite of the most recent move in the undo history.
          puzzleBoard[ row ][ column ] = 'o';
          puzzleBoard[ row - 1 ][ column ] = 'o';
          puzzleBoard[ row - 2 ][ column ] = '.';
        }
        // Undo a down command.
        else if( undoHistory[ 0 ][ 0 ] == 'd' ) {
          // Row of the peg to undo.
          int row = undoHistory[ 0 ][ 5 ] - '0';
          // Column of the peg to undo.
          int column = undoHistory[ 0 ][ 7 ] - '0';

          // Perform the opposite of the most recent move in the undo history.
          puzzleBoard[ row ][ column ] = 'o';
          puzzleBoard[ row + 1 ][ column ] = 'o';
          puzzleBoard[ row + 2 ][ column ] = '.';
        }

        // Update the undo history to not include the most recent move.
        strcpy( undoHistory[ 0 ], undoHistory[ 1 ] );
        strcpy( undoHistory[ 1 ], undoHistory[ 2 ] );
        strcpy( undoHistory[ 2 ], "" );

        // Send a success message to the client.
        mq_send( clientQueue, successMessage, strlen( successMessage ), 0 );
      }
    }
    // Perform the show command if this command is inputted.
    else if( strcmp( serverReceive, "show" ) == 0 ) {
      // String that the board message will be contained in.
      char boardMessage[ 73 ] = "";

      // Copy the puzzle board into the message
      strncpy( boardMessage, puzzleBoard[ 0 ], 9 );
      for ( int i = 1; i < GRID_SIZE; i++ ) {
        strncat( boardMessage, puzzleBoard[ i ], 9 );
      }

      // Ensure null terminator is at the end of the board message.
      strcat( boardMessage, "\0" );

      mq_send( clientQueue, boardMessage, strlen( boardMessage ), 0 );
    }
    else {
      // whatever error is... NOT RIGHT _-----_______------______----- 
      mq_send( clientQueue, errorMessage, strlen( errorMessage ), 0 );
    }

    //fprintf( stderr, "\n%s\n%s\n%s\n", undoHistory[ 0 ], undoHistory[ 1 ], undoHistory[ 2 ] );
  }

  // Close our two message queues (and delete them).
  mq_close( clientQueue );
  mq_close( serverQueue );

  mq_unlink( SERVER_QUEUE );
  mq_unlink( CLIENT_QUEUE );

  return 0;
}
