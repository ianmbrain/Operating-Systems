/** 
    @file exclude.c
    @author Ian M Brain (imbrain)
    This program provides functionality to exclude a certain line from an input file and write it to an output file.
    Contains the main method and convertString() helper method to convert the line number to an integer.
    This program does not use the c standard library.
*/

#include <fcntl.h>
#include <unistd.h>

/** Length of invalid error text. */
#define INVALID_ERROR_LENGTH 57

/** Input file argument. */
#define INPUT_FILE 1

/** Output file argument. */
#define OUTPUT_FILE 2

/** Output file argument. */
#define LINE_NUMBER 3

/** Buffer length. */
#define BUFFER_LENGTH 64

/** Error stream. */
#define ERROR_STREAM 2

/** Power of ten. */
#define POWER_10 10

/** Power of ten. */
#define LOWER_ASCII 48

/** Power of ten. */
#define UPPER_ASCII 57

/**
 * Converts the given string to an integer and returns the integer.
 * -1 one is returned if the contains non integer values.
 * @param con_str string to convert from characters to integer.
 * @return integer version of the string or -1 if the string is invalid.
*/
int convertString( char *con_str )
{
    // Determine the string length by calling each character until null termination is reached
    int str_len = 0;
    while ( con_str[ str_len ] ) {
        str_len++;
    }

    // Integer version of the line number converted from the given string.
    int return_val = 0;

    for ( int i = 0; i < str_len; i++ ) {
        // Current line number character.
        int cur_val = 0;

        // Determine the power of the number based on its position.
        int power = 1;
        for ( int j = i; j < str_len - 1; j++ ) {
            power = power * POWER_10;
        }

        // Line number is invalid if the ascii code does not correspond to a digit.
        if ( con_str[ i ] < LOWER_ASCII || con_str[ i ] > UPPER_ASCII ) {
            // Return -1 indicating the character is invalid.
            return -1;
        }

        // Convert character to number.
        cur_val = con_str[ i ] - '0';
        cur_val = cur_val * power;

        // Increase the return value by the current value.
        return_val += cur_val;
    }

    // Return the numeric line number.
    return return_val;
}

/**
 * Main functionality of the program.
 * Read lines from an input file and write them to an output file while excluding the specified line.
 * Command line arguments should include the input file, output file, and line to exclude.
 * Exits as failure and throws an error if an incorrect number of arguments is provided.
 * Exits as failure and throws an error if the line number is not valid.
 * @param argc number of command line arguments.
 * @param argv command line arguments array.
 * @return program exit status.
*/
int main( int argc, char *argv[] )
{
    // Exit as failure if there are an invalid number of command line arguments.
    if ( argc != 4 ) {
        write( ERROR_STREAM, "usage: exclude <input-file> <output-file> <line-number>\n", INVALID_ERROR_LENGTH );
        _exit( 1 );
    }

    // Read the exclude line command line argument.
    char *line = argv[ LINE_NUMBER ];

    // Convert the line number from character to numeric.
    int line_num = 0;
    line_num = convertString( line );

    // Exit as failure if the line number is invalid.
    if ( line_num == -1 ) {
        write( ERROR_STREAM, "usage: exclude <input-file> <output-file> <line-number>\n", INVALID_ERROR_LENGTH );
        _exit( 1 );
    }

    // Exit as failure if the line number is negative.
    if ( line_num < 0 ) {
        write( ERROR_STREAM, "usage: exclude <input-file> <output-file> <line-number>\n", INVALID_ERROR_LENGTH );
        _exit( 1 );
    }

    // Open the input and output files.
    int in_fd = open( argv[ INPUT_FILE ], O_RDONLY, 0600 );
    int out_fd = open( argv[ OUTPUT_FILE ], O_WRONLY, 0600 );

    // Exit as failure if a file cannot be opened
    if ( in_fd == -1 || out_fd == -1 ) {
        write( ERROR_STREAM, "usage: exclude <input-file> <output-file> <line-number>\n", INVALID_ERROR_LENGTH );
        _exit( 1 );
    }
    
    // Buffer that holds 64 read in bytes at a time.
    char read_bufffer[ BUFFER_LENGTH ] = "";
    // Number of bytes read from the input file. Used to determine when the end of file is reached.
    int bytes_read = 0;
    // Number of new line characters read from the input file
    int line_count = 1;

    while ( 1 ) {
        // Read 64 bytes from the input file.
        bytes_read = read( in_fd, read_bufffer, BUFFER_LENGTH );

        for ( int i = 0; i < bytes_read; i++ ) {
            // Write the character to the output file if the line number is not the excluded line.
            if ( line_count != line_num ) {
                write( out_fd, &( read_bufffer[ i ] ), 1 );
            }

            // Increment line_count if the current character is a new line.
            if ( read_bufffer[ i ] == 10 ) {
                line_count++;
            }
        }

        // Break from the loop when the last characters in the file are read.
        if ( bytes_read < 64 )
            break;
    }

    // Close the input and output files.
    close( in_fd );
    close( out_fd );

    // Exit the progam.
    _exit( 0 );
}
