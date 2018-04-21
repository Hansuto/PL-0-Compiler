#ifndef variables
#define variables

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Constant values
#define MAX_STACK_HEIGHT 2000
#define MAX_CODE_LENGTH 32768
#define MAX_LEXI_LEVELS 8
#define MAX_NUMBER_LENGTH 5
#define MAX_IDENTIFIER_LENGTH 15
#define MAX_SYMBOL_TABLE_SIZE 2000

int errorFlag;      // To know if hit an error

// Struct pMachine
typedef struct
{
    int OP;
    int R;
    int L;
    int M;
}instruction;

// Data structure for symbol
// const: kind, name, value
// var:   kind, name, L, M
typedef struct
{
    int kind;                               // const = 1, var = 2, proc = 3
    char name[MAX_IDENTIFIER_LENGTH];       // name up to 11 characters
    int val;                                // number (ASCII value)
    int level;                              // L level
    int addr;                               // M address
}symbol;

// Token struct
typedef struct 
{
    int type;
    char name[MAX_IDENTIFIER_LENGTH];
    char symbol[MAX_IDENTIFIER_LENGTH];
}tokenType;


int parse(char * file);

#endif
