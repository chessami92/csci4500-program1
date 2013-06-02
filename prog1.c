/* A simple shell runner.                              */
/* Josh DeWitt                                         */
/*                                                     */
/* Entry point for program 1. Gets a line of input and */
/* executes accordingly until the EOF is reached.      */
/* Commands can be up to 100 characters, with a max    */
/* of 16 words of at most 64 characters.               */

#define MAX_LINE_LENGTH 100
#define MAX_WORDS 16
#define MAX_WORD_LENGTH 64

//Main function for the shell. Gets input, parses it, then executes it.
int main(int argc, char *argv[]) {
    char line[MAX_LINE_LENGTH + 1];
    char *words[MAX_WORDS + 1];

    while ( 1 ) { 
        if( !getLine( line, MAX_LINE_LENGTH ) ) { /* Get command. */
            return( 0 );        /* EOF reached, terminate program. */
        }

        /* Parse into individual words. */
        if( !parse( MAX_WORDS, MAX_WORD_LENGTH, line, words ) ) {
            continue;           /* User already notified of error. */
        }
        
        execute( words );       /* Execute the parsed commands */
    }
}
