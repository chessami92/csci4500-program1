#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

static void test_input() {
    char line[11];
    assert( getLine( line, 10 ) == 1 && "GetLine encountered an error" );
    assert( strcmp( line, "Test" ) == 0 && "Input should have been 'Test'" );
}

static void test_parse() {
    char line[30];
    char *words[10];
    strcpy( line, "testing\t\t parse |\tfunction\t   \t" );

    assert( parse( 10, 10, line, words ) == 1 && "Parse seems to have failed" );
    assert( strcmp( words[0], "testing" ) == 0 );
    assert( strcmp( words[1], "parse" ) == 0 );
    assert( strcmp( words[2], "|" ) == 0 );
    assert( strcmp( words[3], "function" ) == 0 );
}

static void test_parse_long_word() {
    char line[30];
    char *words[10];
    strcpy( line, "testingLongWord" );

    assert( parse( 10, 10, line, words ) == 0 && "Should have failed because of long word." );
}

static void test_parse_too_many_words() {
    char line[30];
    char *words[10];
    strcpy( line, "0 1 2 3 4 5 6 7 8 9 10" );

    assert( parse( 10, 10, line, words ) == 0 && "Should have failed because of long word." );
}

int main( void ) {
    test_input();
    test_parse();
    test_parse_long_word();
    test_parse_too_many_words();
    return 0;
}   
