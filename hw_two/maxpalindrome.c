/** 
    @file maxpalindrome.c
    @author Ian M Brain (imbrain)
    This program finds the max palindrome from the specified input file.
    Users can ask the program to use multiple processes to make the process faster.
    The program returns maximum length a palindrome in the file.
    Helper process can also report the largest palindrome they find.
*/

#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <limits.h>
#include <stdbool.h>

// Print out an error message and exit.
static void fail( char const *message ) {
  fprintf( stderr, "%s\n", message );
  exit( 1 );
}

// Print out a usage message, then exit.
static void usage() {
  printf( "usage: maxpalindrome <workers>\n" );
  printf( "       maxpalindrome <workers> report\n" );
  exit( 1 );
}

// Input sequence of letters.
char *vList;

// Number of letters on the list.
int vCount = 0;

// Capacity of the list of letters.
int vCap = 0;

// Read the list of letters.
void readList() {
  // Set up initial list and capacity.
  vCap = 5;
  vList = (char *) malloc( vCap * sizeof( char ) );

  // Keep reading as many values as we can.
  char v;
  while ( scanf( "%c\n", &v ) == 1 ) {
    // Grow the list if needed.
    if ( vCount >= vCap ) {
      vCap *= 2;
      vList = (char *) realloc( vList, vCap * sizeof( char ) );
    }

    // Store the latest value in the next array slot.
    vList[ vCount++ ] = v;
  }
}

/**
 * Check for a palindrome within the specified range from index i up to j. 
 * @param i first index
 * @param j second index
 * @return true if string is palindrom and false otherwise.
*/
bool checkPalindrome( int i, int j ) {
  // Index that acts as a counter counting down from the higher index.
  int jCount = 0;

  // Search through the array. Compare the lowest index to the highest index. Return false if any characters do not match.
  for ( int c = i; c < ( j - jCount ); c++ ) {
    // Return false if the characters do not match.
    if( vList[ c ] != vList[ j - jCount ] )
      return false;

    // Otherwise, increment jcount to examine the index before the current j index.
    jCount++;
  }

  // Return true if the characters within the range are a palindrome
  return true;
}

/**
 * Reads in a file and determines the maximum length of palindrome within the file.
 * Utilizes forked child process to perfrom quicker analysis of characters.
 * Returns the size of the palindrome and can display a child report.
 * @param argc number of arguments passed
 * @param argv array of passed arguments
 * @return program exit status
*/
int main( int argc, char *argv[] ) {
  // boolean indicating if a report is specified.
  bool report = false;
  // Number of worker processes.
  int workers = 4;

  // Parse command-line arguments.
  if ( argc < 2 || argc > 3 )
    usage();

  if ( sscanf( argv[ 1 ], "%d", &workers ) != 1 ||
       workers < 1 )
    usage();

  // If there's a second argument, it better be the word, report
  if ( argc == 3 ) {
    if ( strcmp( argv[ 2 ], "report" ) != 0 )
      usage();
    report = true;
  }

  readList();

  // You get to add the rest.

  // ...

  // Longest palindrome in the entire list of characters.
  int parentLongestPalindrome = 0;

  // Create the anonymous pipe to enable interprocess communication.
  int pfd[ 2 ];

  // Fail if the pipe could not be created.
  if ( pipe( pfd ) != 0 )
    fail( "Unable to create pipe" );

  // Create child processes.
  for ( int i = 0; i < workers; i++ ) {
    // Fork into a new child process
    int cid = fork();

    // Throw an error if the child process could not be created.
    if ( cid == -1 )
      fail( "Child process could not be created" );

    if ( cid == 0 ) {
      // Close the reading end of the pipe for the child process.
      close( pfd[ 0 ] );

      // Longest palindrome of the child process.
      int longestPalindrome = 1;
      // Index of the first character in the palindrome.
      int firstChar = 0;
      // Index of the last character in the palindrome.
      int lastChar = 0;

      // Iterate through the input characters based on the worker number.
      for ( int w = i; w < vCount; w += workers ) {
        // Check the range of each character after the worker number character for a palindrome.
        for ( int c = w + 1; c < vCount; c++ ) {
          if ( checkPalindrome( w, c ) ) {
            // Update the longest palindrome if the current palindrome is longer.
            if ( ( c - w + 1 ) >= longestPalindrome ) {
              longestPalindrome = c - w + 1;
              firstChar = w;
              lastChar = c;
            }
          }
        }
      }

      // Print out a report from each child process if report is specified
      if ( report ) {
        // Print out the text, child id, and palindrome length
        printf( "%s%d%s%d%s", "I’m process ", getpid(), ". Max length found: ", longestPalindrome, ". Sequence is: ");

        // Print out each character of the palindrome.
        for ( int ch = firstChar; ch < lastChar + 1; ch++ ) {
          printf( "%c", vList[ ch ] );
        }
        printf( "%s\n", "." );
      }

      // Lock the pipe before communicating through it.
      lockf( pfd[ 1 ], F_LOCK, 0 );

      // Communicate the longest palindrome to the parent process.
      // This needs to be communicated in binary  ___-----____----____-----____----____----____-
      write( pfd[ 1 ], &longestPalindrome, sizeof( longestPalindrome ) );

      // Unlock the pipe after communicating.
      lockf( pfd[ 1 ], F_ULOCK, 0 );

      // Close the writing end of the pipe after communicating the longest palindrome.
      close( pfd[ 1 ] );

      return 0;
    }
  }

  // Wait for each of the children processes to finish.
  for ( int i = 0; i < workers; i++ ) {
    wait( NULL );

    // Close the writing end of the pipe on the parent process.
    close( pfd[ 1 ] );

    // Length of child's palindrome
    int readLength = 0;

    // Read in the child palindrome length
    read( pfd[ 0 ], &readLength, sizeof( readLength ) );

    // Update the longest palindrome length.
    if ( readLength > parentLongestPalindrome )
      parentLongestPalindrome = readLength;
  }

  // Print the longest palindrome length.
  printf( "%s%d%s", "Maximum Length: ", parentLongestPalindrome, "\n" );

  return EXIT_SUCCESS;
}
