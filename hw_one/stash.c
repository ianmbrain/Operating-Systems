/** 
    @file stash.c
    @author Ian M Brain (imbrain)
    This program is a custom shell command-line interface.
    Reads user commands from standard input and perfroms the functions of a typical command-line.
    Non-built in commands are run as separate programs.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

/** Maximum number of characters in the command. */
#define MAX_COMMAND_LENGTH 1024

/** Maximum number of words in the command. */
#define MAX_COMMAND_WORDS 513

/**
 * Parses the command and sets each word of the comman in the array of pointers provided.
 * @param line command to be parsed
 * @param word array of pointers to words in the command
*/
int parseCommand( char *line, char *words[] )
{
    // Length of the user command.
    int cmd_length = 0;
    cmd_length = strlen( line );

    // Index of the current word in the line.
    int word_count = 0;
    // Boolean to determine if the first word has been reached.
    bool first_word = false;
    // Index where the first word begins
    int first_index = 0;
    // Boolean to check if a space is in sucession of another.
    bool one_space = false;

    // Point to the first word in the line.
    for ( int i = 0; !first_word; i++ ) {
        // Continue searching through the line if the first indexes are spaces.
        if ( line[ i ] == ' ' ) {
            first_index++;
            continue;
        }

        // Update the first word boolean and point to the first word of the line when the first non-space character is found.
        words[ word_count ] = &( line[ i ] );
        word_count++;
        first_word = true;
    }

    // Iterate through the command and point to the start of each word.
    // Add null termination at the end of each word.
    for ( int i = first_index; i < cmd_length; i++ ) {
        // Continue to the next loop iteration if the current space is in succession of another.
        if ( line[ i ] == ' ' && one_space ) {
            continue;
        }
        // Set the word pointer to the word if the character is not a space yet a space has been identified indicating this is a different word.
        else if ( line[ i ] != ' ' && one_space ) {
            // Point to the start of the word at the current index.
            words[ word_count ] = &( line[ i ] );
            word_count++;

            // Indicate that the sequence of spaces is finished.
            one_space = false;
        }
        // If the current character is a space, change the space to a null termination and point to the current index.
        else if ( line[ i ] == ' ' && !one_space ) {
            line[ i ] = '\0';
            one_space = true;
        }
    }

    // Return the number of words found in the inputted command.
    return word_count;
}

/**
 * Runs the exit command
 * Prints Invalid Command if there are an invalid number of commands or the exit status cannot be parsed to an integer.
 * @param words array of pointers to words in the command
 * @param count number of words in the command
*/
void runExit( char *words[], int count )
{
    // Print invalid command if there are an invalid number of arguments in the command.
    if ( count > 2 || count < 2 ) {
        printf( "Invalid command\n" );
        return;
    }

    // Convert the exit status argument to an integer.
    int status = 0;
    // Used to check if there are any non integer characters after the digits.
    char char_check = ' ';
    int status_valid = sscanf( words[ 1 ], "%d%c", &status, &char_check );

    // Print invalid command if the exit status argument could not be parsed.
    if ( status_valid != 1 ) {
        printf( "Invalid command\n" );
        return;
    }

    // Exit program using the provided exit argument.
    exit( status );
}

/**
 * Runs the cd command
 * Prints Invalid Command if there are an invalid number of commands or the path is invalid.
 * @param words array of pointers to words in the command
 * @param count number of words in the command
*/
void runCd( char *words[], int count )
{
    // Print invalid command if there are an invalid number of arguments in the command.
    if ( count > 2 || count < 2 ) {
        printf( "Invalid command\n" );
        return;
    }

    // Change the directory using the path word.
    int valid_cmd = 0;
    valid_cmd = chdir( words[ 1 ] );

    // Print invalid command if the path is invalid.
    if ( valid_cmd == -1 ) {
        printf( "Invalid command\n" );
        return;
    }
}

/** Citing Help from course slides
 * The code for this method is based on the implementation from the class slides discussed on 1/9/2024.
 * 
 * Runs commands other than the commands expressed in other functions.
 * Commands are run in the background as children processes.
 * The parent process waits for the child process to finish by default.
 * If an ambersand in the last word in the command the processes are run non-concurrently.
 * Prints Invalid Command if there are an invalid number of commands or the exit status cannot be parsed to an integer.
 * @param words array of pointers to words in the command
 * @param count number of words in the command
 */
