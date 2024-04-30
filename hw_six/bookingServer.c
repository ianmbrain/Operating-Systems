/** 
    @file bookingServer.c
    @author Ian M Brain (imbrain)
    This program serves as a server that allows clients to book rooms on specific days.
    Clients can book rooms, cancel bookings, and show the board using various commands.
    This program utilizes a socket to communicate with the client.
    Multiple clients can connect to the socket and perform operations as threads.
*/

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <errno.h>
#include <semaphore.h>

/** Port number used by my server */
#define PORT_NUMBER "26308"

/** Maximum command line length */
#define N 256

/** Maximum command line length */
#define CMD_LENGTH 15

// To dimensional char array representing the booking board of rooms and days.
char **bookingBoard;

// Number of days allowed to be booked on.
int globalDays = 0;

// Number of rooms available to be booked each day.
int globalRooms = 0;

// Semaphore to prevent concurrent access of booking information.
sem_t globalSem;

/** Print out an error message and exit. */
static void fail( char const *message ) {
  fprintf( stderr, "%s\n", message );
  exit( EXIT_FAILURE );
}

/** handle a client connection, close it when we're done. */
void *handleClient( void *sock ) {
  // Here's a nice trick, wrap a C standard IO FILE around the
  // socket, so we can communicate the same way we would read/write
  // a file.
  FILE *fp = fdopen( *( (int *)sock ), "a+" );
  
  // Prompt the user for a command.
  fprintf( fp, "cmd> " );

  // Boolean indicating if the command is invalid.
  bool invalidCmd = false;

  // Temporary values for parsing commands.
  char cmd[ N ];
  while ( fscanf( fp, "%s", cmd ) == 1 &&
          strncmp( cmd, "quit", 4 ) != 0 ) {

    // Acquire sempahore to prevent other threads from accessing critical section.
    sem_wait( &globalSem );

    invalidCmd = false;

    // Perform these operations if the command is book.
    if ( strncmp( cmd, "book", 4 ) == 0 ) {
      // String representing the day.
      char day[ N ] = "";
      // String representing the room.
      char room[ N ] = "";
      // String representing the number of days.
      char numDays[ N ] = "";
      // String representing the character input.
      char characterInput[ N ] = "";

      // Current character processed
      char curChar = ' ';
      // Number of spaces found
      int spaceCount = 0;
      // Boolean indicating if there is a character after a space.
      bool letterBoolean = true;
      // Boolean indicating if the current character is the last one.
      bool lastVar = false;
      // Current index used to prevent buffer overflow.
      int index = 0;

      // Day index used to parse day strings to ints.
      int dayIndex = 0;
      // Room index used to parse room strings to ints.
      int roomIndex = 0;
      // Number days index used to parse number of day strings to ints.
      int numDaysIndex = 0;

      // Read each character from the file input, quit reading at the end of the line.
      while ( ( curChar = fgetc( fp ) ) != '\n' ) {
        index++;
        // If the length of the input is longer than allowed stop reading.
        if ( index >= CMD_LENGTH ) {
          invalidCmd = true;
          break;
        }
        // Count number of spaces in the input character line.
        if ( curChar == ' ' ) {
          if ( letterBoolean == false ) {
            invalidCmd = true;
            break;
          }
          spaceCount++;
          letterBoolean = false;
        }
        else if ( curChar == '\n' ) {
          break;
        }
        else {
          letterBoolean = true;
          // Update the day if the space is the first space.
          if ( spaceCount == 1 ) {
            day[ dayIndex ] = curChar;
            day[ dayIndex + 1 ] = '\0'; 
            dayIndex++;
          }
          // Update the room if the space is the second space.
          else if ( spaceCount == 2 ) {
            room[ roomIndex ] = curChar;
            room[ roomIndex + 1 ] = '\0'; 
            roomIndex++;
          }
          // Update the number of days if the space is the third space.
          else if ( spaceCount == 3 ) {
            numDays[ numDaysIndex ] = curChar;
            numDays[ numDaysIndex + 1 ] = '\0'; 
            numDaysIndex++;
          }
          // Update the character input if the space is the final space and break after this input.
          else if ( !lastVar && spaceCount == 4 ) {
            characterInput[ 0 ] = curChar;
            lastVar = true;
            // break;
          }
        }
      }

      // The input is invalid if there are not at least four spaces.
      if ( spaceCount < 4 ) {
        invalidCmd = true;
      }

      // If the command is invlaid, print invalid command to the socket, close the semaphore, and continue.
      if ( invalidCmd ) {
        fprintf( fp, "Invalid command\n" );
        sem_post( &globalSem );
        fprintf( fp, "cmd> " );
        continue;
      }

      int dayInt = atoi( day );
      int roomInt = atoi( room );
      int numDaysInt = atoi( numDays );
      char character = characterInput[ 0 ];

      // If the day, room, number of days are out of bounds, or if the character is not valid, print invalid command
      if ( dayInt > globalDays || roomInt > globalRooms || numDaysInt <= 0 || dayInt < 1 || roomInt < 1 || !isalpha(character) || character == ' ' ) {
        fprintf( fp, "Invalid command\n" );
        sem_post( &globalSem );
        fprintf( fp, "cmd> " );
        continue;
      }

      // If the days and number of days are out of bounds print invalid command.
      if ( ( dayInt + numDaysInt - 1 ) > globalDays ) {
        fprintf( fp, "Invalid command\n" );
        sem_post( &globalSem );
        fprintf( fp, "cmd> " );
        continue;
      }

      // If the room number is out of bounds print invalid command.
      if ( ( roomInt ) > globalRooms ) {
        fprintf( fp, "Invalid command\n" );
        sem_post( &globalSem );
        fprintf( fp, "cmd> " );
        continue;
      }

      // Boolean used to determine if the space on the board is valid or not.
      bool unavailable = false;

      for( int i = dayInt - 1; i < dayInt - 1 + numDaysInt; i++ ) {
        // If the space on the board is not valid then change to unavailable and break from the program.
        if ( bookingBoard[ roomInt - 1 ][ i ] != ' ' ) {
          unavailable = true;
        }

        if ( unavailable ) {
          break;
        }
      }

      // If the board space is unavaible then print that the room is not available
      if ( unavailable ) {
        fprintf( fp, "Room is not available\n" );
        sem_post( &globalSem );
        fprintf( fp, "cmd> " );
        continue;
      }

      // Update the associated spaces on the board with the inputted character.
      for( int i = dayInt - 1; i < ( dayInt - 1 + numDaysInt ); i++ ) {
        bookingBoard[ roomInt - 1 ][ i ] = character;
      }
    }
    // Perform these operations if the command in cancel.
    else if ( strncmp( cmd, "cancel", 6 ) == 0 ) {

      // String representing the day.
      char day[ N ] = "";
      // String representing the room.
      char room[ N ] = "";
      // String representing the character input.
      char characterInput[ N ] = "";

      // Current character processed
      char curChar = ' ';
      // Number of spaces found
      int spaceCount = 0;
      // Boolean indicating if there is a character after a space.
      bool letterBoolean = true;
      // Boolean indicating if the current character is the last one.
      bool lastVar = false;
      // Current index used to prevent buffer overflow.
      int index = 0;

      // Day index used to parse day strings to ints.
      int dayIndex = 0;
      // Room index used to parse room strings to ints.
      int roomIndex = 0;

      // fgets( message, sizeof( message ), fp );
      // printf( "\n%s\n", message );

      // Read each character from the file input, quit reading at the end of the line.
      while ( ( curChar = fgetc( fp ) ) != '\n' ) {
        index++;
        // If the length of the input is longer than allowed stop reading.
        if ( index >= CMD_LENGTH ) {
          invalidCmd = true;
          break;
        }
        // Count number of spaces in the input character line.
        if ( curChar == ' ' ) {
          if ( letterBoolean == false ) {
            invalidCmd = true;
            break;
          }
          spaceCount++;
          letterBoolean = false;
        }
        else if ( curChar == '\n' ) {
          break;
        }
        else {
          letterBoolean = true;
          // Update the day if the space is the first space.
          if ( spaceCount == 1 ) {
            day[ dayIndex ] = curChar;
            day[ dayIndex + 1 ] = '\0'; 
            dayIndex++;
          }
          // Update the room if the space is the second space.
          else if ( spaceCount == 2 ) {
            room[ roomIndex ] = curChar;
            room[ roomIndex + 1 ] = '\0'; 
            roomIndex++;
          }
          // Update the character input if the space is the final space and break after this input.
          else if ( !lastVar && spaceCount == 3 ) {
            characterInput[ 0 ] = curChar;
            lastVar = true;
            // break;
          }
        }
      }


      // If the space count is not at least three then the command is invalid.
      if ( spaceCount < 3 ) {
        invalidCmd = true;
      }

      // If the command is invalid then print out invalid command to the socket and release the semaphore.
      if ( invalidCmd ) {
        fprintf( fp, "Invalid command\n" );
        sem_post( &globalSem );
        fprintf( fp, "cmd> " );
        continue;
      }

      // Integer version of the day.
      int dayInt = atoi( day );
      // Integer version of the room.
      int roomInt = atoi( room );
      // Character version of the character string.
      char character = characterInput[ 0 ];

      // If the day or room valid is out of bounds then print invalid commands and release the semaphore.
      if ( dayInt > globalDays || roomInt > globalRooms || dayInt < 1 || roomInt < 1 ) {
        fprintf( fp, "Invalid command\n" );
        fprintf( fp, "cmd> " );
        sem_post( &globalSem );
        continue;
      }

      dayInt = dayInt - 1;
      roomInt = roomInt - 1;

      // If the board space is not eqaul to the inputted character then print invalid commands and release the semaphore.
      if ( bookingBoard[ roomInt ][ dayInt ] != character ) {
        fprintf( fp, "Invalid command\n" );
        fprintf( fp, "cmd> " );
        sem_post( &globalSem );
        continue;
      }

      // While the booking board character is equal to the specified character, change the character to a space and go to the next day.
      while ( true ) {
        if ( bookingBoard[ roomInt ][ dayInt ] != character ) {
          break;
        }
        bookingBoard[ roomInt ][ dayInt ] = ' ';
        
        dayInt++;
      }
    }
    // Perform these operations if the command is show. The board should be printed to the socket for the client.
    else if ( strncmp( cmd, "show", 4 ) == 0 ) {

      fprintf( fp, "    R" );
      for ( int i = 0; i < globalRooms - 1; i++ ) {
        fprintf( fp, "  R");
      }
      fprintf( fp, "\n" );

      fprintf( fp, "    1" );

      // for each of the roms, print out the room numbers.
      for ( int i = 0; i < globalRooms - 1; i++ ) {
        // Print two spaces if the index is single digit.
        if ( i + 1 < 10 ) {
          fprintf( fp, "  %d", i + 2 );
        }
        // Print two spaces if the index is double digit.
        else {
          fprintf( fp, " %d", i + 2 );
        }
      }
      fprintf( fp, "\n" );

      // For each of the days, print out the day number
      for ( int i = 0; i < globalDays; i++ ) {
        // Print two spaces if the index is single digit.
        if ( i + 1 < 10 ) {
          fprintf( fp, "D%d |", i + 1 );
        }
        // Print two spaces if the index is double digit.
        else {
          fprintf( fp, "D%d|", i + 1 );
        }
        
        // Print out the characters at each room and day based on the values in the board.
        for ( int j = 0; j < globalRooms; j++ ) {
          fprintf( fp, "%c  ", bookingBoard[ j ][ i ] );
        }
        fprintf( fp, "\n" );
      }
    }
    // If the command is invalid then print out invalid command and relase the semaphore
    else {
      fprintf( fp, "Invalid command\n" );
      fprintf( fp, "cmd> " );
      sem_post( &globalSem );
      continue;
    }

    // Release the semaphore so another thread can acquire it.
    sem_post( &globalSem );

    // Just echo the command back to the client for now.
    // fprintf( fp, "%s\n", cmd );

    // Prompt the user for the next command.
    fprintf( fp, "cmd> " );
  }

  // Close the connection with this client.
  fclose( fp );
  return NULL;
}

