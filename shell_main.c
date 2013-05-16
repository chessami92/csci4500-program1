#define MAX_LINE_LENGTH 100

//Main function for the shell. Gets input, parses it, then executes it.
int main(int argc, char *argv[]) {
    char line[MAX_LINE_LENGTH + 1];

    while ( getLine( line, MAX_LINE_LENGTH ) ) { //Get command.
        /*if (!parse( line ))			// parse into words
            continue;			    // some problem...
        execute();			// execute the command*/
    }
    write(1,"\n",1);
    return 0;
}
