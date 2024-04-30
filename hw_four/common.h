/** 
    @file common.h
    @author Ian M Brain (imbrain)
    Common header file used to specify the puzzle board struct, semaphore name, and other variables.
*/

// Name for the semaphore used to protect access to
// shared state.
#define LOCK_NAME "/imbrain-peg-lock"

// Height and width of the playing area.
#define GRID_SIZE 8

// Length of the undo history.
#define UNDO_SIZE 3

// Direction options for the move.  These are used by two functions
// in peg.c
#define RIGHT_DIR 0
#define DOWN_DIR 1
#define LEFT_DIR 2
#define UP_DIR 3

struct Puzzle {
  /** Eight by eight array representing the puzzle board. */
  char puzzleBoard[ GRID_SIZE ][ GRID_SIZE + 1 ];

  /** Array containing most recent three moves. */
  char undoHistory[ UNDO_SIZE ][ 3 + 1 ];
} typedef Puzzle;
