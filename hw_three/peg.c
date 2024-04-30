/** 
    @file peg.c
    @author Ian M Brain (imbrain)
    This program acts similarly as a client.
    Allows users to play a peg-jumping puzzle.
    Attaches to shared memory and gets the puzzle board struct from it.
    Performs allowed operations on the puzzle and updates the shared memory.
    Citing Help from other Assignments
    The code for this program is based on client.c in homework 2 that I completed on 2/1/2024.
*/

#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <errno.h>
#include <string.h>
#include "common.h"

// Print out an error message and exit.
static void fail( char const *message ) {
  fprintf( stderr, "%s\n", message );
  exit( 1 );
}

int main( int argc, char *argv[] ) {
  // Eventual exit status for success or failure.
  int status = EXIT_SUCCESS;

  /* Citing Help from the course website
  * The code for opening and attaching to the shared memory is based on the shmWriter.c example program from the Processes section of the course website.
  */
  // Open the shared memory.
  int shmid = shmget( ftok( "/mnt/ncsudrive/i/imbrain", 1 ), sizeof( Puzzle ), 0 );
  if ( shmid == -1 )
    fail( "Shared memory could not be attached " );

  // Attach to the puzzle memory point to the contained struct.
  Puzzle *puzzleMemory = ( Puzzle * )shmat( shmid, 0, 0 );
  // Fail if the memory cannot be pointed to
  if ( puzzleMemory == ( Puzzle * )-1 )
    fail( "Cannot map to shared memory" );

  // Print error and exit if command line arguments are invalid.
  if ( argc < 2 || argc > 4 ) 
      fail( "error" );

  // Command that the user inputs.
  char command[ 5 + 1 ] = "";
  strcpy( command, argv[ 1 ] );

  // Right, left, up, or down move command.
  if ( strcmp( argv[ 1 ], "left" ) == 0 || strcmp( argv[ 1 ], "right" ) == 0 || strcmp( argv[ 1 ], "up" ) == 0 || strcmp( argv[ 1 ], "down" ) == 0 ) {
    // Entire command to be inserted into the undo history.
    char full_command[ 9 + 1 ] = "";
    strcpy( full_command, argv[ 1 ] );
    strcat( full_command, " " );
    strcat( full_command, argv[ 2 ] );
    strcat( full_command, " " );
    strcat( full_command, argv[ 3 ] );
    
    // Fail if there are an incorrect number of command line arguments.
    if ( argc != 4 ) 
        fail( "error" );

    // Row of the peg to move.
    int row = atoi( argv[ 2 ] );
    // Column of the peg to move.
    int column = atoi( argv[ 3 ] );

    if( strcmp( command, "right" ) == 0 ) {
      // A right command is invalid if the column is more than five or less than zero.
      if ( column > 5 || column < 0 ) {
        fail( "error" );
      }
      // A right command is invalid if the index to the right is not a peg or the index after that is not empty
      else if ( puzzleMemory->puzzleBoard[ row ][ column ] != 'o' || puzzleMemory->puzzleBoard[ row ][ column + 1 ] != 'o' || puzzleMemory->puzzleBoard[ row ][ column + 2 ] != '.' ) {
        fail( "error" );
      }
      // Otherwise, perform the valid move and udpate the undo history with the successful move.
      else {
        // Perform the move.
        puzzleMemory->puzzleBoard[ row ][ column ] = '.';
        puzzleMemory->puzzleBoard[ row ][ column + 1 ] = '.';
        puzzleMemory->puzzleBoard[ row ][ column + 2 ] = 'o';

        // Update the undo history to include the most recent move.
        strcpy( puzzleMemory->undoHistory[ 2 ], puzzleMemory->undoHistory[ 1 ] );
        strcpy( puzzleMemory->undoHistory[ 1 ], puzzleMemory->undoHistory[ 0 ] );
        strcpy( puzzleMemory->undoHistory[ 0 ], full_command );

        // Send a success message.
        status = EXIT_SUCCESS;
      }
    } 
    else if( strcmp( command, "left" ) == 0 ) {
      // A left command is invalid if the column is more than five or less than zero.
      if ( column > 7 || column < 2 ) {
        fail( "error" );
      }
      // A left command is invalid if the index to the left is not a peg or the index after that is not empty
      else if ( puzzleMemory->puzzleBoard[ row ][ column ] != 'o' || puzzleMemory->puzzleBoard[ row ][ column - 1 ] != 'o' || puzzleMemory->puzzleBoard[ row ][ column - 2 ] != '.' ) {
        fail( "error" );
      }
      // Otherwise, perform the valid move and udpate the undo history with the successful move.
      else {
        // Perform the move.
        puzzleMemory->puzzleBoard[ row ][ column ] = '.';
        puzzleMemory->puzzleBoard[ row ][ column - 1 ] = '.';
        puzzleMemory->puzzleBoard[ row ][ column - 2 ] = 'o';

        // Update the undo history to include the most recent move.
        strcpy( puzzleMemory->undoHistory[ 2 ], puzzleMemory->undoHistory[ 1 ] );
        strcpy( puzzleMemory->undoHistory[ 1 ], puzzleMemory->undoHistory[ 0 ] );
        strcpy( puzzleMemory->undoHistory[ 0 ], full_command );

        // Send a success message.
        status = EXIT_SUCCESS;
      }
    }
    else if( strcmp( command, "up" ) == 0 ) {
      // A up command is invalid if the column is more than five or less than zero.
      if ( row < 2 || row > 7 ) {
        fail( "error" );
      }
      // A up command is invalid if the index to the up is not a peg or the index after that is not empty
      else if ( puzzleMemory->puzzleBoard[ row ][ column ] != 'o' || puzzleMemory->puzzleBoard[ row - 1 ][ column ] != 'o' || puzzleMemory->puzzleBoard[ row - 2 ][ column ] != '.' ) {
        fail( "error" );
      }
      // Otherwise, perform the valid move and udpate the undo history with the successful move.
      else {
        // Perform the move.
        puzzleMemory->puzzleBoard[ row ][ column ] = '.';
        puzzleMemory->puzzleBoard[ row - 1 ][ column ] = '.';
        puzzleMemory->puzzleBoard[ row - 2 ][ column ] = 'o';

        // Update the undo history to include the most recent move.
        strcpy( puzzleMemory->undoHistory[ 2 ], puzzleMemory->undoHistory[ 1 ] );
        strcpy( puzzleMemory->undoHistory[ 1 ], puzzleMemory->undoHistory[ 0 ] );
        strcpy( puzzleMemory->undoHistory[ 0 ], full_command );

        // Send a success message.
        status = EXIT_SUCCESS;
      }
    }
    else if( strcmp( command, "down" ) == 0 ) {
      // A down command is invalid if the column is more than five or less than zero.
      if ( row > 5 || row < 0 ) {
        fail( "error" );
      }
      // A down command is invalid if the index to the down is not a peg or the index after that is not empty
      else if ( puzzleMemory->puzzleBoard[ row ][ column ] != 'o' || puzzleMemory->puzzleBoard[ row + 1 ][ column ] != 'o' || puzzleMemory->puzzleBoard[ row + 2 ][ column ] != '.' ) {
        fail( "error" );
      }
      // Otherwise, perform the valid move and udpate the undo history with the successful move.
      else {
        // Perform the move.
        puzzleMemory->puzzleBoard[ row ][ column ] = '.';
        puzzleMemory->puzzleBoard[ row + 1 ][ column ] = '.';
        puzzleMemory->puzzleBoard[ row + 2 ][ column ] = 'o';

        // Update the undo history to include the most recent move.
        strcpy( puzzleMemory->undoHistory[ 2 ], puzzleMemory->undoHistory[ 1 ] );
        strcpy( puzzleMemory->undoHistory[ 1 ], puzzleMemory->undoHistory[ 0 ] );
        strcpy( puzzleMemory->undoHistory[ 0 ], full_command );

        // Send a success message.
        status = EXIT_SUCCESS;
      }
    }
  }
  else if ( strcmp( command, "undo" ) == 0 ) {
    // Send error if no move has been made or the undo history is empty.
    if ( strlen( puzzleMemory->undoHistory[ 0 ] ) <= 1 )
      fail( "error" );
    // Otherwise undo the move and send a success message.
    else {
      // Undo a right command.
      if( puzzleMemory->undoHistory[ 0 ][ 0 ] == 'r' ) {
        // Row of the peg to undo.
        int row = puzzleMemory->undoHistory[ 0 ][ 6 ] - '0';
        // Column of the peg to undo.
        int column = puzzleMemory->undoHistory[ 0 ][ 8 ] - '0';

        // Perform the opposite of the most recent move in the undo history.
        puzzleMemory->puzzleBoard[ row ][ column ] = 'o';
        puzzleMemory->puzzleBoard[ row ][ column + 1 ] = 'o';
        puzzleMemory->puzzleBoard[ row ][ column + 2 ] = '.';
      }
      // Undo a left command.
      else if( puzzleMemory->undoHistory[ 0 ][ 0 ] == 'l' ) {
        // Row of the peg to undo.
        int row = puzzleMemory->undoHistory[ 0 ][ 5 ] - '0';
        // Column of the peg to undo.
        int column = puzzleMemory->undoHistory[ 0 ][ 7 ] - '0';

        // Perform the opposite of the most recent move in the undo history.
        puzzleMemory->puzzleBoard[ row ][ column ] = 'o';
        puzzleMemory->puzzleBoard[ row ][ column - 1 ] = 'o';
        puzzleMemory->puzzleBoard[ row ][ column - 2 ] = '.';
      }
      // Undo an up command.
      else if( puzzleMemory->undoHistory[ 0 ][ 0 ] == 'u' ) {
        // Row of the peg to undo.
        int row = puzzleMemory->undoHistory[ 0 ][ 3 ] - '0';
        // Column of the peg to undo.
        int column = puzzleMemory->undoHistory[ 0 ][ 5 ] - '0';

        // Perform the opposite of the most recent move in the undo history.
        puzzleMemory->puzzleBoard[ row ][ column ] = 'o';
        puzzleMemory->puzzleBoard[ row - 1 ][ column ] = 'o';
        puzzleMemory->puzzleBoard[ row - 2 ][ column ] = '.';
      }
      // Undo a down command.
      else if( puzzleMemory->undoHistory[ 0 ][ 0 ] == 'd' ) {
        // Row of the peg to undo.
        int row = puzzleMemory->undoHistory[ 0 ][ 5 ] - '0';
        // Column of the peg to undo.
        int column = puzzleMemory->undoHistory[ 0 ][ 7 ] - '0';

        // Perform the opposite of the most recent move in the undo history.
        puzzleMemory->puzzleBoard[ row ][ column ] = 'o';
        puzzleMemory->puzzleBoard[ row + 1 ][ column ] = 'o';
        puzzleMemory->puzzleBoard[ row + 2 ][ column ] = '.';
      }

      // Update the undo history to not include the most recent move.
      strcpy( puzzleMemory->undoHistory[ 0 ], puzzleMemory->undoHistory[ 1 ] );
      strcpy( puzzleMemory->undoHistory[ 1 ], puzzleMemory->undoHistory[ 2 ] );
      strcpy( puzzleMemory->undoHistory[ 2 ], "" );

      // Send a success message to the client.
      status = EXIT_SUCCESS;
    }
  }
  // Perform the show command if this command is inputted.
  else if( strcmp( command, "show" ) == 0 ) {
    // String that the board message will be contained in.
    char boardMessage[ 73 ] = "";

    // Copy the puzzle board into the message
    strncpy( boardMessage, puzzleMemory->puzzleBoard[ 0 ], 9 );
    for ( int i = 1; i < GRID_SIZE; i++ ) {
      strncat( boardMessage, puzzleMemory->puzzleBoard[ i ], 9 );
    }

    // Ensure null terminator is at the end of the board message.
    strcat( boardMessage, "\0" );

    // Print out the puzzle board.
    printf( "%s", boardMessage );

    // Detatch from the shared memory.
    shmdt( puzzleMemory );

    return status;
  }
  else {
    // If the command is not one of the allowed commands fail as error.
    fail( "error" );
  }

  if( status == 0 ) {
    printf( "success\n" );
  }

  // Detatch from the shared memory.
  shmdt( puzzleMemory );

  return status;
}