int main( int argc, char *argv[] ) {
  // Prepare a description of server address criteria.
  struct addrinfo addrCriteria;
  memset(&addrCriteria, 0, sizeof(addrCriteria));
  addrCriteria.ai_family = AF_INET;
  addrCriteria.ai_flags = AI_PASSIVE;
  addrCriteria.ai_socktype = SOCK_STREAM;
  addrCriteria.ai_protocol = IPPROTO_TCP;

  // Lookup a list of matching addresses
  struct addrinfo *servAddr;
  if ( getaddrinfo( NULL, PORT_NUMBER, &addrCriteria, &servAddr) )
    fail( "Can't get address info" );

  // Try to just use the first one.
  if ( servAddr == NULL )
    fail( "Can't get address" );

  // Create a TCP socket
  int servSock = socket( servAddr->ai_family, servAddr->ai_socktype,
                         servAddr->ai_protocol);
  if ( servSock < 0 )
    fail( "Can't create socket" );

  // Bind to the local address
  if ( bind(servSock, servAddr->ai_addr, servAddr->ai_addrlen) != 0 )
    fail( "Can't bind socket" );
  
  // Tell the socket to listen for incoming connections.
  if ( listen( servSock, 5 ) != 0 )
    fail( "Can't listen on socket" );

  // Free address list allocated by getaddrinfo()
  freeaddrinfo(servAddr);

  // Fields for accepting a client connection.
  struct sockaddr_storage clntAddr; // Client address
  socklen_t clntAddrLen = sizeof(clntAddr);

  // If there are not three arguments then print usage message and exit as failure.
  if ( argc != 3 ) {
    printf( "usage: bookingServer <days> <rooms>\n" );
    exit( 1 );
  }

  // Number of possible days.
  globalDays = atoi( argv[ 1 ] );

  // Number of possible rooms
  globalRooms = atoi( argv[ 2 ] );

  // If the number of days or number of rooms are not positive then print usage message and exit as failure.
  if ( !( globalDays > 0 ) || !( globalRooms > 0 ) ) {
    printf( "usage: bookingServer <days> <rooms>\n" );
    exit( 1 );
  }

  // Create the booking board and allocate memory to it.
  bookingBoard = ( char ** ) malloc( globalDays * sizeof( char * ) );
  // Allocate memory for each row of the booking board.
  for ( int i = 0; i < globalDays; i++ ) {
    bookingBoard[ i ] = ( char * ) malloc( globalRooms * sizeof( char ) );
  }

  // Initialize each space on the board to an empty space.
  for ( int i = 0; i < globalDays; i++ ) {
    for ( int j = 0; j < globalRooms; j++ ) {
      bookingBoard[ j ][ i ] = ' ';
    }
  }

  // Initialize the seamphore and set its value to one.
  sem_init( &globalSem, 0, 1 );

  while ( true  ) {

    // Accept a client connection.
    int sock = accept( servSock, (struct sockaddr *) &clntAddr, &clntAddrLen);

    // Create a new variable for the thread.
    pthread_t clientThread;

    // Create the thread and pass in the handleClient function and the thread socket.
    if ( pthread_create( &clientThread, NULL, handleClient, (void *)&sock ) != 0 ) {
      // Detach the thread when completed to free the memory.
      pthread_detach( clientThread );
    }
  }

  // Destroy the semaphore.
  sem_destroy( &globalSem );

  // Stop accepting client connections (never reached).
  close( servSock );
  
  return 0;
}
