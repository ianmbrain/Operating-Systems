/** 
    @file maxpalindrome-sem.c
    @author Ian M Brain (imbrain)
    This program finds the length of the maximum palindrome from an input file.
    Reads in the characters from input file into global string variable.
    As characters are read, worker threads begin to search for palindromes in the string.
    Utilizes anonymous semaphores to provide mutual exclusion.
    Prints the maximum semaphore length and can optionally print a report of semaphores found from each thread.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <pthread.h>
#include <limits.h>
#include <semaphore.h>
#include <sys/syscall.h>

// Print out an error message and exit.
static void fail( char const *message ) {
  fprintf( stderr, "%s\n", message );
  exit( EXIT_FAILURE );
}

// Print out a usage message, then exit.
static void usage() {
  printf( "usage: maxpalindrome-sem <workers>\n" );
  printf( "       maxpalindrome-sem <workers> report\n" );
  exit( 1 );
}

// True if we're supposed to report what we find.
bool report = false;

// Maximum length we've found.
int global_max = 0;

// Fixed-sized array for holding the sequence.
#define MAX_VALUES 500000

// List of the characters read in from the file.
char vList[ MAX_VALUES ];

// Current number of values on the list.
int vCount = 0;

// Index of last character that was checked for palindromes.
int bufferIndex = 0;

// Semaphore indicating there are new characters in the buffer to check.
sem_t buffer_sem;

// Semaphore to control mutual exclusion to the global_max variable.
sem_t max_sem;

// Boolean value indicating all the characters have been read.
bool finishedReading = false;

// Number of workers used to release the buffer semaphore so that threads can receive the special value to know to exit
int global_workers = 0;

// Semaphore to ensure non-concurrent updates to the bufferIndex variable.
sem_t buffer_index_sem;

// Semaphore to ensure that multiple threads do not print at the same time, messing up the format of output
sem_t print_sem;

// Read the list of values.
void readList() {
  // Keep reading as many values as we can.
  char v;
  while ( scanf( "%c\n", &v ) == 1 ) {
    // Make sure we have enough room, then store the latest input.
    if ( vCount > MAX_VALUES )
      fail( "Too many input values" );

    // Store the latest value.
    vList[ vCount++ ] = v;

    // Release the semaphore indicating there is more work
    sem_post( &buffer_sem );
  }

  // Indicate that reading values is finished.
  finishedReading = true;

  for ( int i = 0; i < global_workers; i++ ) {
    // Release the semaphore to allow waiting workers to acquire and finish.
    sem_post( &buffer_sem );
  }
}

/* Citing Help from other assignments
* The code for checking for a palindrome is based on the checkPalindrome function from homework 2 that I completed on 2/1/2024.
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

/** Return the index, b, of the value we're supposed to use (Checking it against all
    earlier values in the list). */
int getWork() {
  // ...

  // Return special value indicating all the characters have been read in and processed.
  if ( finishedReading && bufferIndex >= vCount ) {
    return -1;
  }

  // Wait until values have been read into the buffer.
  sem_wait( &buffer_sem );

  // Return special value indicating all the characters have been read in and processed.
  if ( finishedReading && bufferIndex >= vCount ) {
    return -1;
  }

  // Save and increment the buffer index.
  sem_wait( &buffer_index_sem );
  int returnIndex = bufferIndex;
  bufferIndex++;
  sem_post( &buffer_index_sem );

  // Return the previous index of the buffer
  return returnIndex;
}

/** Start routine for each worker. */
void *workerRoutine( void *arg ) {
  // ...

  // getWork() then do the stuff to actually check for palindrome
  // getWork will have the waiting

  // Index of characters that have been checked.
  int charIndex = 0;

  // Longest palindrome found by the worker thread. The global maximum thread length will be set to this.
  int longestPalindrome = 0;

  // Index of the first character in the palindrome.
  int firstChar = 0;
  // Index of the last character in the palindrome.
  int lastChar = 0;

  // String containing the characters in the palindrome
  char palindromeChars[ MAX_VALUES ] = "";

  while ( true ) {
    charIndex = getWork();

    // If the character index is the sentinel value indicating work is done, update the longest palindrome and finish.
    if ( charIndex == -1 ) {
      // Update the global maximum palindrome with the longest recorded length.
      sem_wait( &max_sem );
      if( longestPalindrome >= global_max ) {
        global_max = longestPalindrome;
      }
      sem_post( &max_sem );

      // If report is specified print out the longest palindrome found.
      if ( report ) {
        // Wait for the print semaphore so that output isn't messed up if two threads try to print at the same time.
        sem_wait( &print_sem );
        printf( "I'm thread %d. ", syscall(__NR_gettid) );
        printf( "Max length found: %d. ", longestPalindrome );
        printf( "Sequence is: %s.\n", palindromeChars );
        sem_post( &print_sem );
      }

      break;
    }

    // Check for palindrome from the first index to the current index.
    for ( int i = 0; i < charIndex; i++ ) {
      if ( checkPalindrome( i, charIndex ) ) {
        // If the new palindrome is longer than the previous longest palindrome, update the longest palindrome variable and string.
        if ( ( charIndex - i + 1 ) >= longestPalindrome ) {
          longestPalindrome = charIndex - i + 1;
          firstChar = i;
          lastChar = charIndex;

          int charCount = 0;
          for ( int c = firstChar; c < lastChar + 1; c++ ) {
            palindromeChars[ charCount ] = vList[ c ];
            charCount++;
          }
        }
      }
    }
  }

  return NULL;
}

int main( int argc, char *argv[] ) {
  // Number of workers the user selects.
  int workers = 4;
  
  // Parse command-line arguments.
  if ( argc < 2 || argc > 3 )
    usage();
  
  if ( sscanf( argv[ 1 ], "%d", &workers ) != 1 ||
       workers < 1 )
    usage();

  // Set the global variables for workers to the number of workers.
  global_workers = workers;

  // Initialize the buffer sempahore to 0.
  sem_init( &buffer_sem, 0, 0 );
  // Initialize the max sempahore to 1.
  sem_init( &max_sem, 0, 1 );
  // Initialize the buffer index sempahore to 1.
  sem_init( &buffer_index_sem, 0, 1 );
  // Initialize the print index sempahore to 1.
  sem_init( &print_sem, 0, 1 );

  // If there's a second argument, it better be "report"
  if ( argc == 3 ) {
    if ( strcmp( argv[ 2 ], "report" ) != 0 )
      usage();
    report = true;
  }

  // Make each of the workers.
  pthread_t worker[ workers ];
  for ( int i = 0; i < workers; i++ )
    // ...

    /* Citing Help from the course website
    * The code for creating the semaphore and checking for failure is based on the race6.c example program from the synchronization section of the course website.
    */
    // Create each worker and fail if the worker cannot be created.
    if ( pthread_create( &(worker[ i ]), NULL, workerRoutine, NULL ) != 0 ) 
      fail( "Cannot create worker thread\n" );

  // Then, start getting work for them to do.
  readList();

  // Wait until all the workers finish.
  for ( int i = 0; i < workers; i++ ) {
    // ...

    pthread_join( worker[ i ], NULL );
  }

  // Report the max and release the semaphores.
  printf( "Maximum Length: %d\n", global_max );
  
  return EXIT_SUCCESS;
}
