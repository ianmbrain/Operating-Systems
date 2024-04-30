// Elapsed Real Time for input-4.txt: 0m1.376s

/** 
    @file maxpalindrome.cu
    @author Ian M Brain (imbrain)
    This program determines the longest palindrome from an input file containing a sequence of characters.
    Utilizes CUDA programming to run palindrome operations on the gpu.
    Users can specify report flag to indicate the maximum palindrome found by each thread.
*/

#include <stdio.h>
#include <stdbool.h>
#include <cuda_runtime.h>

// Input sequence of letters.
char *vList;

// Number of values on the list.
int vCount = 0;

// Capacity of the list of letters.
int vCap = 0;

// General function to report a failure and exit.
static void fail( char const *message ) {
  fprintf( stderr, "%s\n", message );
  exit( 1 );
}

// Print out a usage message, then exit.
static void usage() {
  printf( "usage: maxpalindrome [report]\n" );
  exit( 1 );
}

// Read the list of letters.
__host__ void readList() {
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

// Determines the length of the longest palindrome for the characters the thread is responsible for.
// Runs on the gpu uitilizing CUDA programming.
__global__ void checkLength( int vCount, bool report, char *dList, int *lengthList ) {
  int idx = blockDim.x * blockIdx.x + threadIdx.x;

  if ( idx < vCount ) {
    // ...

    // Index tracking the current character from the end of the list.
    int countDown = 0;

    // Indicates if there is no palindrome and thus the max palindrome should not be set.
    bool noPalindrome = false;

    // Length of the longest palindrome.
    int maxPalindrome = 1;

    /** Citing Help from MaxPalindrome.c that I completed on 2/1/2024
    * The code for this method is based on the main method operation to parse through input file characters to find palindromes from MaxPalindrome.c
    */
    for ( int i = idx + 1; i < vCount; i++ ) {
      noPalindrome = false;

      countDown = 0;

      // Start at the thread index and parse every character sequence starting from the index to the character at index vCount.
      for ( int j = idx; j < ( i - countDown ); j++ ) {

        // If the characters are not equal, set noPalindrom to true and stop comparing the characters.
        if( dList[ j ] != dList[ i - countDown ] ) {
          noPalindrome = true;
          break;
        } 

        countDown++;
      }

      // If there is a palindrome and the current palindrome is longer than the last longest palindrome, set the longest palindrome to this length.
      if ( !noPalindrome && maxPalindrome < ( i - idx + 1 ) ) {
        maxPalindrome = ( i - idx + 1 );
      }
    }

    lengthList[ idx ] = maxPalindrome;
  }
}

int main( int argc, char *argv[] ) {
  if ( argc < 1 || argc > 2 )
    usage();

  // If there's an argument, it better be "report"
  bool report = false;
  if ( argc == 2 ) {
    if ( strcmp( argv[ 1 ], "report" ) != 0 )
      usage();
    report = true;
  }

  readList();

  // Add code to allocate memory on the device and copy over the list.

  // List on the device that will hold the copied values from the host input files.
  char *dList;

  /* Citing Help from Course Website
  * The code for allocating device memory is based on slide "Preparing Device Input" from the "Intro to CUDA Programming" lecture.
  */
  cudaMalloc( ( void ** ) &dList, vCount * sizeof( char ) );

  // Add code to copy the list over to the device.

  /* Citing Help from Course Website
  * The code for copying host memory to the device is based on slide "Preparing Device Input" from the "Intro to CUDA Programming" lecture.
  */
  // Copy the list of characters from the host to the memory in device.
  cudaMemcpy( dList, vList, vCount * sizeof( char ), cudaMemcpyHostToDevice );

  // Add code to allocate space on the device to hold the results.

  // List on the device that will hold lengths of palindromes
  int *lengthList;

  /* Citing Help from Course Website
  * The code for allocating device memory is based on slide "Preparing Device Input" from the "Intro to CUDA Programming" lecture.
  */
  cudaMalloc( ( void ** ) &lengthList, vCount * sizeof( int ) );
  

  // Block and grid dimensions.
  int threadsPerBlock = 100;
  // Round up for the number of blocks we need.
  int blocksPerGrid = ( vCount + threadsPerBlock - 1 ) / threadsPerBlock;

  // Run our kernel on these block/grid dimensions (you'll need to add some parameters)
  checkLength<<<blocksPerGrid, threadsPerBlock>>>( vCount, report, dList, lengthList );
  if ( cudaGetLastError() != cudaSuccess )
    fail( "Failure in CUDA kernel execution." );

  // Add code to copy results back to the host, compare the local maximum lengths
  // and report the final maximum length

  // Allocate memory for the palindrome lengths on the host.
  int *results = ( int * ) malloc( vCount * sizeof( int ) );

  /* Citing Help from Course Website
  * The code for copying device memory to the host is based on slide "Recovering Device Output" from the "Intro to CUDA Programming" lecture.
  */
  // Copy the palindrome length memory from the device to the host.
  cudaMemcpy( results, lengthList, vCount * sizeof( int ), cudaMemcpyDeviceToHost );

  // Maximum length of all the palindromes.
  int maxLength = 1;

  // For each thread, determine the maximum length of all the threads and report the max length of that thread if the report argument is specified.
  for ( int i = 0; i < vCount; i++ ) {
    if ( results[ i ] > maxLength ) {
      maxLength = results[ i ];
    }

    if ( report ) {
      printf( "I’m thread %d. Max length found: %d.\n", i, results[ i ] );
    }
  }

  // Print the maximum length of all the threads
  printf( "Maximum Length: %d\n", maxLength );

  

  // Free memory on the device and the host.
  // ...
  free( vList );
  free( results );
  cudaFree( dList );
  cudaFree( lengthList );

  cudaDeviceReset();

  return 0;
}
