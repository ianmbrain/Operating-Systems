#include <stdio.h>
#include <cuda_runtime.h>

// This is the routine that runs on each of our threads as part of a kernel
__global__ void doSomething( int *list, int count ) {

  // Compute a unique index for this thread, based on its
  // location in its block and its block location in its grid.
  int i = blockDim.x * blockIdx.x + threadIdx.x;

  // Since we make threads in uniform-sized blocks, some threads
  // in the last block may not actually have anything to do.
  if ( i < count ) {
    // OK.  Let's do something.
  }
}

// General function to report a failure and exit.
static void fail( char const *message ) {
  fprintf( stderr, "%s\n", message );
  exit( 1 );
}

int main( int argc, char **argv ) {
  // Make up some random numbers.
  int count = 50000;
  int *list = (int *)malloc( count * sizeof(int) );
  for ( int i = 0; i < count; i++ )
    list[ i ] = rand() % 1000;

  // Allocate space on the device to hold a copy of the list.
  int *devList = NULL;
  if ( cudaMalloc((void **)&devList, count * sizeof(int) ) != cudaSuccess )
    fail( "Failed to allocate space for lenght list on device" );

  // Copy the list over to the device.
  if ( cudaMemcpy( devList, list, count * sizeof(int),
                   cudaMemcpyHostToDevice) != cudaSuccess )
    fail( "Failed to copy list to device" );

  // Block and grid dimensions.
  int threadsPerBlock = 100;
  // Round up.
  int blocksPerGrid = ( count + threadsPerBlock - 1 ) / threadsPerBlock;

  // Run our kernel on these block/grid dimensions
  doSomething<<<blocksPerGrid, threadsPerBlock>>>( devList, count );
  if ( cudaGetLastError() != cudaSuccess )
    fail( "Failure in CUDA kernel execution." );

  // Copy the same array back to the host.
  if ( cudaMemcpy( list, devList, count * sizeof(int),
                   cudaMemcpyDeviceToHost) != cudaSuccess )
    fail( "Can't copy list from device to host" );

  // Print out a sample of the resulting values.
  for ( int i = 0; i < count; i += 100 )
    printf( "%d\n", list[ i ] );

  // Free memory on the device and the host.
  cudaFree( devList );
  free( list );

  cudaDeviceReset();
}
