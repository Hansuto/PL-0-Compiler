/*
COP3402
Spring 2018
McAlpin
-----------
Chris Perkins    - ch289391
Chris Taliaferro - ch119541
*/

#include "parser.h"
#include "state.h"

// Some initial values and global variables
symbol symbolTable[MAX_SYMBOL_TABLE_SIZE];
instruction code[MAX_CODE_LENGTH];
tokenType tokens[MAX_CODE_LENGTH];

tokenType token;
int symVal = 0;         // symbolTable[]
int codeVal = 0;        // code[]
int tokenVal = 0;       // tokenArray[]
int count = 0;          // removeID_Num function
int relOp = 0;          // condition switch
int codeIndex = 0;      // code counter emit function
int spaceOffset = 4;    // startin point for symbol table
int lexLevel = 0;       // keep track of current lex level--
                        // --increase when enter Block and decrease when exit it

FILE *inputFile;
FILE *outputFile;
FILE *updatedToken;
FILE *errorFile;

// Function prototypes
void program();
void block();
void statement();
void condition();
void expression();
void term();
void factor();

void gotoNextToken();
void printTokens(int, int);
void emit(int op, int l, int m);
void errorMessage();
void convert();
void getTokenType(char*, int);
void printCode();

// Gets the next token
void gotoNextToken()
{
    token = tokens[tokenVal];
	tokenVal++;
}

void program()
{
    gotoNextToken();
    block();

    // Period
    if (token.type != periodsym)
    {
        errorMessage(9);    // Period expected
        errorFlag = 1;
    }

    emit(9,0,3);      // End program
}

void block()
{
    int jumpAddress = codeIndex;
    int varCounter = 0;
    lexLevel++;
    
    // TODO: Emit (most likely)

    // [ "const" ident "=" number {"," ident "=" number} ";"]
    if (token.type == constsym)
    {
        do
        {
            gotoNextToken();
            if (token.type != identsym)
            {
                errorMessage(4);
                errorFlag = 1;
            }
            
            symbolTable[symVal].kind = 1;
            strcpy(symbolTable[symVal].name, token.symbol);
            symbolTable[symVal].level = lexLevel;
            gotoNextToken();

            if (token.type != eqlsym)
            {
                errorMessage(3);
                errorFlag = 1;
            }

            gotoNextToken();
            if (token.type != numbersym)
            {
                errorMessage(2);
                errorFlag = 1;
            }

            int val = atoi(token.symbol);
            symbolTable[symVal].val = val;

            // symbolTable[symVal].addr = codeIndex;

            symVal++;
            gotoNextToken();
        }while (token.type == commasym);

        if (token.type != semicolonsym)
        {
            errorMessage(5);            // Semicolon or comma missing
            errorFlag = 1;
        }

        gotoNextToken();
    }

    // [ "var" ident {"," ident} ";"]
    if (token.type == varsym)
    {
        //
        do
        {
            gotoNextToken();
            if (token.type != identsym)
            {
                errorMessage(4);
                errorFlag = 1;
            }

            // printf("symval const: %d\n", symVal);
            symbolTable[symVal].kind = 2;
            symbolTable[symVal].level = lexLevel;
            symbolTable[symVal].addr = spaceOffset + varCounter;
            strcpy(symbolTable[symVal].name, token.symbol);

            symVal++;
            varCounter++;
            gotoNextToken();
        }while (token.type == commasym);

        if (token.type != semicolonsym)
        {
            errorMessage(5);
            errorFlag = 1;
        }

        gotoNextToken();
    }

    // This isn't a part of the assignment...
    // Why is it here?
//    while (token.type == procsym)
//    {
//        gotoNextToken();
//        if (token.type != identsym)
//        {
//            errorMessage(4);        // const, var, procedure must be followed by identifier
//            errorFlag = 1;
//        }
//
//        // printf("symval proc: %d\n", symVal);
//        symbolTable[symVal].kind = 3;
//        symbolTable[symVal].level = lexLevel;
//        symbolTable[symVal].addr = codeIndex;
//        strcpy(symbolTable[symVal].name, token.symbol);
//
//        gotoNextToken();
//
//        // lexLevel++;
//
//        if (token.type != semicolonsym)
//        {
//            errorMessage(5);            // Semicolon or comma missing
//            errorFlag = 1;
//        }
//
//        gotoNextToken();
//
//        symVal++;
//        // printf("symval proc2: %d\n", symVal);
//        block();
//
//        if (token.type != semicolonsym)
//        {
//            errorMessage(5);            // Semicolon or comma missing
//            errorFlag = 1;
//        }
//
//        gotoNextToken();
//    }

    code[jumpAddress].M = codeIndex;
    // TODO: Emit

    statement();

    if (token.type != periodsym)
    {
        // TODO: Emit
    }
    
    lexLevel--;
}

