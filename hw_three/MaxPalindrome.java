/** 
    @file MaxPalindrome.java
    @author Ian M Brain (imbrain)
    This program finds the longest palindrome in a sequence of characters using threads.
    Reads in a sequence characters from a file.
    Parses the characters to find the longest palindrome using a specified number of threads.
    Each worker thread prints out a report containing the longest palindrome it found if specified.
*/

import java.util.Scanner;

/* Citing Help from MaxPalindrome.c that I completed on 2/1/2024
* The code for this project is based on the MaxPalindrome.c code.
*/
public class MaxPalindrome {

    /** String read from the specified input file. */
    private static String fileCharacters = new String();

    /** Length of the input file. */
    private static int characterLength = 0;

    /** Longest palindrome in the input file */
    private volatile static int longestPalindrome = 0;

    /** Number of threads to use. */
    private static int numThreads = 1;

    /**
     * Class representing threads used to read palindromes.
     * Each thread contains a rank, longest palindrome read, and the length of that palindrome.
     * Threads check for palindromes in positions based on their rank to allow faster processing with more threads.
     */
    static class PalindromeThread extends Thread {
        // Longest palindrome in the thread finds.
        private String longestPal;
        // Length of the longest palindrome found.
        private int palindromeLength;
        // Rank of the thread.
        private int rank;

        // Construct the thread and set the threads rank
        public PalindromeThread( int rank ) {
            this.rank = rank;
        }

        /* Citing Help from MaxPalindrome.c that I completed on 2/1/2024
         * The code for this method is based on the checkPalindrome method from MaxPalindrome.c
         * Check if a palindrome exists between index a and b.
         * If palindrome does exist return true and otherwise return false
         */
        public boolean checkPalindrome( int a, int b ) {
            // Variable used to count down from the last index checked.
            int countDown = 0;

            // Check characters at the first and last index. If they are the same move to the next indexes. Otherwise return false.
            for ( int i = a; i < ( b - countDown ); i++ ) {
                if( fileCharacters.charAt( i ) != fileCharacters.charAt( b - countDown ) ) {
                    return false;
                }

                // Increment countDown to iterate the last index down.
                countDown++;
            }

            // Return true if palindrome.
            return true;
        }

        /** Citing Help from MaxPalindrome.c that I completed on 2/1/2024
         * The code for this method is based on the main method operation to parse through input file characters to find palindromes from MaxPalindrome.c
         * Function ran by thread on start().
         * Starts at the rank of the thread and checks for palindromes at every offset of the thread number.
         * This enables the program to speed up as more threads are utilized
         */
        public void run() {
            for ( int i = rank; i < characterLength; i += numThreads ) {
                for ( int j = i + 1; j < characterLength; j++ ) {
                    // Check if a palindrome exists between the indexes
                    if( checkPalindrome( i, j ) ) {
                        // If a palindrome exists and is longer than the longest palindrome of the thread, set it as the new thread palindrome.
                        if ( ( j - i + 1 ) > this.palindromeLength ) {
                            this.palindromeLength = ( j - i + 1 );
                            this.longestPal = fileCharacters.substring( i, j + 1 );
                        }
                        // Update the longest palindrome overall.
                        if ( ( j - i + 1 ) > longestPalindrome ) {
                            longestPalindrome = ( j - i + 1 );
                        }
                    }
                }
            }
        }
    }

    /**
     * Main functionality of the program.
     * Reads in the input file as a string.
     * Constructs new threads based on the number specified as command line argument.
     * Finds palindromes and prints out the length of the maximum palindrome.
     * Prints report of palindromes threads found if specified.
     */
    public static void main( String[] args ) {
        // Print message if there are more or less than allowed command line arguments
        if ( args.length < 1 || args.length > 2 ) {
            System.out.println( "Invalid number of command line arguments." );
            System.exit( 1 );
        }
        // Set the number of threads from the command line argument.
        numThreads = Integer.valueOf( args[ 0 ] );
        if ( numThreads < 1 ) {
            System.out.println( "Invalid number of worker threads." );
            System.exit(1);
        }

        // Current character read from the file.
        String currentCharacter = new String();

        Scanner fileScanner = new Scanner(System.in);

        // Read in each character from the file and add it to the file characters string.
        while( fileScanner.hasNextLine() ) {
            currentCharacter = fileScanner.nextLine();

            fileCharacters = fileCharacters + currentCharacter;

            // Do not count empty lines from the file towards the length of the string.
            if ( !currentCharacter.isEmpty() ) {
                characterLength++;
            }
        }

        // Create each thread. Have each thread search for the longest palindrome.
        PalindromeThread[] threadWorkers = new PalindromeThread[ numThreads ];

        /* Citing Help from the course website example files
        * The code to construct new threads and assign them ranks is based of example file ThreadArguments.java from chapter four.
        */
        // Construct each thread. Assign thread unique rank. Start each thread.
        for ( int i = 0; i < numThreads; i++ ) {
            threadWorkers[ i ] = new PalindromeThread( i );
            threadWorkers[ i ].start();
        }

        /* Citing Help from the course website example files
        * The code to join threads and catch exceptions is based of example file ThreadArguments.java from chapter four.
        */
        // Do not print out a report if there is no report argument.
        if( args.length == 1 ) {
            try {
                // Join the threads.
                for ( int i = 0; i < numThreads; i++ ) {
                    threadWorkers[ i ].join();
                }
            } catch ( InterruptedException e ) {
                System.out.println( "Unable to join threads" );
            }
        }
        // Print out report if there is a report argument.
        else if( args.length == 2 ) {
            boolean report = false;
            // Check that the report argument is provided.
            if ( args[ 1 ].equals( "report" ) ) {
                report = true;
            }
            else {
                System.out.println( "Invalid command line argument." );
                System.exit(1);
            }
            try {
                // Join the threads
                for ( int i = 0; i < numThreads; i++ ) {
                    threadWorkers[ i ].join();
                    // Print out the report of each thread.
                    if ( report ) {
                        System.out.println( "I’m worker " + threadWorkers[ i ].rank + ". Max length found: " + threadWorkers[ i ].palindromeLength + ". Sequence is: " + threadWorkers[ i ].longestPal + "." );
                    }
                }
            } catch ( InterruptedException e ) {
                // Print message indicating threads could not be joined if join fails.
                System.out.println( "Unable to join threads" );
            }
        }
        // Print error if there are an invalid number of arguments.
        else {
            System.out.println( "Invalid number of command line arguments" );
            System.exit(1);
        }

        // Print out the length of the longest palindrome found.
        System.out.println( "Maximum Length: " + longestPalindrome );
    }
}
