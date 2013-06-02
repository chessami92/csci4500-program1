/* A simple shell                                           */
/* Josh DeWitt                                              */
/*                                                          */
/* Get user input, find executable path for execution, and  */
/* Support for multiple pipes from one program to another.  */
/* No autocomplete, wildcards, conditional, or sequential   */
/* command handling provided. Words are separated with tabs */
/* or spaces.                                               */
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <signal.h>

#define MAX_PATH_LENGTH 64

extern char **environ;  /*Environment, for getting path*/

/* Get a line from the standard input. Return 1 on success, or 0 at */
/* end of file. If the line contains only whitespace, ignore it and */
/* get another line. If the line is too long, display a message and */
/* get another line. If read fails, diagnose the error and abort.   */
/*------------------------------------------------------------------*/
/* This function will always displays prompt ("$ "). The input is   */
/* echoed to the standard output before execution.                  */
int getLine( char buffer[], int maxLength ) {
    int length;     /* Current line length. */
    int whitespace; /* Zero when only whitespace seen. */
    char c;         /* Current input character. */
    char *msg;		/* Holds error messages. */

    while( 1 ) {
        write( 1, "$ ", 2 );
        whitespace = length = 0;
        while( 1 ) {
            switch( read( 0, &c, 1 ) ) {
                case 0: 
                    return 0;
                case -1: 
                    msg = "ERROR reading command line.\n";
                    write( 2, msg, strlen( msg ) );
                    exit( 1 );
            }

            write ( 1, &c, 1 );             /* Echo the character. */

            if( c == '\n' ) {               /* Check for end of line. */
                break;
            }

            if( length >= maxLength ) {     /* Check if input too long. */
                length++;                   /* Just update length. */
                continue;
            }

            if( c != ' ' && c != '\t' ) {   /* Check if not whitespace. */
                whitespace = 1;
            }

            buffer[length++] = c;           /* Save the input character. */
        }

        if ( length >= maxLength ) {        /* Alert user if line too long. */
            msg = "ERROR: Input line is too long.\n";
            write( 2, msg, strlen( msg ) );
            continue;
        }
        if ( whitespace == 0 ) {            /* Alert user if only whitespace. */
            msg = "ERROR: Only whitespace detected.\n";
            write( 2, msg, strlen( msg ) );
            continue;
        }

        buffer[length] = '\0';              /* End with null character. */
        return 1;
    }
}

/* Parse the command line into an array of words. */
/* Return 1 on success, or 0 if there is an ERROR */
/* (i.e. there are too many words, or a word is   */
/* too long), diagnosing the ERROR.               */
/* The words are identified by the pointed in the */
/* 'words' array, and 'nwds' = number of words.   */
int parse( int maxWords, int maxWordLength, char *line, char *words[] ) {
    char *p;			/* Pointer to current word. */
    char *msg;			/* Holds error messages. */
    int numWords = 0;   /* For incrementally storing words. */

    p = strtok( line, " \t" );
    while( p != NULL ) {
        /* First check that no error conditions occurred. */
        if( numWords == maxWords ) {
            msg = "ERROR: Too many words.\n";
            write(2,msg,strlen(msg));
            return 0;
        }
        if( strlen( p ) >= maxWordLength ) {
            msg = "ERROR: Word too long.\n";
            write( 2, msg, strlen( msg ) );
            return 0;
        }
        words[numWords] = p;        /* Save pointer to the word. */
        numWords++;			        /* Increase the word count. */
        p = strtok( NULL," \t" );	/* Get pointer to next word, if any. */
    }
    words[numWords] = NULL;		    /* Mark end of argument array. */

    return 1;
}