void statement()
{
    // [ident ":=" expression ]
    if (token.type == identsym)
    {
        int i, j = 0;

        if (symVal == 0)
        {
            errorMessage(11);           // Undeclared identifier
            errorFlag = 1;
        }

        for (i = 0; i < symVal; i++)
        {
            // printf("SYM Name: %s TOK Sym: %s\n",symbolTable[i].name,token.symbol);
            if (strcmp(symbolTable[i].name,token.symbol) == 0 && symbolTable[i].kind == 2)
            {
                j = i;
            }
            // printf("i value: %d j value: %d\n", i, j);
        }

        if (symbolTable[j].kind != 2)
        {
            printf("In statement() identsym: ");
            errorMessage(12);             // Assignment to constant or procedure is not allowed
            errorFlag = 1;
        }

        gotoNextToken();
        if (token.type != becomessym)
        {
            errorMessage(19);           // Incorrect symbol following statement
            errorFlag = 1;
        }

        gotoNextToken();
        expression();

        // THIS ADDED IN
        // printf("j value2: %d\n", j);
        // printf("EMIT(4,0,%d)\n", symbolTable[j].addr);
        if ((lexLevel - symbolTable[j].level) < 0)
        {
            errorMessage(11);             // Undeclared identifier
            errorFlag = 1;
        }

        // TODO: emit
    }
    
    // [ "begin" statement{ ";" statement} "end" ]
    else if (token.type == beginsym)
    {
        gotoNextToken();
        statement();

        while (token.type == semicolonsym)
        {
            gotoNextToken();
            statement();
        }

        if (token.type != endsym)
        {
            errorMessage(26);           // end expected
            errorFlag = 1;
        }

        gotoNextToken();
    }
    
    // [ "if" condition "then" statement ]
    else if (token.type == ifsym)
    {
        int codeIndexTemp, codeIndexTemp2;
        
        gotoNextToken();
        condition();

        if (token.type != thensym)
        {
            errorMessage(16);           // then expected
            errorFlag = 1;
        }

        codeIndexTemp = codeIndex;

        gotoNextToken();
        statement();
        
        code[codeIndexTemp].M = codeIndex;
    }
    
    // [ "while" condition "do" statement ]
    else if (token.type == whilesym)
    {
        int codeIndexTemp1 = codeIndex;

        gotoNextToken();
        condition();

        int codeIndexTemp2 = codeIndex;
        // TODO: Emit

        if (token.type != dosym)
        {
            errorMessage(18);           // do expected
            errorFlag = 1;
        }

        gotoNextToken();
        
        statement();
        // TODO: Emit

        code[codeIndexTemp2].M = codeIndex;
    }
    
    // [ "read" ident ]
    else if (token.type == readsym)
    {
        int i, j = 0;
        
        gotoNextToken();

        if (token.type != identsym)
        {
            errorMessage(14);            // read must be followed by an identifier
            errorFlag = 1;
        }

        for (i = 0; i < symVal; i++)
        {
            // printf("SYM Name: %s TOK Sym: %s\n",symbolTable[i].name,token.symbol);
            if (strcmp(symbolTable[i].name,token.symbol) == 0 && symbolTable[i].kind == 2)
            {
                j = i;
            }
            // printf("i value: %d j value: %d\n", i, j);
        }

        if (symbolTable[j].kind != 2)
        {
            errorMessage(12);           // Assignment to constant or procedure is not allowed
            errorFlag = 1;
        }

        // TODO: Emit

        if ((lexLevel - symbolTable[j].level) < 0)
        {
            errorMessage(11);             // Undeclared identifier
            errorFlag = 1;
        }

        // TODO: emit
        
        gotoNextToken();
    }
    
    // [ "write" ident ]
    else if (token.type == writesym)
    {
        int i, j = 0;

        gotoNextToken();

        if (token.type != identsym)
        {
            errorMessage(14);            // call must be followed by an identifier
            errorFlag = 1;
        }

        // TODO: Emit Code Here
		gotoNextToken();
    }
}

