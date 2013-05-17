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

    strcpy( line, "testingLongWord" );
    assert( parse( 10, 10, line, words ) == 0 && "Should have failed because of long word." );

    strcpy( line, "0 1 2 3 4 5 6 7 8 9 10" );
    assert( parse( 10, 10, line, words ) == 0 && "Should have failed because of long word." );
}

static void test_getPath() {
    char testCommand[30];
    char path[50];

    strcpy( testCommand, "echo" );
    assert( getPath( testCommand, path ) == 0 && "Should have found the path for echo." );
    assert( strcmp( path, "/bin/echo" ) == 0 );

    strcpy( testCommand, "grep" );
    assert( getPath( testCommand, path ) == 0 && "Should have found the path for grep." );
    assert( strcmp( path, "/bin/grep" ) == 0 );

    strcpy( testCommand, "vim" );
    assert( getPath( testCommand, path ) == 0 && "Should have found the path for vim." );
    assert( strcmp( path, "/usr/bin/vim" ) == 0 );
    
    strcpy( testCommand, "./shell_test" );
    assert( getPath( testCommand, path ) == 0 && "Should have found the current program!" );
    assert( strcmp( path, "./shell_test" ) == 0 );

    strcpy( testCommand, "badCommand" );
    assert( getPath( testCommand, path ) == -1 && "Should not have found this nonsense." );
    assert( strcmp( path, "" ) == 0 );
}

int main( void ) {
    test_input();
    test_parse();
    test_getPath();
    return 0;
}   
