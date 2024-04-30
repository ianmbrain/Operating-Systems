/** 
    @file reset.c
    @author Ian M Brain (imbrain)
    This program provdes functionality to read in a game board and allow the peg.c program to interact with it.
    Interaction is done through shared memory.
    This program also contructs a named semaphore to provide mutual exclusion when updating the shared memory.
    Interacts with peg.c through shared memory.
*/

#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <errno.h>
#include <string.h>
#include "common.h"
#include <semaphore.h>

// Print out an error message and exit.
static void fail( char const *message ) {
  fprintf( stderr, "%s\n", message );
  exit( 1 );
}

// Print out a usage message and exit.
static void usage() {
  fprintf( stderr, "usage: reset <puzzle-file>\n" );
  exit( 1 );
}

int main( int argc, char *argv[] ) {
  // Eight by eight array representing the puzzle board.
  char puzzleBoard[ GRID_SIZE ][ GRID_SIZE + 1 ] = { };

  // The command line aruments are invalid if there are not two arguments.
  if ( argc != 2 )
    usage();

  // Open the specified file.
  int fd = open( argv[ 1 ], O_RDONLY, 0600 );

  // Print error message and exit unsuccessfully if the file cannot be opened.
  if ( fd == -1 ) {
    char errorMessage[ 1024 + 20 ] = "";
    strcat( errorMessage, "Invalid input file: " );
    strcat( errorMessage, argv[ 1 ] );

    fail( errorMessage );
  }

  // Currently read line of the file.
  char readLine[ GRID_SIZE + 1 ] = "";

  // How many characters from the line have been read.
  int charsRead = 0;

  // Read the lines from the input file.
  for( int i = 0; i < 8; i++ ) {
    // Read in each of the characters into the buffer including the new line character.
    charsRead = read( fd, readLine, 9 );

    // Exit as failure if the file line does not contain nine characters
    if ( charsRead != ( GRID_SIZE + 1 ) ) {
      char errorMessage[ 1024 + 20 ] = "";
      strcat( errorMessage, "Invalid input file: " );
      strcat( errorMessage, argv[ 1 ] );

      fail( errorMessage );
    }

    // Exit as failure if the last character in the line is not a new line character.
    if ( readLine[ GRID_SIZE ] != '\n' ) {
      char errorMessage[ 1024 + 20 ] = "";
      strcat( errorMessage, "Invalid input file: " );
      strcat( errorMessage, argv[ 1 ] );

      fail( errorMessage );
    }

    for ( int j = 0; j < GRID_SIZE; j++ ) {
      // Exit as failure if the character is not a valid character.
      if ( readLine[ j ] != '.' && readLine[ j ] != 'o' && readLine[ j ] != '#' ) {
        char errorMessage[ 1024 + 20 ] = "";
        strcat( errorMessage, "Invalid input file: " );
        strcat( errorMessage, argv[ 1 ] );

        fail( errorMessage );
      }

      // Copy just the characters from the line into the puzzle board.
      puzzleBoard[ i ][ j ] = readLine[ j ];
    }

    // Add new line to the end of the puzzle board.
    puzzleBoard[ i ][ 8 ] = '\n';

    strcpy( readLine, "" );
  }

  // Read the last line of the input file
  charsRead = read( fd, readLine, 1 );

  // Exit as failure if the last line contains any characters.
  if ( charsRead != 0 ) {
    char errorMessage[ 1024 + 20 ] = "";
    strcat( errorMessage, "Invalid input file: " );
    strcat( errorMessage, argv[ 1 ] );

    fail( errorMessage );
  }

  /* Citing Help from the course website
  * The code for opening and attaching to the shared memory is based on the shmReader.c example program from the Processes section of the course website.
  */
  // Create the memory shared between the server and client that is size of the puzzle struct.
  int shmid = shmget( ftok( "/mnt/ncsudrive/i/imbrain", 1 ), sizeof( Puzzle ), 0666 | IPC_CREAT );
  // Fail as error if memory cannot be gotten.
  if ( shmid == -1 )
    fail( "Shared memory could not be created " );

  // Attach to the shared memory and get the puzzle struct.
  Puzzle *puzzleMemory = ( Puzzle * )shmat( shmid, 0, 0 );
  // Fail if the memory cannot be attached to.
  if ( puzzleMemory == ( Puzzle * )-1 )
    fail( "Cannot map Puzzle to shared memory" );

  // Write the Puzzle struct into the shared memory.
  for ( int i = 0; i < GRID_SIZE; i++ ) {
    strcpy( puzzleMemory->puzzleBoard[ i ], puzzleBoard[ i ] );
  }

  // Detatch from the shared memory.
  shmdt( puzzleMemory );

  /* Citing Help from the course website
  * The code for opening the semaphore and checking for failure is based on the semPost.c example program from the synchronization section of the course website.
  */
  // Create the named Semaphore with an initial value of one.
  sem_unlink( LOCK_NAME );
  sem_t *sem_lock = sem_open( LOCK_NAME, O_CREAT, 0600, 1 );

  // Fail as error if the semaphore cannot be created.
  if ( sem_lock == SEM_FAILED ) {
    fail( "Cannot create the named semaphore" );
  }
  
  // Exit as success
  return EXIT_SUCCESS;
}