void condition()
{
    // Odd
    if (token.type == oddsym)
    {
        gotoNextToken();
        expression();
        // TODO: Emit code here
    }
    else
    {
        expression();

        if ((token.type != eqlsym) && (token.type != neqsym) && (token.type != lessym) &&
            (token.type != leqsym) && (token.type != gtrsym) && (token.type != geqsym))
        {
            errorMessage(20);           // Relational operator expected
            errorFlag = 1;
        }
        relOp = token.type;
        gotoNextToken();
        
        expression();
        
        // TODO: Emit code here
    }
}

void expression()
{
    int addOp;

    // Plus and Minus
    if (token.type == plussym || token.type == minussym)
    {
        addOp = token.type;
        gotoNextToken();
        
        term();

        if(addOp == minussym)
        {
            // TODO: Emit
        }
    }
    else
        term();

    while (token.type == plussym || token.type == minussym)
    {
        addOp = token.type;
        gotoNextToken();
        
        term();

        if (addOp == plussym)
        {
            // TODO: Emit
        }
        else
        {
            // TODO: Emit
        }
    }
}

void term()
{
    // Stores mult/div symbol for use in generating code
    int mulOp;

    factor();

    // Multiply and Divide
    while (token.type == multsym || token.type == slashsym)
    {
        mulOp = token.type;
        gotoNextToken();
        
        factor();

        if (mulOp == multsym)
        {
            // TODO: Emit
        }
        else
        {
            // TODO: Emit
        }
    }
}

void factor()
{
    if (token.type == identsym)
    {
        int i, j = 0;

        // printf("symval: %d\n", symVal);
        for (i = 0; i < symVal; i++)
        {
            if (strcmp(symbolTable[i].name, token.symbol) == 0)
            {
                j = i;

                if (symbolTable[i].kind == 1)
                {
                    // TODO: Emit
                }
                else if (symbolTable[i].kind == 2)
                {
                    if ((lexLevel - symbolTable[j].level) < 0)
                    {
                        errorMessage(11);             // Undeclared identifier
                        errorFlag = 1;
                    }

                    // TODO: Emit
                }
                else
                {
                    errorMessage(21);   // Expression must not contain a procedure identifier
                    errorFlag = 1;
                }
            }
        }

        gotoNextToken();
    }
    else if (token.type == numbersym)
    {
        int num = atoi(token.symbol);
        
        // TODO: Emit
        
        gotoNextToken();
    }
    else if (token.type == lparentsym)
    {
        gotoNextToken();
        
        expression();

        if (token.type != rparentsym)
        {
            errorMessage(22);           // Right parenthesis missing
            errorFlag = 1;
        }

        gotoNextToken();
    }
    else
    {
        errorMessage(27);               // Invalid factor
        errorFlag = 1;
    }
}

// For code generation
void emit(int op, int l, int m)
{
    if (codeIndex > MAX_CODE_LENGTH)
    {
        errorMessage(28);
        errorFlag = 1;
    }
    else
    {
        code[codeIndex].OP = op;
        code[codeIndex].L = l;
        code[codeIndex].M = m;
        codeIndex++;
    }
}

