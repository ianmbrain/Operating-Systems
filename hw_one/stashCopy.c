#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

// The user can type a command of up to 1024 characters in length (not counting line termination or any
// null termination).

// array of pointers

// external commands are run using fork

/** Maximum number of characters in the command. */
#define MAX_COMMAND_LENGTH 1024

/** Maximum number of words in the command. */
#define MAX_COMMAND_WORDS 513

int parseCommand( char *line, char *words[] )
{
    // This function takes a user command (line) as input. As described above it breaks the line into
    // individual words, adds null termination between the words so each word is a separate string, and
    // it  lls in a pointer in the words array to point to the start of each word. It returns the number
    // of words found in the given line. The words array should be at least 513 elements in length, so it
    // has room for the largest possible number of words that could  t in a 1024-character input line.

    // Determine the length of the user input.
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

        // Update the first word and point to the first word of the line when the first non-space character is found.
        words[ word_count ] = &( line[ i ] );
        word_count++;
        first_word = true;
    }

    for ( int i = first_index; i < cmd_length; i++ ) {
        //printf( "\n%d\n", word_count );

        // Continue to the next loop iteration if the current space is in succession of another.
        if ( line[ i ] == ' ' && one_space ) {
            continue;
        }
        // Set the word pointer to the word if the character is not a space yet a space has been identified.
        else if ( line[ i ] != ' ' && one_space ) {
            // Point to the start of the word at the current index.
            words[ word_count ] = &( line[ i ] );
            word_count++;

            // Indicate that the sequence of spaces is finished.
            one_space = false;
            // printf( "\nFIRST_CHAR\n" );
        }
        // If the current character is a space, change the space to a null termination and point to the current index.
        else if ( line[ i ] == ' ' && !one_space ) {
            line[ i ] = '\0';
            one_space = true;

            // Point to the start of the word at the current index.
            // words[ word_count ] = &( line[ i + 1 ] );
            // word_count++;
            //printf( "\nTRUE\n" );
        }

        // printf( "\n%c\n", line[ i ] );
    }

    // Return the number of words ofund in the input line.
    return word_count;
}

void runExit( char *words[], int count )
{
    // This function performs the built-in exit command. The words array is the list of pointers to words
    // in the user's command and count is the number of words in the array.
    
    // printf( "\n%d\n", count );

    if ( count > 2 || count < 2 ) {
        printf( "Invalid command\n" );
        return;
    }
    // printf( "\nTRUE\n" );

    int status = 0;
    int status_valid = sscanf( words[ 1 ], "%d", &status );

    if ( status_valid != 1 ) {
        printf( "Invalid command\n" );
        return;
    }

    exit( status );
}

void runCd( char *words[], int count )
{
    if ( count > 2 || count < 2 ) {
        printf( "Invalid command\n" );
        return;
    }

    int valid_cmd = 0;
    valid_cmd = chdir( words[ 1 ] );

    if ( valid_cmd == 0 ) {
        printf( "Invalid command\n" );
        return;
    }
}

// CITE lecture 1____ -_____--__----_----_-_---_-_--_---_---____--__--_-_--
void runCommand( char *words[], int count )
{
    // This function runs a (non-built-in) command by creating a child process and having it call execvp()
    // to run the given command.

    //printf( "\n%ld\n", strlen( words[ 0 ] ) );
    // if ( count == 1 ) {
    //     printf( "\n%d\n", strlen( words[ 0 ] ) );
    // }
    static pid_t bg_id = -1;

    if ( strcmp( "&", words[ count - 1 ] ) == 0 ) {
        bg_id = fork();

        if ( bg_id == 0 ) {
            // create and array of rest of words and add NULL as last pointer __---_-_-_---_--_-_- CHANGE
            char *param_words[ count + 1 ];
            int idx = 0;
            
            while ( idx < count - 1 ) {
                param_words[ idx ] = words[ idx ];
                idx++;
            }

            param_words[ idx ] = NULL;

            // printf( "\n%d\n", ch_id );

            // int test = 0;
            // while ( test < count ) {
            //     printf( "\n%s\n", param_words[ test ] );
            //     test++;
            // }

            // printf( "\n%s\n", param_words[ 1 ] );

            if ( execvp( words[ 0 ], param_words ) == -1 ) {
                printf( "%s%s", "Can't run command ", words[ 0 ] );
                printf( "\n" );
            }

            // printf( "\n%d\n", ch_id );

            // // is this right? __-----_____---____---___--__---__--___--_---_--__-
            // printf( "\n%s%d%s\n", "[", ch_id, " done]" );
            // exit( 0 );
        }
        else {
            printf( "%s%d%s\n", "[", bg_id, "]" );
            return;
        }
    }
    // If an ampersand in not included at the end of the command run the comman non-concurrently.
    else {
        // printf( "\nHERE\n" );
        // printf( "%s%d%s\n", "[", bg_id, "]" );
        // cite this somehow (maybe in book) __----___---____---___--___--___-_-_--_-_-__-_-_-_--_-_-_-
        int temp = 0;
        int ch_terminate = waitpid(bg_id, &temp, WNOHANG);
        if ( bg_id != -1 && ch_terminate != -1 && ch_terminate != 0 ) {
            printf( "%s%d%s\n", "[", bg_id, " done]" );
            bg_id = -1;
        }

        pid_t id = fork();

        if ( id == 0 ) {
            // create and array of rest of words and add NULL as last pointer __---_-_-_---_--_-_- CHANGE
            char *param_words[ count + 1 ];
            int idx = 0;
            
            while ( idx < count ) {
                param_words[ idx ] = words [ idx ];
                idx++;
            }

            param_words[ idx ] = NULL;

            // int test = 0;
            // while ( test < count ) {
            //     printf( "\n%s\n", param_words[ test ] );
            //     test++;
            // }

            // printf( "\n%s\n", param_words[ 1 ] );

            if ( execvp( words[ 0 ], param_words ) == -1 ) {
                printf( "%s%s", "Can't run command ", words[ 0 ] );
                printf( "\n" );
            }

            // is this right? __-----_____---____---___--__---__--___--_---_--__-
            exit( 0 );
        }
        else {
            wait( NULL );
        }
    }

    // pid_t id = fork();

    // if ( id == 0 ) {
    //     // create and array of rest of words and add NULL as last pointer __---_-_-_---_--_-_- CHANGE
    //     char *param_words[ count + 1 ];
    //     int idx = 0;
        
    //     while ( idx < count ) {
    //         param_words[ idx ] = words [ idx ];
    //         idx++;
    //     }

    //     param_words[ idx ] = NULL;

    //     // int test = 0;
    //     // while ( test < count ) {
    //     //     printf( "\n%s\n", param_words[ test ] );
    //     //     test++;
    //     // }

    //     // printf( "\n%s\n", param_words[ 1 ] );

    //     if ( execvp( words[ 0 ], param_words ) == -1 ) {
    //         printf( "%s%s", "Can't run command ", words[ 0 ] );
    //         printf( "\n" );
    //     }

    //     // is this right? __-----_____---____---___--__---__--___--_---_--__-
    //     exit( 0 );
    // }
    // else {
    //     wait( NULL );
    // }
}

