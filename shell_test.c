#include <assert.h>
#include <stdbool.h>
#include <string.h>

static void test_parse() {
    char line[30];
    int *numWords;
    char *words[10];
    strcpy( line, "testing\t\t parse |\tfunction\t   \t" );

    assert( parse( line ) == 1 && "Parse seems to have failed" );
}

int main(void) {
    test_parse();
    return 0;
}