//Error messages for the PL/0 Parser
void errorMessage(int error)
{
    switch(error)
    {
        case 1:
            printf("Use = instead of :=.\n");
            fprintf(errorFile, "Use = instead of :=.\n");
            break;
        case 2:
            printf("= must be followed by a number.\n");
            fprintf(errorFile, "= must be followed by a number.\n");
            break;
        case 3:
            printf("Identifier must be followed by =.\n");
            fprintf(errorFile, "Identifier must be followed by =.\n");
            break;
        case 4:
            printf("const, var, procedure must be followed by identifier.\n");
            fprintf(errorFile, "const, var, procedure must be followed by identifier.\n");
            break;
        case 5:
            printf("Semicolon or comma missing.\n");
            fprintf(errorFile, "Semicolon or comma missing.\n");
            break;
        case 6:
            printf("Incorrect symbol after procedure declaration.\n");
            fprintf(errorFile, "Incorrect symbol after procedure declaration.\n");
            break;
        case 7:
            printf("Statement expected.\n");
            fprintf(errorFile, "Statement expected.\n");
            break;
        case 8:
            printf("Incorrect symbol after statement part in block.\n");
            fprintf(errorFile, "Incorrect symbol after statement part in block.\n");
            break;
        case 9:
            printf("Period expected.\n");
            fprintf(errorFile, "Period expected.\n");
            break;
        case 10:
            printf("Semicolon between statements missing.\n");
            fprintf(errorFile, "Semicolon between statements missing.\n");
            break;
        case 11:
            printf("Undeclared identifier.\n");
            fprintf(errorFile, "Undeclared identifier.\n");
            break;
        case 12:
            printf("Assignment to constant or procedure is not allowed.\n");
            fprintf(errorFile, "Assignment to constant or procedure is not allowed.\n");
            break;
        case 13:
            printf("Assignment operator expected.\n");
            fprintf(errorFile, "Assignment operator expected.\n");
            break;
        case 14:
            printf("call must be followed by an identifier.\n");
            fprintf(errorFile, "call must be followed by an identifier.\n");
            break;
        case 15:
            printf("Call of a constant or variable is meaningless.\n");
            fprintf(errorFile, "Call of a constant or variable is meaningless.\n");
            break;
        case 16:
            printf("then expected.\n");
            fprintf(errorFile, "then expected.\n");
            break;
        case 17:
            printf("Semicolon or } expected.\n");
            fprintf(errorFile, "Semicolon or } expected.\n");
            break;
        case 18:
            printf("do expected.\n");
            fprintf(errorFile, "do expected.\n");
            break;
        case 19:
            printf("Incorrect symbol following statement.\n");
            fprintf(errorFile, "Incorrect symbol following statement.\n");
            break;
        case 20:
            printf("Relational operator expected.\n");
            fprintf(errorFile, "Relational operator expected.\n");
            break;
        case 21:
            printf("Expression must not contain a procedure identifier.\n");
            fprintf(errorFile, "Expression must not contain a procedure identifier.\n");
            break;
        case 22:
            printf("Right parenthesis missing.\n");
            fprintf(errorFile, "Right parenthesis missing.\n");
            break;
        case 23:
            printf("The preceding factor cannot begin with this symbol.\n");
            fprintf(errorFile, "The preceding factor cannot begin with this symbol.\n");
            break;
        case 24:
            printf("An expression cannot begin with this symbol.\n");
            fprintf(errorFile, "An expression cannot begin with this symbol.\n");
            break;
        case 25:
            printf("This number is too large.\n");
            fprintf(errorFile, "This number is too large.\n");
            break;
        case 26:
            printf("end expected.\n");
            fprintf(errorFile, "end expected.\n");
            break;
        case 27:
            printf("Invalid factor.\n");
            fprintf(errorFile, "Invalid factor.\n");
            break;
        case 28:
            printf("Problem with code generation overflow. \n");
            fprintf(errorFile, "Problem with code generation overflow. \n");
            break;
        case 29:
            printf("Error. Invalid type for tokenArray. \n");
            fprintf(errorFile, "Error. Invalid type for tokenArray. \n");
            break;
        case 30:
            printf("Cannot write to a procedure. \n");
            fprintf(errorFile, "Cannot write to a procedure. \n");
        default:
            printf("General Error. Need to make an error message for this.\n");
            fprintf(errorFile, "General Error. Need to make an error message for this.\n");
    }
}

// Converts symbolic lexeme list into tokens
void convert()
{
    char *temp;
    char str[MAX_CODE_LENGTH];
    int identFlag, numFlag;

    while(fgets(str, sizeof str, inputFile) != NULL)
    {
        temp = strtok(str, " ");
        while (temp != NULL)
        {
            identFlag = strcmp(temp, "identsym");
            numFlag = strcmp(temp, "numbersym");

            if ((identFlag == 0) || (numFlag == 0))
            {
                strcpy(tokens[count].name, temp);
                getTokenType(temp, count);
                fputs(temp, updatedToken);
                fputs("\n", updatedToken);

                // if identsym or numbersym add the identifier that follows
                temp = strtok(NULL, " ");
                strcpy(tokens[count].symbol, temp);
            }
            else
            {
                strcpy(tokens[count].name, temp);
                getTokenType(temp, count);
                fputs(temp, updatedToken);
                fputs("\n", updatedToken);
            }

            temp = strtok(NULL, " ");
            count++;
        }
    }
}

