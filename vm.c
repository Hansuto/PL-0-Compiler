/*
COP3402
Spring 2018
McAlpin
-----------
Chris Perkins    - ch289391
Chris Taliaferro - ch119541
*/

#include <stdio.h> 
#include <stdlib.h>

#define NUM_REGISTERS 8
#define MAX_LEXI_LEVELS 3
#define MAX_CODE_LENGTH 500
#define MAX_STACK_HEIGHT 2000
#define DEBUG 0

typedef struct instruction {
    int OP;
    int R;
    int L;
    int M;
}
instruction;

// Default values for PM/0 CPU Registers
int SP = 0;
int BP = 1;
int PC = 0;
int lexr = 0;
instruction IR;

//Handles halt conditions.
int halt = 0;

// Stack automatically initialized to all 0s
int stack[MAX_STACK_HEIGHT];
int activationRecordList[MAX_STACK_HEIGHT];

// The corresponding OP or Operation given the OP/M as index
char *
    const OP_CODES[] = {
        " ",
        "LIT",
        "RTN",
        "LOD",
        "STO",
        "CAL",
        "INC",
        "JMP",
        "JPC",
        "SIO",
        "NEG",
        "ADD",
        "SUB",
        "MUL",
        "DIV",
        "ODD",
        "MOD",
        "EQL",
        "NEQ",
        "LSS",
        "LEQ",
        "GTR",
        "GEQ"
    };

// All instructions we have to do
instruction instructions[MAX_CODE_LENGTH];
int registers[NUM_REGISTERS];

void printStack(int sp, int bp, int * stack, int lexr);
int isActivationRecord(int index);
int processInputFile(char * filename);
int base(int L, int B);
void fetchCycle();
void executeCycle();

int vm(char * file) {
    int i, j, numInstructions;
    numInstructions = processInputFile(file);

    if (DEBUG) {
        printf("Program:\n");
        for (i = 0; i < numInstructions; i++) {
            printf("%d ", instructions[i].OP);
            printf("%d ", instructions[i].R);
            printf("%d ", instructions[i].L);
            printf("%d\n", instructions[i].M);
        }

        printf("\nOp Printout:\n");
        for (i = 0; i < numInstructions; i++) {
            printf("%d ", i);
            printf("%s ", OP_CODES[instructions[i].OP]);
            printf("%d ", instructions[i].R);
            printf("%d ", instructions[i].L);
            printf("%d\n", instructions[i].M);
        }
    }

    printf("\n OP   Rg Lx Vl[ PC BP SP]\n");
    while (!halt) {
        fetchCycle();
        executeCycle();

        printf("%-4s%3d%3d%3d[%3d%3d%3d] ",
            OP_CODES[IR.OP],
            IR.R,
            IR.L,
            IR.M,
            PC,
            BP,
            SP);

        printStack(SP, BP, stack, lexr);
        printf("\n");

        printf("\tRegisters:[");
        for (j = 0; j < NUM_REGISTERS; j++)
            printf("%3d", registers[j]);
        printf("]\n");
        if (halt)
            break;
    }

    return 0;
}

void printStack(int sp, int bp, int * stack, int lexr) {
    int i;
    if (bp == 1) {
        if (lexr > 0) {
            printf("|");
        }
    } else {
        //Print the lesser lexical level
        printStack(bp - 1, stack[bp + 2], stack, lexr - 1);
        printf("|");
    }
    //Print the stack contents - at the current level
    for (i = bp; i <= sp; i++) {
        printf("%3d ", stack[i]);
    }
}

int processInputFile(char * filename) {
    int num, i = 0, j = 0;

    if (filename == NULL)
        return 0;

    FILE * fin;

    if ((fin = fopen(filename, "r")) == NULL)
        return 0;

    while (fscanf(fin, "%d", & num) != EOF) {
        switch (i++ % 4) {
        case 0:
            instructions[j].OP = num;
            break;
        case 1:
            instructions[j].R = num;
            break;
        case 2:
            instructions[j].L = num;
            break;
        case 3:
            instructions[j].M = num;
            j++;
            break;
        }
    }

    fclose(fin);
    return j;
}

void fetchCycle() {
    IR = instructions[PC++];
}

void executeCycle() {
    switch (IR.OP) {
    case 1: // LIT
        registers[IR.R] = IR.M;
        break;
    case 2: // RET
        SP = BP - 1;
        BP = stack[SP + 3];
        PC = stack[SP + 4];

        lexr--;
        break;
    case 3: // LOD
        registers[IR.R] = stack[base(IR.L, BP) + IR.M];
        break;
    case 4: // STO
        stack[base(IR.L, BP) + IR.M] = registers[IR.R];
        break;
    case 5: // CAL
        stack[SP + 1] = 0;
        stack[SP + 2] = base(IR.L, BP);
        stack[SP + 3] = BP;
        stack[SP + 4] = PC;

        BP = SP + 1;
        PC = IR.M;
        SP += 4;

        lexr++;
        break;
    case 6: // INC
        SP += IR.M;
        break;
    case 7: // JMP
        PC = IR.M;
        break;
    case 8: // JPC
        if (registers[IR.R] == 0) {
            PC = IR.M;
        }
        break;
    case 9: // SIO
        if (IR.M == 1)
            printf("%d\n", registers[IR.R]);
        else if (IR.M == 2)
            scanf("%d", & registers[IR.R]);
        else if (IR.M == 3)
            halt = 1;
        break;
    case 10: // NEG
        registers[IR.R] = -registers[IR.L];
        break;
    case 11: // ADD
        registers[IR.R] = registers[IR.L] + registers[IR.M];
        break;
    case 12: // SUB
        registers[IR.R] = registers[IR.L] - registers[IR.M];
        break;
    case 13: // MUL
        registers[IR.R] = registers[IR.L] * registers[IR.M];
        break;
    case 14: // DIV
        registers[IR.R] = registers[IR.L] / registers[IR.M];
        break;
    case 15: // ODD
        registers[IR.R] %= 2;
        break;
    case 16: // MOD
        registers[IR.R] = registers[IR.L] % registers[IR.M];
        break;
    case 17: // EQL
        registers[IR.R] = registers[IR.L] == registers[IR.M];
        break;
    case 18: // NEQ
        registers[IR.R] = registers[IR.L] != registers[IR.M];
        break;
    case 19: // LSS
        registers[IR.R] = registers[IR.L] < registers[IR.M];
        break;
    case 20: // LEQ
        registers[IR.R] = registers[IR.L] <= registers[IR.M];
        break;
    case 21: // GTR
        registers[IR.R] = registers[IR.L] > registers[IR.M];
        break;
    case 22: // GEQ
        registers[IR.R] = registers[IR.L] >= registers[IR.M];
        break;
    }
}

// Traverses down the stack "L" levels
int base(int L, int B) {
    while (L > 0) {
        B = stack[B + 1];
        L--;
    }

    return B;
}
