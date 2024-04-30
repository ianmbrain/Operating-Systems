
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
#include <semaphore.h>

/** Sempahore to provide mutual exclusion for different commands. */
sem_t *sem_lock;

// Print out an error message and exit.
static void fail( char const *message ) {
  fprintf( stderr, "%s\n", message );
  exit( 1 );
}

// Make a move in the given direction from the given
// row, column location (r, c). The dir value is one of
// the four constants, RIGHT_DIR, DOWN_DIR, LEFT_DIR or UP_DIR
// from common.h
bool move( Puzzle *state, int r, int c, int dir )
{
    // Acquire the shared semaphore to prevent other commands from running.
    #ifndef UNSAFE
      sem_wait( sem_lock );
    #endif

    if( dir == RIGHT_DIR ) {
      // A right command is invalid if the column is more than five or less than zero.
      if ( c > 5 || c < 0 ) {
        // Release the semaphore so that other operations can be performed.
        #ifndef UNSAFE
          sem_post ( sem_lock );
        #endif

        return false;
      }
      // A right command is invalid if the index to the right is not a peg or the index after that is not empty
      else if ( state->puzzleBoard[ r ][ c ] != 'o' || state->puzzleBoard[ r ][ c + 1 ] != 'o' || state->puzzleBoard[ r ][ c + 2 ] != '.' ) {
        // Release the semaphore so that other operations can be performed.
        #ifndef UNSAFE
          sem_post ( sem_lock );
        #endif

        return false;
      }
      // Otherwise, perform the valid move and udpate the undo history with the successful move.
      else {
        // Perform the move.
        state->puzzleBoard[ r ][ c ] = '.';
        state->puzzleBoard[ r ][ c + 1 ] = '.';
        state->puzzleBoard[ r ][ c + 2 ] = 'o';

        // Convert the integer row into a string.
        char row_str[ 2 ] = "";
        sprintf(row_str, "%d", r);

        // Convert the integer column into a string.
        char col_str[ 2 ] = "";
        sprintf(col_str, "%d", c);

        // Convert the integer direction into a string.
        char dir_str[ 2 ] = "";
        sprintf(dir_str, "%d", dir);

        // Copy each of the strings into a full command to be added to the undo history.
        char full_command[ 3 + 1 ] = "";
        strcpy( full_command, row_str );
        strcat( full_command, col_str );
        strcat( full_command, dir_str );

        // Update the undo history to include the most recent move.
        strcpy( state->undoHistory[ 2 ], state->undoHistory[ 1 ] );
        strcpy( state->undoHistory[ 1 ], state->undoHistory[ 0 ] );
        strcpy( state->undoHistory[ 0 ], full_command );

        // Release the semaphore so that other operations can be performed.
        #ifndef UNSAFE
          sem_post ( sem_lock );
        #endif

        return true;
      }
    }
    else if( dir == LEFT_DIR ) {
      // A left command is invalid if the column is more than five or less than zero.
      if ( c > 7 || c < 2 ) {
        // Release the semaphore so that other operations can be performed.
        #ifndef UNSAFE
          sem_post ( sem_lock );
        #endif

        return false;
      }
      // A left command is invalid if the index to the left is not a peg or the index after that is not empty
      else if ( state->puzzleBoard[ r ][ c ] != 'o' || state->puzzleBoard[ r ][ c - 1 ] != 'o' || state->puzzleBoard[ r ][ c - 2 ] != '.' ) {
        // Release the semaphore so that other operations can be performed.
        #ifndef UNSAFE
          sem_post ( sem_lock );
        #endif

        return false;
      }
      // Otherwise, perform the valid move and udpate the undo history with the successful move.
      else {
        // Perform the move.
        state->puzzleBoard[ r ][ c ] = '.';
        state->puzzleBoard[ r ][ c - 1 ] = '.';
        state->puzzleBoard[ r ][ c - 2 ] = 'o';

        // Convert the integer row into a string.
        char row_str[ 2 ] = "";
        sprintf(row_str, "%d", r);

        // Convert the integer column into a string.
        char col_str[ 2 ] = "";
        sprintf(col_str, "%d", c);

        // Convert the integer direction into a string.
        char dir_str[ 2 ] = "";
        sprintf(dir_str, "%d", dir);

        // Copy each of the strings into a full command to be added to the undo history.
        char full_command[ 3 + 1 ] = "";
        strcpy( full_command, row_str );
        strcat( full_command, col_str );
        strcat( full_command, dir_str );

        // Update the undo history to include the most recent move.
        strcpy( state->undoHistory[ 2 ], state->undoHistory[ 1 ] );
        strcpy( state->undoHistory[ 1 ], state->undoHistory[ 0 ] );
        strcpy( state->undoHistory[ 0 ], full_command );

        // Release the semaphore so that other operations can be performed.
        #ifndef UNSAFE
          sem_post ( sem_lock );
        #endif

        return true;
      }
    }
    else if( dir == UP_DIR ) {
      // A up command is invalid if the column is more than five or less than zero.
      if ( r < 2 || r > 7 ) {
        // Release the semaphore so that other operations can be performed.
        #ifndef UNSAFE
          sem_post ( sem_lock );
        #endif

        return false;
      }
      // A up command is invalid if the index to the up is not a peg or the index after that is not empty
      else if ( state->puzzleBoard[ r ][ c ] != 'o' || state->puzzleBoard[ r - 1 ][ c ] != 'o' || state->puzzleBoard[ r - 2 ][ c ] != '.' ) {
        // Release the semaphore so that other operations can be performed.
        #ifndef UNSAFE
          sem_post ( sem_lock );
        #endif

        return false;
      }
      // Otherwise, perform the valid move and udpate the undo history with the successful move.
      else {
        // Perform the move.
        state->puzzleBoard[ r ][ c ] = '.';
        state->puzzleBoard[ r - 1 ][ c ] = '.';
        state->puzzleBoard[ r - 2 ][ c ] = 'o';

        // Convert the integer row into a string.
        char row_str[ 2 ] = "";
        sprintf(row_str, "%d", r);

        // Convert the integer column into a string.
        char col_str[ 2 ] = "";
        sprintf(col_str, "%d", c);

        // Convert the integer direction into a string.
        char dir_str[ 2 ] = "";
        sprintf(dir_str, "%d", dir);

        // Copy each of the strings into a full command to be added to the undo history.
        char full_command[ 3 + 1 ] = "";
        strcpy( full_command, row_str );
        strcat( full_command, col_str );
        strcat( full_command, dir_str );

        // Update the undo history to include the most recent move.
        strcpy( state->undoHistory[ 2 ], state->undoHistory[ 1 ] );
        strcpy( state->undoHistory[ 1 ], state->undoHistory[ 0 ] );
        strcpy( state->undoHistory[ 0 ], full_command );

        // Release the semaphore so that other operations can be performed.
        #ifndef UNSAFE
          sem_post ( sem_lock );
        #endif

        return true;
      }
    }
    else if( dir == DOWN_DIR ) {
      // A down command is invalid if the column is more than five or less than zero.
      if ( r > 5 || r < 0 ) {
        // Release the semaphore so that other operations can be performed.
        #ifndef UNSAFE
          sem_post ( sem_lock );
        #endif

        return false;
      }
      // A down command is invalid if the index to the down is not a peg or the index after that is not empty
      else if ( state->puzzleBoard[ r ][ c ] != 'o' || state->puzzleBoard[ r + 1 ][ c ] != 'o' || state->puzzleBoard[ r + 2 ][ c ] != '.' ) {
        // Release the semaphore so that other operations can be performed.
        #ifndef UNSAFE
          sem_post ( sem_lock );
        #endif

        return false;
      }
      // Otherwise, perform the valid move and udpate the undo history with the successful move.
      else {
        // Perform the move.
        state->puzzleBoard[ r ][ c ] = '.';
        state->puzzleBoard[ r + 1 ][ c ] = '.';
        state->puzzleBoard[ r + 2 ][ c ] = 'o';

        // Convert the integer row into a string.
        char row_str[ 2 ] = "";
        sprintf(row_str, "%d", r);

        // Convert the integer column into a string.
        char col_str[ 2 ] = "";
        sprintf(col_str, "%d", c);

        // Convert the integer direction into a string.
        char dir_str[ 2 ] = "";
        sprintf(dir_str, "%d", dir);

        // Copy each of the strings into a full command to be added to the undo history.
        char full_command[ 3 + 1 ] = "";
        strcpy( full_command, row_str );
        strcat( full_command, col_str );
        strcat( full_command, dir_str );

        // Update the undo history to include the most recent move.
        strcpy( state->undoHistory[ 2 ], state->undoHistory[ 1 ] );
        strcpy( state->undoHistory[ 1 ], state->undoHistory[ 0 ] );
        strcpy( state->undoHistory[ 0 ], full_command );

        // Release the semaphore so that other operations can be performed.
        #ifndef UNSAFE
          sem_post ( sem_lock );
        #endif

        return true;
      }
    }
    // Otherwise, return false if the move is invalid MAYBE FAIL() _____-------________-------______----
    else {
      // Release the semaphore so that other operations can be performed.
        #ifndef UNSAFE
          sem_post ( sem_lock );
        #endif

      return false;
    }
  // Release the semaphore so that other operations can be performed.
  #ifndef UNSAFE
    sem_post ( sem_lock );
  #endif

  return false;
}