/*--------------------------------------------------------------------------*/
/* Put in 'path' the relative or absolute path to the command identified by */
/* words[0]. If the file identified by 'path' is not executable, return -1. */
/* Otherwise return 0.                                                      */
/*--------------------------------------------------------------------------*/
int getPath( char *command, char *fullPath ) {
    char *p;        /* Pointer to each individual path element. */
    char *pathenv;  /* Pointer to copy of PATH. */

    /* Check to see if it is already a fully qualified */
    /* path name. If so, check executability and       */
    /* put the command in the full path label.         */
    if( strchr( command, '/' ) != NULL ) {	/* if it has no '/' */
        strcpy( fullPath, command );		    /* copy it to path */
        return access( fullPath, X_OK );	    /* return executable status */
    }

    /* Search for the program in each of the listings in the */
    /* environment variable PATH. Append the command name    */
    /* and test if it is executable. If so, put that in the  */
    /* full path variable.                                   */
    pathenv = strdup( getenv( "PATH" ) );		/* Make copy of PATH. */
    p = strtok( pathenv, ":" );			        /* Get first directory. */
    while( p != NULL ) {
        strcpy( fullPath, p );				    /* Copy dir to full path. */
        strcat( fullPath, "/" );			    /* Append a slash. */
        strcat( fullPath, command );			/* Append executable's name. */
        if ( access( fullPath, X_OK ) == 0 ) {  /* Ensure executability. */
            free( pathenv );			        /* Free PATH copy. */
            return 0;				            /* Report found. */ 
        }
        p = strtok( NULL, ":" );			    /* Get next directory. */
    }
    free( pathenv );				            /* Free PATH copy. */

    *fullPath = '\0';
    return -1;                                  /* Report not found. */
}
/* Run the command with the path in the element */
/* zero of the array. The path should already   */
/* validated.                                   */
/* Returns the pid of the child process if      */
/* successful, -1 if not successfull.           */
pid_t forkAndRun( char *command[], int fd[] ) {
    pid_t pid;                          /* Newly created PID. */
    char *msg;                          /* Holds error messages. */

    pid = fork();                               /* Create new process. */
    switch( pid ) {
        case -1:                                /* Ensure that it succeeded. */
            msg = "ERROR: Fork failed.\n";
            write( 2, msg, strlen( msg ) );
            exit( 1 );
        case 0:                                 /* In child process, execute. */
            if( fd[0] != 0 ) {
                close( 0 );     /* Close standard input. */
                dup( fd[0] );   /* Fill the standard input. */
                close( fd[0] ); /* Close the extra file descriptor. */
            }
            if( fd[1] != 1 ) {
                close( 1 );     /* Close standard output. */
                dup( fd[1] );   /* Fill the standard output. */
                close( fd[1] ); /* Close the extra file descriptor. */
            }

            /* Execute the command now that input and output are set. */
            execve( command[0], command, environ );
            msg = "ERROR: execve failed.\n";
            write( 2, msg, strlen( msg ) );
            exit( 1 );
        default:
            /* Close the file descriptors if they are not stdin or stdout. */
            if( fd[0] != 0 ) {
                close( fd[0] );
            }
            if( fd[1] != 1 ) {
                close( fd[1] );
            }

            return pid;         /*Return the pid to the calling method. */
    }
}

/* Switch two ints in place. */
void swap( int *num1, int *num2 ) {
    int temp;

    temp = *num1;
    *num1 = *num2;
    *num2 = temp;
}

/* Execute the command, if possible. If it is not */
/* executable, returns -1. Otherwise return 0 if  */
/* it completed successfully, or 1 if it did not. */
int execute( char *words[] ) {
    int fd[2][2];           /* Standard input and output for each process. */
    char executable[2][MAX_PATH_LENGTH];   /* The executable path to be run. */
    char *curCommand;       /* Pointer to current command searching for. */
    int status;             /* The status of the PID that exited. */
    int i;                  /* For looping to find pipes and wait on PIDs. */
    int command[2];         /* Points to command in words array. */
    char *msg;              /* Holds error messages. */

    command[0] = 0;         /* First command is at location 0. */
    command[1] = -1;        /* Assume no second command. */

    fd[0][0] = 0;           /* Assume standard input and output to begin. */
    fd[0][1] = 1;

    /* See if there is more than one command. */
    for( i = 1; words[i] != NULL; ++i ) {
        if( *words[i] == '|' ) {    /* Found a pipe. */
            if( command[1] != -1 ) {
                msg = "ERROR: Cannot have multiple pipes.\n";
                write( 2, msg, strlen( msg ) );
                return -1;
            }

            if( words[i + 1] == NULL ) {
                msg = "ERROR: Must pipe into another command.\n";
                write( 2, msg, strlen( msg ) );
                return -1;
            }

            words[i] = NULL;        /* Set pipe as null to separate commands. */
            command[1] = i + 1;     /* Next word must be command. */
        }
    }

    /* Validate all commands seen and get paths. */
    for( i = 0; i < 2 && command[i] != -1; ++i ) {
        curCommand = words[command[i]];

        if( getPath( curCommand, executable[i] ) != 0 ) {
            // Command cannot be executed. Display appropriate message.
            msg = "ERROR: '";
            write( 2, msg, strlen( msg ) );
            write( 2, curCommand, strlen( curCommand ) );
            msg = "' cannot be executed.\n";
            write( 2, msg, strlen( msg ) );
            return( -1 );
        }

        words[command[i]] = executable[i];   /* Set the path for the command. */
    }

    /* Create pipe for second command if necessary. */
    if( command[1] != -1 ) {
        if( pipe( fd[1] ) == -1 ) {
            msg = "ERROR: Pipe failed.\n";
            write( 2, msg, strlen( msg ) ); 
            exit( 1 );
        }
        /* Update the file descriptors - read from last execution */
        /* or standard input and write to next execution. */
        swap( &fd[0][1], &fd[1][1] );
    }

    /* Execute all commands. */
    for( i = 0; i < 2 && command[i] != -1; ++i ) {
        forkAndRun( words + command[i], fd[i] );
    }

    wait( &status );                /* Wait for all child processes to end. */
    wait( &status );

    return( 0 );
}
