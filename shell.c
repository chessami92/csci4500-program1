/*---------------------------------------------------------------*/
/* A simple shell                                                */
/* Stanley Wileman                                               */
/* Last change: 5/13/2013                                        */
/*                                                               */
/* Process command lines (100 char max) with at most 16 "words"  */
/* and one command. No wildcard or shell variable processing.    */
/* No pipes, conditional or sequential command handling is done. */
/* Each word contains at most MAXWORDLEN characters.             */
/* Words are separated by spaces and/or tab characters.          */
/*                                                               */
/* This file is provided to students in CSCI 4500 for their use  */
/* in developing solutions to the first programming assignment.  */
/*---------------------------------------------------------------*/
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <signal.h>

#define MAX_PATH_LENGTH 50
#define MAX_PIPES 10

extern char **environ;  /*Environment, for getting path*/

/*------------------------------------------------------------------*/
/* Get a line from the standard input. Return 1 on success, or 0 at */
/* end of file. If the line contains only whitespace, ignore it and */
/* get another line. If the line is too long, display a message and */
/* get another line. If read fails, diagnose the error and abort.   */
/*------------------------------------------------------------------*/
/* This function will display a prompt ("$ ") if the input comes    */
/* from a terminal. Otherwise no prompt is displayed, but the       */
/* input is echoed to the standard output. If the input is from a   */
/* terminal, the input is automatically echoed (assuming we're in   */
/* "cooked" mode).                                                  */
/*------------------------------------------------------------------*/
int getLine( char buffer[], int maxLength ) {
    int length;     /* Current line length. */
    int whitespace; /* Zero when only whitespace seen. */
    char c;         /* Current input character. */
    char *msg;		/* Holds error messages. */
    int isterm;		/* Non-zero if input is from a terminal. */

    isterm = isatty( 0 ); /* See if being run from terminal. */
    while( 1 ) {
        if( isterm ) {
            write( 1, "$ ", 2 );
        }
        whitespace = length = 0;
        while( 1 ) {
            switch( read( 0, &c, 1 ) ) {
                case 0: 
                    return 0;
                case -1: 
                    msg = "Error reading command line.\n";
                    write( 2, msg, strlen( msg ) );
                    exit( 1 );
            }

            if( !isterm ) {                 /* If input not from terminal */
                write ( 1, &c, 1 );         /* Echo the character. */
            }

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

/*------------------------------------------------*/
/* Parse the command line into an array of words. */
/* Return 1 on success, or 0 if there is an error */
/* (i.e. there are too many words, or a word is   */
/* too long), diagnosing the error.               */
/* The words are identified by the pointed in the */
/* 'words' array, and 'nwds' = number of words.   */
/*------------------------------------------------*/
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
        strcpy( fullPath, p );				    /* Copy directory to full path. */
        strcat( fullPath, "/" );			    /* Append a slash. */
        strcat( fullPath, command );			/* Append executable's name. */
        if ( access( fullPath, X_OK ) == 0 ) {  /* Ensure that it is executable. */
            free( pathenv );			        /* Free PATH copy. */
            return 0;				            /* Report found. */ 
        }
        p = strtok( NULL, ":" );			    /* Get next directory. */
    }
    free( pathenv );				            /* Free PATH copy. */
    *fullPath = 0;                              /* Set path to null. */

    return -1;                                  /* Report not found. */
}
/* Find the location of the command and run it. */
/* Returns the pid of the child process if      */
/* successful, -1 if not successfull.           */
pid_t forkAndRun( char *command[], int fd[] ) {
    char executable[MAX_PATH_LENGTH];   /* The actual executable path to be run. */
    pid_t pid;                          /* Newly created PID that shell will wait on. */
    char *msg;                          /* Holds error messages. */

    if ( getPath( command[0], executable ) == 0 ) { /* Check executability, get path. */
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
                execve( executable, command, environ );
                msg = "Error: execve failed.\n";
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
    } else {
        /* Command cannot be executed. Display appropriate message. */
        msg = "ERROR: '";
        write( 2, msg, strlen( msg ) );
        write( 2, command[0], strlen( command[0] ) );
        msg = "' cannot be executed.\n";
        write( 2, msg, strlen( msg ) );
        return( -1 );
    }

    return pid;
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
    int fd[2], fdNext[2];   /* File descriptors for current and next process. */
    int status;             /* The status of the PID that exited. */
    int i;                  /* For looping to find pipes and wait on PIDs. */
    int command;            /* Executable location with words parameter. */
    int commandCount;       /* How many commands were executed (one less than # of pipes). */
    char *msg;              /* Holds error messages. */

    fdNext[0] = 0;
    fdNext[1] = 1;

    /* See if there are any pipes. */
    for( i = 0, command = 0; words[i] != NULL; ++i ) {
        if( *words[i] == '|' ) {    /* Found a pipe. */
            words[i] = NULL;
            fd[0] = fdNext[0];      /* Update the current file descriptors. */
            fd[1] = fdNext[1];

            if( pipe( fdNext ) == -1 ) {
                msg = "ERROR: Pipe failed.\n";
                write( 2, msg, strlen( msg ) );
                exit( 1 );
            }

            /* Update the file descriptors - read from last execution */
            /* or standard input and write to next execution. */
            swap( &fd[1], &fdNext[1] );
            /* Run the command with the correct file descriptors. */
            forkAndRun( words + command, fd );
            command = i + 1;        /* Point to next valid command. */
            commandCount++;         /* Signal that another command was run. */
        }
    }

    /* Run the final command and capture the PID. */
    forkAndRun( words + command, fdNext );
    commandCount++;                 /* Signal that another command was run. */

    for( i = 0; i < commandCount; ++i ) {   /* Wait for all child processes to end. */
        wait( &status );
    }

    return( 0 );
}
