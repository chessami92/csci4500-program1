#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

static void test_parse() {
    char line[30];
    int *numWords;
    char *words[10];
    strcpy( line, "testing\t\t parse |\tfunction\t   \t" );

    assert( parse( line ) == 1 && "Parse seems to have failed" );
}

static void test_input() {
    char line[11];
    assert( getLine( line, 10 ) == 1 && "GetLine encountered an error" );

    printf( "Input was: %s", line );
}

int main( void ) {
    test_parse();
    test_input();
    return 0;
}   
