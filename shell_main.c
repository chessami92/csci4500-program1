/* Josh DeWitt                                         */
/* Entry point for program 1. Gets a line of input and */
/* executes accordingly until the EOF is reached.      */

#define MAX_LINE_LENGTH 100
#define MAX_WORDS 16
#define MAX_WORD_LENGTH 64

//Main function for the shell. Gets input, parses it, then executes it.
int main(int argc, char *argv[]) {
    char line[MAX_LINE_LENGTH + 1];
    char *words[MAX_WORDS];

    while ( 1 ) { 
        if( !getLine( line, MAX_LINE_LENGTH ) ) { /* Get command. */
            return( 0 );        /* EOF reached, terminate program. */
        }

        /* Parse into individual words. */
        if( !parse( MAX_WORDS, MAX_WORD_LENGTH, line, words ) ) {
            continue;           /* Do not notify user - they were given the issue already. */
        }
        
        execute( words );       /* Execute the parsed commands */
    }
}