int main( int argc, char *argv[] )
{
    // User command
    char cmd[ MAX_COMMAND_LENGTH + 1 ] = "";

    // Pointer array to each word in the command.
    char *cmd_words[ MAX_COMMAND_WORDS ] = { NULL };

    int word_count = 0;

    while ( true ) {
        printf( "stash> " );

        // Read in the user command.
        // scanf( " %[^\n]", cmd );

        // try reading just a character at a time
        int read_bool = 0;
        int char_read = 0;
        char cur_char = ' ';

        read_bool = scanf( "%c", &cur_char );

        while ( read_bool ) {
            if ( cur_char == '\n' )
                break;

            cmd[ char_read ] = cur_char;
            cmd[ char_read + 1 ] = '\0';
            char_read++;

            read_bool = scanf( "%c", &cur_char );
            // printf( "\n%c\n", cur_char );
        }

        // printf( "\n%s\n", cmd );

        // Parse the command to point to each word and replace spaces with null temination.
        word_count = parseCommand( cmd, cmd_words );

        //printf( "\n%d\n", word_count );
        // printf( "\n%s\n", cmd_words[ 0 ] );
        // if ( word_count == 0 ) {
        //     continue;
        // }
        if ( strcmp( "exit", cmd_words[ 0 ] ) == 0 ) {
            runExit( cmd_words, word_count );
            continue;
        }
        else if ( strcmp( "cd", cmd_words[ 0 ] ) == 0 ) {
            runCd( cmd_words, word_count );
            continue;
        }
        else {
            runCommand( cmd_words, word_count );
            continue;
        }
        // Also check if command is empty __---__-_-_-_---__--__-_-_-

    }
}




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

void runExit( char *words[], int count )
{
    // Print invalid command if there are an invalid number of arguments in the command.
    if ( count > 2 || count < 2 ) {
        printf( "Invalid command\n" );
        return;
    }

    // Convert the exit status argument to an integer.
    int status = 0;
    int status_valid = sscanf( words[ 1 ], "%d", &status );

    // Print invalid command if the exit status argument could not be parsed.
    if ( status_valid != 1 ) {
        printf( "Invalid command\n" );
        return;
    }

    // Exit program using the provided exit argument.
    exit( status );
}

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

// CITE lecture 1____ -_____--__----_----_-_---_-_--_---_---____--__--_-_--
void runCommand( char *words[], int count )
{
    printf( "\n%d\n", count );
    printf( "\n%s\n", words[ 0 ] );
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
        // cite this somehow (maybe in book) __----___---____---___--___--___-_-_--_-_-__-_-_-_--_-_-_-
        int temp = 0;
        // Determine if the child process has finished.
        int ch_terminate = waitpid(bg_id, &temp, WNOHANG);
        
        // Print out child id if the background child process has finished.
        if ( bg_id != -1 && ch_terminate != -1 && ch_terminate != 0 ) {
            printf( "%s%d%s\n", "[", bg_id, " done]" );
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

int main( int argc, char *argv[] )
{
    // User command.
    char cmd[ MAX_COMMAND_LENGTH + 1 ] = "";

    // Pointer array to each word in the command.
    char *cmd_words[ MAX_COMMAND_WORDS ] = { NULL };

    // Number of words in the user command.
    int word_count = 0;

    while ( true ) {
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
            char_read++;

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
            printf( "\nHERE\n" );
            runCommand( cmd_words, word_count );
            system("clear");
            continue;
        }
        // Also check if command is empty __---__-_-_-_---__--__-_-_-
    }
}
