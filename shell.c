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
#include <errno.h>
#include <stdlib.h>
#include <signal.h>

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
    int numWords = 0;

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

/*--------------------------------------------------*/
/* Execute the command, if possible.                */
/* If it is not executable, return -1.              */
/* Otherwise return 0 if it completed successfully, */
/* or 1 if it did not.                              */
/*--------------------------------------------------*/
int execute( char *words[] )
{
    int i, j;
    int status;
    char *msg;

    if (0 == 0 ) { //execok() == 0) {			/* is it executable? */
        status = fork();			/* yes; create a new process */

        if (status == -1) {			/* verify fork succeeded */
            perror("fork");
            exit(1);
        }

        if (status == 0) {			/* in the child process... */
            //status = execve(path,words,environ); /* try to execute it */

            perror("execve");			/* we only get here if */
            exit(0);				/* execve failed... */
        }

        /*------------------------------------------------*/
        /* The parent process (the shell) continues here. */
        /*------------------------------------------------*/
        wait(&status);				/* wait for process to end */

    } else {
        /*----------------------------------------------------------*/
        /* Command cannot be executed. Display appropriate message. */
        /*----------------------------------------------------------*/
        msg = "*** ERROR: '";
        write(2,msg,strlen(msg));
        //write(2,words[0],strlen(words[0]));
        msg = "' cannot be executed.\n";
        write(2,msg,strlen(msg));
    }
}
