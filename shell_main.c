//Main function for the shell. Gets input, parses it, then executes it.
int main(int argc, char *argv[])
{
    while (Getline()) {			/* get command line */
        if (!parse())			/* parse into words */
            continue;			    /* some problem... */
        execute();			/* execute the command */
    }
    write(1,"\n",1);
    return 0;
}