// Undo the most recent move.
bool undo( Puzzle *state )
{
    // Acquire the shared semaphore to prevent other commands from running.
    #ifndef UNSAFE
      sem_wait( sem_lock );
    #endif

    // Send error if no move has been made or the undo history is empty.
    if ( strlen( state->undoHistory[ 0 ] ) <= 1 ) {
      // Release the semaphore so that other operations can be performed.
      #ifndef UNSAFE
        sem_post ( sem_lock );
      #endif

      return false;
    }
    // Otherwise undo the move and send a success message.
    else {
      // Undo a right command.
      if( state->undoHistory[ 0 ][ 2 ] - '0' == RIGHT_DIR ) {
        // Row of the peg to undo.
        int row = state->undoHistory[ 0 ][ 0 ] - '0';
        // Column of the peg to undo.
        int column = state->undoHistory[ 0 ][ 1 ] - '0';

        // Perform the opposite of the most recent move in the undo history.
        state->puzzleBoard[ row ][ column ] = 'o';
        state->puzzleBoard[ row ][ column + 1 ] = 'o';
        state->puzzleBoard[ row ][ column + 2 ] = '.';
      }
      // Undo a left command.
      else if( state->undoHistory[ 0 ][ 2 ] - '0' == LEFT_DIR ) {
        // Row of the peg to undo.
        int row = state->undoHistory[ 0 ][ 0 ] - '0';
        // Column of the peg to undo.
        int column = state->undoHistory[ 0 ][ 1 ] - '0';

        // Perform the opposite of the most recent move in the undo history.
        state->puzzleBoard[ row ][ column ] = 'o';
        state->puzzleBoard[ row ][ column - 1 ] = 'o';
        state->puzzleBoard[ row ][ column - 2 ] = '.';
      }
      // Undo an up command.
      else if( state->undoHistory[ 0 ][ 2 ] - '0' == UP_DIR ) {
        // Row of the peg to undo.
        int row = state->undoHistory[ 0 ][ 0 ] - '0';
        // Column of the peg to undo.
        int column = state->undoHistory[ 0 ][ 1 ] - '0';

        // Perform the opposite of the most recent move in the undo history.
        state->puzzleBoard[ row ][ column ] = 'o';
        state->puzzleBoard[ row - 1 ][ column ] = 'o';
        state->puzzleBoard[ row - 2 ][ column ] = '.';
      }
      // Undo a down command.
      else if( state->undoHistory[ 0 ][ 2 ] - '0' == DOWN_DIR ) {
        // Row of the peg to undo.
        int row = state->undoHistory[ 0 ][ 0 ] - '0';
        // Column of the peg to undo.
        int column = state->undoHistory[ 0 ][ 1 ] - '0';

        // Perform the opposite of the most recent move in the undo history.
        state->puzzleBoard[ row ][ column ] = 'o';
        state->puzzleBoard[ row + 1 ][ column ] = 'o';
        state->puzzleBoard[ row + 2 ][ column ] = '.';
      }

      // Update the undo history to not include the most recent move.
      strcpy( state->undoHistory[ 0 ], state->undoHistory[ 1 ] );
      strcpy( state->undoHistory[ 1 ], state->undoHistory[ 2 ] );
      strcpy( state->undoHistory[ 2 ], "" );
    }
    // Release the semaphore so that other operations can be performed.
    #ifndef UNSAFE
      sem_post ( sem_lock );
    #endif

    return true;
}

