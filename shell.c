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
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <signal.h>

#define NWORDS 16		  /* max words on command line */
#define MAXWORDLEN 64		  /* maximum word length */

extern char **environ;		  /* environment */

char *words[NWORDS];              /* ptrs to words from the command line */
int nwds;			  /* # of words in the command line */
char path[MAXWORDLEN];		  /* path to the command */
char *argv[NWORDS+1];		  /* argv structure for execve */

/*------------------------------------------------------------------*/
/* Get a line from the standard input. Return 1 on success, or 0 at */
/* end of file. If the line contains only whitespace, ignore it and */
/* get another line. If the line is too long, display a message and */
/* get another line. If read fails, diagnose the error and abort.   */
/*------------------------------------------------------------------*/
/* This function will display a prompt ("# ") if the input comes    */
/* from a terminal. Otherwise no prompt is displayed, but the       */
/* input is echoed to the standard output. If the input is from a   */
/* terminal, the input is automatically echoed (assuming we're in   */
/* "cooked" mode).                                                  */
/*------------------------------------------------------------------*/
int getLine( char buffer[], int maxLength ) {
    int length;     //Current line length.
    int whitespace; //Zero when only whitespace seen.
    char c;         //Current input character.
    char *msg;		//Holds error messages.
    int isterm;		//Non-zero if input is from a terminal.

    isterm = isatty(0); //See if being run from terminal.
    while( length >= maxLength || whitespace == 0 ) {
        if( isterm ) {
            write(1,"# ",2);
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

            if( !isterm )                   //If input not from terminal
                write ( 1, &c, 1 );         //Echo the character.

            if( c == '\n' )                 //Check for end of line.
                break;

            if( length >= maxLength ) {     //Check if input too long.
                length++;                   //Just update length.
                continue;
            }

            if( c != ' ' && c != '\t' )     //Check if not whitespace.
                whitespace = 1;

            buffer[length++] = c;           //Save the input character.
        }

        if ( length >= maxLength ) {        //Alert user if line too long
            msg = "Input line is too long.\n";
            write( 2, msg, strlen( msg ) );
        }
        if ( whitespace == 0 ) {            //Alert user if only whitespace
            msg = "Only whitespace detected.\n";
            write( 2, msg, strlen( msg ) );
        }
    }

    buffer[length] = '\0';                  //End with null character.
    return 1;
}

/*------------------------------------------------*/
/* Parse the command line into an array of words. */
/* Return 1 on success, or 0 if there is an error */
/* (i.e. there are too many words, or a word is   */
/* too long), diagnosing the error.               */
/* The words are identified by the pointed in the */
/* 'words' array, and 'nwds' = number of words.   */
/*------------------------------------------------*/
int parse( char *line )
{
    char *p;			/* pointer to current word */
    char *msg;			/* error message */

    nwds = 0;
    p = strtok(line," \t");
    while (p != NULL) {
        if (nwds == NWORDS) {
            msg = "*** ERROR: Too many words.\n";
            write(2,msg,strlen(msg));
            return 0;
        }
        if (strlen(p) >= MAXWORDLEN) {
            msg = "*** ERROR: Word too long.\n";
            write(2,msg,strlen(msg));
            return 0;
        }
        words[nwds] = p;	// save pointer to the word 
        nwds++;			// increase the word count
        p = strtok(NULL," \t");	// get pointer to next word, if any
    }
    return 1;
}

/*--------------------------------------------------------------------------*/
/* Put in 'path' the relative or absolute path to the command identified by */
/* words[0]. If the file identified by 'path' is not executable, return -1. */
/* Otherwise return 0.                                                      */
/*--------------------------------------------------------------------------*/
int execok(void)
{
    char *p;
    char *pathenv;

    /*-------------------------------------------------------*/
    /* If words[0] is already a relative or absolute path... */
    /*-------------------------------------------------------*/
    if (strchr(words[0],'/') != NULL) {		/* if it has no '/' */
        strcpy(path,words[0]);			/* copy it to path */
        return access(path,X_OK);		/* return executable status */
    }

    /*-------------------------------------------------------------------*/
    /* Otherwise search for a valid executable in the PATH directories.  */
    /* We do this by getting a copy of the value of the PATH environment */
    /* variable, and checking each directory identified there to see it  */
    /* contains an executable file named word[0]. If a directory does    */
    /* have such a file, return 0. Otherwise, return -1. In either case, */
    /* always free the storage allocated for the value of PATH.          */
    /*-------------------------------------------------------------------*/
    pathenv = strdup(getenv("PATH"));		/* get copy of PATH value */
    p = strtok(pathenv,":");			/* find first directory */
    while (p != NULL) {
        strcpy(path,p);				/* copy directory to path */
        strcat(path,"/");			/* append a slash */
        strcat(path,words[0]);			/* append executable's name */
        if (access(path,X_OK) == 0) {		/* if it's executable */
            free(pathenv);			    /* free PATH copy */
            return 0;				    /* and return 0 */
        }
        p = strtok(NULL,":");			/* get next directory */
    }
    free(pathenv);				/* free PATH copy */
    return -1;					/* say we didn't find it */
}

/*--------------------------------------------------*/
/* Execute the command, if possible.                */
/* If it is not executable, return -1.              */
/* Otherwise return 0 if it completed successfully, */
/* or 1 if it did not.                              */
/*--------------------------------------------------*/
int execute(void)
{
    int i, j;
    int status;
    char *msg;

    if (execok() == 0) {			/* is it executable? */
        status = fork();			/* yes; create a new process */

        if (status == -1) {			/* verify fork succeeded */
            perror("fork");
            exit(1);
        }

        if (status == 0) {			/* in the child process... */
            words[nwds] = NULL;			/* mark end of argument array */

            status = execve(path,words,environ); /* try to execute it */

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
        write(2,words[0],strlen(words[0]));
        msg = "' cannot be executed.\n";
        write(2,msg,strlen(msg));
    }
}