void runCommand( char *words[], int count )
{
    // Background process id. This is a static global variable so that after it is run -1 is not reset to -1.
    static pid_t bg_id = -1;

    // If the last word is '&' then run the child process concurrently.
    if ( strcmp( "&", words[ count - 1 ] ) == 0 ) {
        // Fork the processes.
        bg_id = fork();

        // If the child process cannot be forked print output indicating so.
        if ( bg_id == -1 ) {
            printf( "%s%s", "Can't run command ", words[ 0 ] );
            printf( "\n" );
        }

        // Child process.
        if ( bg_id == 0 ) {
            // Create an array of the command words.
            char *param_words[ count + 1 ];
            int idx = 0;
            
            // Copy the commmand words to the new array.
            while ( idx < count - 1 ) {
                param_words[ idx ] = words[ idx ];
                idx++;
            }

            // Set the las command word to NULL.
            param_words[ idx ] = NULL;

            // Run the provided command. Print that the command cannot be run if it can't be run.
            if ( execvp( words[ 0 ], param_words ) == -1 ) {
                printf( "%s%s", "Can't run command ", words[ 0 ] );
                printf( "\n" );
                exit( -1 );
            }
        }
        // Parent process.
        else {
            // Print out the child process id and continue running without waiting.
            printf( "%s%d%s\n", "[", bg_id, "]" );
            return;
        }
    }
    // If an ampersand in not included at the end of the command run the comman non-concurrently.
    else {
        // Determine if the child process has finished.
        int ch_terminate = waitpid( bg_id, NULL, WNOHANG );
        
        // Print out child id if the background child process has finished.
        if ( bg_id != -1 && ch_terminate != -1 && ch_terminate != 0 ) {
            printf( "%s%d%s\n", "[", ch_terminate, " done]" );
            bg_id = -1;
        }

        // Fork the child process.
        pid_t id = fork();

        // If the child process cannot be forked print output indicating so.
        if ( id == -1 ) {
            printf( "%s%s", "Can't run command ", words[ 0 ] );
            printf( "\n" );
        }

        // If the process is the child.
        if ( id == 0 ) {
            // create and array of rest of words with space for the last pointer to be NULL.
            char *param_words[ count + 1 ];
            int idx = 0;
            
            // Copy the words to the new array.
            while ( idx < count ) {
                param_words[ idx ] = words [ idx ];
                idx++;
            }

            // Set the last index of the words to NULL.
            param_words[ idx ] = NULL;

            // Run the provided command. Print that the command cannot be run if it can't be run.
            if ( execvp( words[ 0 ], param_words ) == -1 ) {
                printf( "%s%s", "Can't run command ", words[ 0 ] );
                printf( "\n" );
                exit( -1 );
            }
        }
        // Wait for the child process to finish as the parent.
        else {
            wait( NULL );
        }
    }
}

/**
 * Main function of the stash program
 * Reads a command from standard input and parses the command.
 * Runs the command using the runExit(), runCd(), and runCommand() functions.
 * A new command is read if the the command line is empty.
 * @param argc number of command line arguments
 * @param argv array of command line arguments
*/
int main( int argc, char *argv[] )
{
    while ( true ) {
        // User command.
        char cmd[ MAX_COMMAND_LENGTH + 1 ] = "";

        // Pointer array to each word in the command.
        char *cmd_words[ MAX_COMMAND_WORDS ] = { NULL };

        // Number of words in the user command.
        int word_count = 0;

        // Print the stash prompt for commands.
        printf( "stash> " );

        // Boolean indicating if an additional character has been read from standard input.
        int read_bool = 0;
        // Current index of the next character in the command.
        int char_read = 0;
        // Current character read.
        char cur_char = ' ';

        // Scan user input for the first character.
        read_bool = scanf( "%c", &cur_char );

        while ( read_bool ) {
            // Break the loop at the end of the command.
            if ( cur_char == '\n' )
                break;

            // Set the next character in the command to the read command and increase the character index.
            cmd[ char_read ] = cur_char;
            cmd[ char_read + 1 ] = '\0';
            if ( cur_char != '\n' && cur_char != '\0' )
                char_read++;
            
            // Limit command to 1024 characters not include line or null termination.
            if ( char_read >= 1024 )
                break;

            // Scan for the next character.
            read_bool = scanf( "%c", &cur_char );
        }

        // Parse the command to point to each word and replace spaces with null temination.
        word_count = parseCommand( cmd, cmd_words );

        if ( strcmp( "", cmd_words[ 0 ] ) == 0 ) {
            continue;
        }
        // Run the exit command if the command is exit.
        if ( strcmp( "exit", cmd_words[ 0 ] ) == 0 ) {
            runExit( cmd_words, word_count );
            continue;
        }
        // Run the cd command if the command is cd.
        else if ( strcmp( "cd", cmd_words[ 0 ] ) == 0 ) {
            runCd( cmd_words, word_count );
            continue;
        }
        // Otherwise, run the arbitrary command.
        else {
            runCommand( cmd_words, word_count );
            continue;
        }
    }
}