// Print the current state of the puzzle
void show( Puzzle *state )
{
    // Acquire the shared semaphore to prevent other commands from running.
    #ifndef UNSAFE
      sem_wait( sem_lock );
    #endif
    
    // String that the board message will be contained in.
    char boardMessage[ 73 ] = "";

    // Copy the puzzle board into the message
    strncpy( boardMessage, state->puzzleBoard[ 0 ], 9 );
    for ( int i = 1; i < GRID_SIZE; i++ ) {
      strncat( boardMessage, state->puzzleBoard[ i ], 9 );
    }

    // Ensure null terminator is at the end of the board message.
    strcat( boardMessage, "\0" );

    // Print out the puzzle board.
    printf( "%s", boardMessage );

    // Detatch from the shared memory.
    shmdt( state );

    // Release the semaphore so that other operations can be performed.
    #ifndef UNSAFE
      sem_post ( sem_lock );
    #endif
}

int main( int argc, char *argv[] ) {
  // Eventual exit status for success or failure.
  int status = EXIT_SUCCESS;

  /* Citing Help from the course website
  * The code for opening the semaphore and checking for failure is based on the semPost.c example program from the synchronization section of the course website.
  */
  // Create the named Semaphore with an initial value of one.
  sem_lock = sem_open( LOCK_NAME, 0, 0, 1 );
  
  // Fail as error if the semaphore cannot be created.
  if ( sem_lock == SEM_FAILED )
    fail( "Cannot create the named semaphore" );

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
  if ( argc < 2 || argc > 6 ) 
      fail( "error" );

  // Command that the user inputs.
  char command[ 5 + 1 ] = "";
  strcpy( command, argv[ 1 ] );

  // Right, left, up, or down move command.
  if ( strcmp( argv[ 1 ], "left" ) == 0 || strcmp( argv[ 1 ], "right" ) == 0 || strcmp( argv[ 1 ], "up" ) == 0 || strcmp( argv[ 1 ], "down" ) == 0 ) {
    // Entire command to be inserted into the undo history.
    // char full_command[ 3 + 1 ] = "";
    // strcpy( full_command, argv[ 1 ] );
    // strcat( full_command, " " );
    // strcat( full_command, argv[ 2 ] );
    // strcat( full_command, " " );
    // strcat( full_command, argv[ 3 ] );
    
    // Fail if there are an incorrect number of command line arguments.
    if ( argc != 4 ) 
        fail( "error" );

    // Row of the peg to move.
    int row = atoi( argv[ 2 ] );
    // Column of the peg to move.
    int column = atoi( argv[ 3 ] );

    if( strcmp( command, "right" ) == 0 ) {
        // Perform the move command. If the command succeeds update the undo history.
        if ( move( puzzleMemory, row, column , RIGHT_DIR ) ) {

            // Update the undo history to include the most recent move.
            // strcpy( puzzleMemory->undoHistory[ 2 ], puzzleMemory->undoHistory[ 1 ] );
            // strcpy( puzzleMemory->undoHistory[ 1 ], puzzleMemory->undoHistory[ 0 ] );
            // strcpy( puzzleMemory->undoHistory[ 0 ], full_command );

            // Set the status to success.
            status = EXIT_SUCCESS;
        }
        else {
           status = EXIT_FAILURE;
        }
    } 
    else if( strcmp( command, "left" ) == 0 ) {
        // Perform the move command. If the command succeeds update the undo history.
        if ( move( puzzleMemory, row, column , LEFT_DIR ) ) {

            // Update the undo history to include the most recent move.
            // strcpy( puzzleMemory->undoHistory[ 2 ], puzzleMemory->undoHistory[ 1 ] );
            // strcpy( puzzleMemory->undoHistory[ 1 ], puzzleMemory->undoHistory[ 0 ] );
            // strcpy( puzzleMemory->undoHistory[ 0 ], full_command );

            // Set the status to success.
            status = EXIT_SUCCESS;
        }
        else {
           status = EXIT_FAILURE;
        }
      }
    else if( strcmp( command, "up" ) == 0 ) {
      if ( move( puzzleMemory, row, column , UP_DIR ) ) {

          // Update the undo history to include the most recent move.
          // strcpy( puzzleMemory->undoHistory[ 2 ], puzzleMemory->undoHistory[ 1 ] );
          // strcpy( puzzleMemory->undoHistory[ 1 ], puzzleMemory->undoHistory[ 0 ] );
          // strcpy( puzzleMemory->undoHistory[ 0 ], full_command );

          // Set the status to success.
          status = EXIT_SUCCESS;
      }
      else {
         status = EXIT_FAILURE;
      }
    }
    else if( strcmp( command, "down" ) == 0 ) {
      if ( move( puzzleMemory, row, column , DOWN_DIR ) ) {

          // Update the undo history to include the most recent move.
          // strcpy( puzzleMemory->undoHistory[ 2 ], puzzleMemory->undoHistory[ 1 ] );
          // strcpy( puzzleMemory->undoHistory[ 1 ], puzzleMemory->undoHistory[ 0 ] );
          // strcpy( puzzleMemory->undoHistory[ 0 ], full_command );

          // Set the status to success.
          status = EXIT_SUCCESS;
      }
      else {
         status = EXIT_FAILURE;
      }
    }
  }
  else if ( strcmp( command, "undo" ) == 0 ) {
    // Perform the undo command if the command is undo
    if ( undo( puzzleMemory ) ) {
      // Set the status to success if the undo command worked.
      status = EXIT_SUCCESS;
    }
    else {
       status = EXIT_FAILURE;
    }
  }
  // Perform the show command if this command is inputted.
  else if( strcmp( command, "show" ) == 0 ) {
    // Run the show comand
    show( puzzleMemory );

    // Return the status of the puzzle and exit
    return status;
  }
  // Perform the test command if this command is inputted.
  else if( ( strcmp( argv[ 1 ], "test" ) == 0 ) ) {
    // Fail as error if there are not a valid number of command line arguments
    if ( argc != 6 ) {
      fail( "error" );
    }

    // Number of times the specified command should be run.
    int n = atoi( argv[ 2 ] );
    // Direction of the specified move command
    char dirCommand[ 5 + 1 ] = "";
    strcpy( dirCommand, argv[ 3 ] );

    // Integer version of the command for the move function.
    int dir = 0;
    // Set the direction to the right direction.
    if ( strcmp( dirCommand, "right" ) == 0 ) {
      dir = RIGHT_DIR;
    }
    // Set the direction to the left direction.
    else if ( strcmp( dirCommand, "left" ) == 0 ) {
      dir = LEFT_DIR;
    }
    // Set the direction to the up direction.
    else if ( strcmp( dirCommand, "up" ) == 0 ) {
      dir = UP_DIR;
    }
    // Set the direction to the down direction.
    else if ( strcmp( dirCommand, "down" ) == 0 ) {
      dir = DOWN_DIR;
    }
    // Fail as error if the command is not valid.
    else {
      fail( "error" );
    }

    // Row of the move command
    int r = atoi( argv[ 4 ] );
    // Column of the move command
    int c = atoi( argv[ 5 ] );

    // Entire command to be inserted into the undo history.
    // char full_command[ 9 + 1 ] = "";
    // strcpy( full_command, argv[ 3 ] );
    // strcat( full_command, " " );
    // strcat( full_command, argv[ 4 ] );
    // strcat( full_command, " " );
    // strcat( full_command, argv[ 5 ] );

    // Run the move command the specified number of times, undoing the command each time. This is used to test mutual exclusion.
    for ( int i = 0; i < n; i++ ) {
      move( puzzleMemory, r, c, dir );
      undo( puzzleMemory );
    }

    // Set the status to success.
    status = EXIT_SUCCESS;
  }
  else {
    // If the command is not one of the allowed commands fail as error.
    fail( "error" );
  }

  if( status == 0 ) {
    printf( "success\n" );
  }
  else {
    fail( "error" ); 
  }

  // Detatch from the shared memory.
  shmdt( puzzleMemory );

  return status;
}
