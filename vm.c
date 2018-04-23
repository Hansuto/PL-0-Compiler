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
#include "vm.h"
#include "state.h"

#define NUM_REGISTERS 16
#define MAX_LEXI_LEVELS 3
#define MAX_CODE_LENGTH 500
#define MAX_STACK_HEIGHT 2000

typedef struct instruction {
	int OP;
	int R;
	int L;
	int M;
} instruction;

// Default values for PM/0 CPU Registers
int SP = 0;
int BP = 1;
int PC = 0;
instruction IR;

//Handles halt conditions.
int halt = 0;

// Stack automatically initialized to all 0s
int stack[MAX_STACK_HEIGHT];
int activationRecordList[MAX_STACK_HEIGHT];
int activationRecordTracker = 0;

// The corresponding OP or Operation given the OP/M as index
char *const OP_CODES[] = {" ","lit","rtn","lod","sto","cal","inc","jmp","jpc","sio", "neg","add","sub","mul","div","odd","mod","eql","neq","lss","leq","gtr","geq"};


// All instructions we have to do
instruction instructions[MAX_CODE_LENGTH];
int registers[NUM_REGISTERS];

void printStack();
int isActivationRecord(int index);
int processInputFile(char *filename);
int base(int L, int B);
void fetchCycle();
void executeCycle();

int vm(char * file)
{
    int i, j, numInstructions;
    numInstructions = processInputFile(file);

    printf("GENERATED INTERMEDIATE CODE:\n");
    for (i = 0; i < numInstructions; i++) {
        printf("%3d ", i);
        printf("%s ",OP_CODES[instructions[i].OP]);
        printf("%d ", instructions[i].R);
        printf("%d ", instructions[i].L);
        printf("%d\n", instructions[i].M);
    }
    printf("-------------------------------------------");


    printf("\nVIRTUAL MACHINE TRACE:\n");
    printf("Initial Values\t   pc\t   bp\t   sp\n");
    for (i = 0; i < numInstructions; i++) {
        printf("%s %d %d %d ",
               OP_CODES[instructions[i].OP],
               instructions[i].R, instructions[i].L,
               instructions[i].M);

        fetchCycle();
        executeCycle();

        printStack();
        printf("\n");
        printf("RF: ");

        for (j = 0; j < NUM_REGISTERS; j++)
            printf("%d%s", registers[j],
                   j + 1 != NUM_REGISTERS ? "," : "\n\n");

        if (halt)
            break;
    }

	return 0;
}

int processInputFile(char *filename)
{
    int num, i = 0, j = 0;

    if (filename == NULL)
        return 0;

    FILE * fin;

	if ((fin = fopen(filename, "r")) == NULL)
		return 0;

    while (fscanf(fin, "%d", &num) != EOF) {
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


void printStack() {
	// Prints the PC, BP, AND SP separated by tabs.
	printf("\t%4d\t%4d\t%4d\t", PC, BP, SP);

    int index;
    for (index = 1; index <= SP; index++)
        printf("%s%d ", isActivationRecord(index) ? "| " : "" , stack[index]);

    if (IR.OP == 9 && IR.M == 2) {
    	exit(0);
	}
}

int isActivationRecord(int index) {
	return index == BP && SP > BP && BP > 1;
}

void fetchCycle() {
	IR = instructions[PC++];
}

void executeCycle() {
	switch(IR.OP) {
		case LIT:
            registers[IR.R] = IR.M;
			break;
            
		case RET:
            SP = BP - 1;
            BP = stack[SP + 3];
            PC = stack[SP + 4];
			break;
            
		case LOD:
			registers[IR.R] = stack[base(IR.L, BP) + IR.M];
			break;
            
		case STO:
			stack[base(IR.L, BP) + IR.M] = registers[IR.R];
			break;
            
		case CAL:
			stack[SP + 1] = 0;
			stack[SP + 2] = base(IR.L, BP);
			stack[SP + 3] = BP;
			stack[SP + 4] = PC;
			BP = SP + 1;
			PC = IR.M;
			break;
            
		case INC:
			SP += IR.M;
			break;
            
		case JMP:
			PC = IR.M;
			break;
            
		case JPC:
			if (registers[IR.R] == 0) {
				PC = IR.M;
			}
			break;
            
		case SIO:
			if (IR.M == 1) {
				printf("\t\t%2d\n", registers[IR.R]);
			} else if (IR.M == 2) {
				int temp;
				scanf("%d", &temp);
				printf("  \t\tread %d from input\n", temp);
                registers[IR.R] = temp;
            } else if (IR.M == 3) {
                halt = 1;
            }
			break;
            
        case NEG:
            registers[IR.R] = -registers[IR.L];
            break;
            
        case ADD:
            registers[IR.R] = registers[IR.L] + registers[IR.M];
            break;
        case SUB:
            registers[IR.R] = registers[IR.L] - registers[IR.M];
            break;
            
        case MUL:
            registers[IR.R] = registers[IR.L] * registers[IR.M];
            break;
            
        case DIV:
            registers[IR.R] = registers[IR.L] / registers[IR.M];
            break;
            
        case ODD:
            registers[IR.R] %= 2;
            break;
            
        case MOD:
            registers[IR.R] = registers[IR.L] % registers[IR.M];
            break;
            
        case EQL:
            registers[IR.R] = registers[IR.L] == registers[IR.M];
            break;
            
        case NEQ:
            registers[IR.R] = registers[IR.L] != registers[IR.M];
            break;
            
        case LSS:
            registers[IR.R] = registers[IR.L] < registers[IR.M];
            break;
            
        case LEQ:
            registers[IR.R] = registers[IR.L] <= registers[IR.M];
            break;
            
        case GTR:
            registers[IR.R] = registers[IR.L] > registers[IR.M];
            break;
            
        case GEQ:
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