// Gets tokenArray[].type value from tokenArray[].name value
void getTokenType(char *name, int count)
{
    if (strcmp(name, "nulsym") == 0)
        tokens[count].type = nulsym;
    else if (strcmp(name, "identsym") == 0)
        tokens[count].type = identsym;
    else if (strcmp(name, "numbersym") == 0)
        tokens[count].type = numbersym;
    else if (strcmp(name, "plussym") == 0)
        tokens[count].type = plussym;
    else if (strcmp(name, "minussym") == 0)
        tokens[count].type = minussym;
    else if (strcmp(name, "multsym") == 0)
        tokens[count].type = multsym;
    else if (strcmp(name, "slashsym") == 0)
        tokens[count].type = slashsym;
    else if (strcmp(name, "oddsym") == 0)
        tokens[count].type = oddsym;
    else if (strcmp(name, "eqlsym") == 0)
        tokens[count].type = eqlsym;
    else if (strcmp(name, "neqsym") == 0)
        tokens[count].type = neqsym;
    else if (strcmp(name, "lessym") == 0)
        tokens[count].type = lessym;
    else if (strcmp(name, "leqsym") == 0)
        tokens[count].type = leqsym;
    else if (strcmp(name, "gtrsym") == 0)
        tokens[count].type = gtrsym;
    else if (strcmp(name, "geqsym") == 0)
        tokens[count].type = geqsym;
    else if (strcmp(name, "lparentsym") == 0)
        tokens[count].type = lparentsym;
    else if (strcmp(name, "rparentsym") == 0)
        tokens[count].type = rparentsym;
    else if (strcmp(name, "commasym") == 0)
        tokens[count].type = commasym;
    else if (strcmp(name, "semicolonsym") == 0)
        tokens[count].type = semicolonsym;
    else if (strcmp(name, "periodsym") == 0)
        tokens[count].type = periodsym;
    else if (strcmp(name, "becomessym") == 0)
        tokens[count].type = becomessym;
    else if (strcmp(name, "beginsym") == 0)
        tokens[count].type = beginsym;
    else if (strcmp(name, "callsym") == 0)
        tokens[count].type = callsym;
    else if (strcmp(name, "endsym") == 0)
        tokens[count].type = endsym;
    else if (strcmp(name, "ifsym") == 0)
        tokens[count].type = ifsym;
    else if (strcmp(name, "thensym") == 0)
        tokens[count].type = thensym;
    else if (strcmp(name, "elsesym") == 0)
        tokens[count].type = elsesym;
    else if (strcmp(name, "whilesym") == 0)
        tokens[count].type = whilesym;
    else if (strcmp(name, "dosym") == 0)
        tokens[count].type = dosym;
    else if (strcmp(name, "constsym") == 0)
        tokens[count].type = constsym;
    else if (strcmp(name, "varsym") == 0)
        tokens[count].type = varsym;
    else if (strcmp(name, "procsym") == 0)
        tokens[count].type = procsym;
    else if (strcmp(name, "writesym") == 0)
        tokens[count].type = writesym;
    else if (strcmp(name, "readsym") == 0)
        tokens[count].type = readsym;
    else
    {
        errorMessage(29);           // Invalid type for tokenArray
        errorFlag = 1;
    }
}

// Prints generated code to be used in Virtual Machine
void printCode()
{
    int i, j;

    for (i = 0; i < codeIndex; i++)
        fprintf(outputFile, "%d %d %d\n", code[i].OP, code[i].L, code[i].M);

    printf("\n\nCode is syntactically correct. Assembly code generated successfully.\n");
    printf("-------------------------------------------\n");
}

int parse(char * file)
{
    inputFile = fopen(file, "r");
    outputFile = fopen("parseOutput.txt", "w");
    updatedToken = fopen("tokenUpdated.txt", "w");
    errorFile = fopen("error.txt", "w");

    convert();
    program();

    if (errorFlag == 1)
        exit(0);

    printCode();

    fclose(inputFile);
    fclose(outputFile);
    fclose(updatedToken);

    return 0;
}
