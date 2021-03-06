
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "pl.h"
#include "dpll.h"


static char* read_all(char *filename)
{
    char *str = NULL;
    long length = 0;
    FILE *f = fopen(filename, "rb");
    if (!f) {
        return NULL;
    }

    fseek(f, 0, SEEK_END);
    length = ftell(f);
    rewind(f);
    str = malloc(length + 1);
    memset(str, 0, length + 1);
    fread(str, sizeof(char), length, f);
    fclose(f);
    return str;
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        printf("Usage: pltest [algorithm] [filename]\n");
        return 1;
    }

    char *str = read_all(argv[2]);
    if (!str) {
        printf("Error: file not found\n");
        return 2;
    }

    printf("%s\n", str);

    int t1 = 0;
    int t2 = 0;
    PL_CONJUNCTION* conjunction = pl_parse(str);
    t1 = (clock() / (CLOCKS_PER_SEC / 1000));

    PL_INTERPRETATION* interpretation = NULL;
    if (strcmp(argv[1], "brute") == 0) {
        interpretation = pl_brute_force(conjunction);
    } else if (strcmp(argv[1], "dpll") == 0) {
        interpretation = pl_dpll(conjunction);
    }

    t2 = (clock() / (CLOCKS_PER_SEC / 1000));
    pl_print_interpretation(interpretation);
    printf("duration = %d ms\n", (t2 - t1));
    free(interpretation);
    free(conjunction);
    return 0;
}
