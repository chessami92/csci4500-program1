#include <string.h>

#define MAX_LINE_LENGTH 100
#define MAX_WORDS 16
#define MAX_WORD_LENGTH 64

//Main function for the shell. Gets input, parses it, then executes it.
int main(int argc, char *argv[]) {
    char line[MAX_LINE_LENGTH + 1];
    char *words[MAX_WORDS];
    char *msg;

    while ( 1 ) { 
        if( !getLine( line, MAX_LINE_LENGTH ) ) { /* Get command. */
            msg = "ERROR: Problem reading command.\n";
            write( 2, msg, strlen( msg ) );
            continue;
        }
 
        /* Parse into individual words. */
        if( !parse( MAX_WORDS, MAX_WORD_LENGTH, line, words ) ) {
            continue;           /* Do not notify user - they were given the issue already. */
        }

        /* Check if use is exiting the shell. */
        if( strcmp( words[0], "exit" ) == 0 ) {
            return( 0 );
        }
        
        execute( words );       /* Execute the parsed commands */
    }
}
