/*
COP3402
Spring 2018
McAlpin
-----------
Chris Perkins    - ch289391
Chris Taliaferro - ch119541
*/
//#include "stdafx.h" <-- to run in visual studio
#include "parser.h"
#include "state.h"

// Some initial values and global variables
symbol symbolTable[MAX_SYMBOL_TABLE_SIZE];
instruction code[MAX_CODE_LENGTH];
tokenType tokens[MAX_CODE_LENGTH];

tokenType token;
int symVal = 0;           // symbolTable[]
int codeVal = 0;          // code[]
int tokenVal = 0;         // tokenArray[]
int count = 0;            // removeID_Num function
int relOp = 0;            // condition switch
int codeIndex = 0;        // code counter emit function
int symbolTableIndex = 4; // Size of the symbol table
int spaceOffset = 4;      // startin point for symbol table
int lexLevel = -1;        // keep track of current lex level--
                          // --increase when enter Block and decrease when exit it
int currentRegister = 0;  // The current register we're saving/loading to

FILE *inputFile;
FILE *outputFile;
FILE *updatedToken;
FILE *errorFile;

// Function prototypes
void parseProgram();
void parseBlock();
void parseConstantDeclaration();
int  parseVarDeclaration();
void parseStatement();
void parseCondition();
OP   parseRelOp();
void parseExpression();
void parseTerm();
void parseFactor();

int emit(OP op, int r, int l, int m);
int enterSymbol(symbolType type, char *name, char *value, int level, int address);
int findSymbolIndexWithName(char* identifier);

void gotoNextToken();
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

// block "."
void parseProgram()
{
    gotoNextToken();
    parseBlock();

    // Period expected
    if (token.type != periodsym) { errorMessage(9); }

    emit(SIO, 0, 0, 3);      // End program
}

// const-declaration  var-declaration  statement.
void parseBlock()
{
    lexLevel++;

    parseConstantDeclaration();
    
    parseVarDeclaration();

    // Increment to the place after where variables are defined
    emit(INC, 0, lexLevel, spaceOffset);
    
    parseStatement();
    
    lexLevel--;
}

// [ "const" ident "=" number {"," ident "=" number} ";"]
void parseConstantDeclaration()
{
    if (token.type == constsym)
    {
        do
        {
            gotoNextToken();
            // identifier expected
            if (token.type != identsym) { errorMessage(4); }
            
            char* identifier = token.name;

            gotoNextToken();
            // "=" expected
            if (token.type != eqlsym) { errorMessage(3); }
            
            gotoNextToken();
            // number expected
            if (token.type != numbersym) { errorMessage(2); }
            
            // Address doesn't matter for constants, so we can set to 0
            enterSymbol(constType, identifier, token.symbol, lexLevel, 0);
            
            gotoNextToken();
        } while (token.type == commasym);
        
        // Semi-colon or comma expected
        if (token.type != semicolonsym) { errorMessage(5); }
        
        gotoNextToken();
    }
}

// [ "var" ident {"," ident} “;"]
// RETURNS: Number of vars parsed
int parseVarDeclaration()
{
    int numVars = 0;
    
    if (token.type == varsym)
    {
        do {
            numVars++;
            
            gotoNextToken();
            if (token.type != identsym) { errorMessage(4); }
            
            // Enters a variable type in the symbol table with value, address of offset.
            // Level is set to lexLevel
            // NOTE: Address is set to to the current offset
            enterSymbol(varType, token.name, token.symbol, lexLevel, spaceOffset);
            spaceOffset++;
            
            gotoNextToken();
        } while (token.type == commasym);
        
        // Semi-colon expected
        if (token.type != semicolonsym) { errorMessage(5); }
        
        gotoNextToken();
    }
    
    return numVars;
}

/*
 statement ::= [ ident":=" expression
                | "begin" statement{ ";" statement} "end"
                | "if" condition"then" statement
                | "while" condition"do" statement
                | "read" ident
                | "write" ident ]
 */
void parseStatement()
{
    // [ident ":=" expression ]
    if (token.type == identsym)
    {
        int identifierIndex = findSymbolIndexWithName(token.symbol);
        
        if (identifierIndex == -1)
        {
            errorMessage(11); // Undeclared identifier;
        }
        else if (symbolTable[identifierIndex].type != varType)
        {
            // Assignment to constant or procedure is not allowed
            errorMessage(12);
        }
        
        gotoNextToken();
        // ":=" expected
        if (token.type != becomessym) { errorMessage(19); }

        gotoNextToken();
        parseExpression();
        
        // stack[base(ILvl, BP) + Add] = Registers[previousExpression.R];
        emit(STO,
             currentRegister,
             symbolTable[identifierIndex].level,
             symbolTable[identifierIndex].address);
    }
    
    // [ "begin" statement{ ";" statement} "end" ]
    else if (token.type == beginsym)
    {
        gotoNextToken();
        parseStatement();

        while (token.type == semicolonsym)
        {
            gotoNextToken();
            parseStatement();
        }

        // End expected
        if (token.type != endsym) { errorMessage(26); }

        gotoNextToken();
    }
    
    // [ "if" condition "then" statement ]
    else if (token.type == ifsym)
    {
        gotoNextToken();
        parseCondition();
        
        // Then expected
        if (token.type != thensym) { errorMessage(16); }

        gotoNextToken();
        
        // if (Registers[L] == 0) { PC = M; }
        int jumpCodeIndex = emit(JPC, currentRegister, 0, codeIndex);
        
        parseStatement();
        
        // Jumps to after the statement if false
        code[jumpCodeIndex].M = codeIndex;
    }
    
    // [ "while" condition "do" statement ]
    else if (token.type == whilesym)
    {
        // The position where the while first starts
        int whileStartIndex = codeIndex;
        
        gotoNextToken();
        parseCondition();
        
        // The code which will trigger our exit of the while
        // if (Registers[0] == 0) { PC = M; }
        int exitWhileJPCIndex = emit(JPC, currentRegister, 0, 0);

        if (token.type != dosym) { errorMessage(18); }

        gotoNextToken();
        parseStatement();
        
        // PC = whileStartIndex;
        emit(JMP, currentRegister, 0, whileStartIndex);
        
        code[exitWhileJPCIndex].M = codeIndex;
    }
    
    // [ "read" ident ]
    else if (token.type == readsym)
    {
        gotoNextToken();
        if (token.type != identsym) { errorMessage(14); }
        
        // Scan in
        // Registers[0] = scan_from_file_int
        emit(SIO, currentRegister, lexLevel, 2);
        
        gotoNextToken();
    }
    
    // [ "write" ident ]
    else if (token.type == writesym)
    {
        gotoNextToken();
        // Identifier expected
        if (token.type != identsym) { errorMessage(14); }

        int identifierIndex = findSymbolIndexWithName(token.symbol);
        
        emit(LOD, currentRegister, lexLevel, identifierIndex - 1);
        // printf(Registers[curReg])
        emit(SIO, currentRegister, 0, 1);
        
		gotoNextToken();
    }
}

/*
 "odd" expression
 | expression  rel-op  expression
 */
void parseCondition()
{
    if (token.type == oddsym)
    {
        gotoNextToken();
        parseExpression();
        
        // Registers[previous] = Registers[previous] % 2
        emit(ODD, code[codeIndex - 1].R, 0, 0);
    }
    else
    {
        parseExpression();
        
        // TODO: This should handle more than one variable
        // Registers[0] = stack[base(0, BP) + M];
        
        OP relationOp = parseRelOp();
        
        parseExpression();
        // TODO: This should handle more than one variable
        // Registers[0] = stack[base(0, BP) + M];
        
        // Registers[0] = Registers[0] (op) Registers[0];
        emit(relationOp, 0, 0, 0);
    }
}

// "="|“<>"|"<"|"<="|">"|">=“
// Returns: the relation op we have to perform
OP parseRelOp()
{
    OP returnValue = 0;
    
    switch (token.type)
    {
        case eqlsym:
            returnValue = EQL;
            break;
        case neqsym:
            returnValue = NEQ;
            break;
        case lessym:
            returnValue = LSS;
            break;
        case leqsym:
            returnValue = LEQ;
            break;
        case gtrsym:
            returnValue = GTR;
            break;
        case geqsym:
            returnValue = GEQ;
            break;
        default:
            errorMessage(20);
            break;
    }
    gotoNextToken();
    
    return returnValue;
}

// ["+"|"-"] term { ("+"|"-") term }.
void parseExpression()
{
    // Stores if we should negate or not
    int shouldNegate = 0;
    
    // ["+" | "-"]
    if (token.type == plussym || token.type == minussym)
    {
        if (token.type == minussym)
        {
            shouldNegate = 1;
        }
        
        gotoNextToken();
    }
    
    parseTerm();
    
    if (shouldNegate)
    {
        // Negate the previously emitted code's register
        // Registers[lastStored] = -Registers[lastScored];
        emit(NEG, code[codeIndex - 1].R, code[codeIndex - 1].R, 0);
    }
    // For code generation, we need to know what value we're adding/subbing
    int firstTermCodeIndex = codeIndex - 1;
    // We're about to parse another item; increment register value
    currentRegister++;

    while (token.type == plussym || token.type == minussym)
    {
        token_type operatorType = token.type;
        
        gotoNextToken();
        parseTerm();
        currentRegister++;
        
        // For code generation, we need to know what value we're adding/subbing
        int newTermCodeIndex = codeIndex - 1;
        
        // For code emission:
        // Registers[first] = Registers[first] (+ | -) Registers[second];
        if (operatorType == plussym)
        {
            emit(ADD,
                 code[firstTermCodeIndex].R,
                 code[firstTermCodeIndex].R,
                 code[newTermCodeIndex].R);
        }
        else if (operatorType == minussym)
        {
            emit(SUB,
                 code[firstTermCodeIndex].R,
                 code[firstTermCodeIndex].R,
                 code[newTermCodeIndex].R);
        }
        
        // We just used the second register and won't need it again; decrement register.
        currentRegister--;
    }
    
    // We're done with the initial register and won't need it again; decrement register.
    currentRegister--;
}

// factor {("*"|"/") factor}
void parseTerm()
{
    parseFactor();
    
    currentRegister++;
    
    // The index of the register where we just parsed our factor
    int previousFactorRegister = code[codeIndex - 1].R;
    
    // Can mult/divide infinite times
    while (token.type == multsym || token.type == slashsym)
    {
        token_type originalSymbol = token.type;
        
        gotoNextToken();
        parseFactor();
        
        currentRegister++;
        
        int newestFactorRegister = code[codeIndex - 1].R;
        
        // Below emissions can be thought of like this:
        // Registers[first] = registers[first] ("*" | "/") registers[second];
        if (originalSymbol == multsym)
        {
            emit(MUL, previousFactorRegister, previousFactorRegister, newestFactorRegister);
        }
        else if (originalSymbol == slashsym)
        {
            emit(DIV, previousFactorRegister, previousFactorRegister, newestFactorRegister);
        }
        
        currentRegister--;
    }
    
    currentRegister--;
}

// ident | number | "(" expression")“.
void parseFactor()
{
    if (token.type == identsym)
    {
        int symbolIndex = findSymbolIndexWithName(token.symbol);
        
        if (symbolIndex == -1)
        {
            // Undeclared identifier
            errorMessage(11);
        }
        else if (symbolTable[symbolIndex].type == varType)
        {
            // Load the value from the symbol table
            // Registers[curReg] = stack[base(level, bp) + address];
            emit(LOD, currentRegister, symbolTable[symbolIndex].level, symbolTable[symbolIndex].address);
        }
        else if (symbolTable[symbolIndex].type == constType)
        {
            // Load the value from the constant symbol table
            // Registers[curReg] = constValue;
            emit(LIT, currentRegister, 0, atoi(symbolTable[symbolIndex].value));
        }
        else { errorMessage(100); } // TODO: Make a better error for this
        
        gotoNextToken();
    }
    else if (token.type == numbersym)
    {
        // Registers[curReg] = value;
        emit(LIT, currentRegister, 0, atoi(token.symbol));
        
        gotoNextToken();
    }
    else if (token.type == lparentsym)
    {
        gotoNextToken();
        parseExpression();

        // Right parenthesis expected
        if (token.type != rparentsym) { errorMessage(22); }

        gotoNextToken();
    }
    // Invalid factor
    else { errorMessage(27); }
}

// For code generation
// Returns the address of code index generation
int emit(OP op, int r, int l, int m)
{
    if (codeIndex == MAX_CODE_LENGTH) { errorMessage(28); }
    
    code[codeIndex].OP = op;
    code[codeIndex].R = r;
    code[codeIndex].L = l;
    code[codeIndex].M = m;
    codeIndex++;
    
    return codeIndex - 1;
}

// Enters in a symbol with the given information to the symbol table
// Returns the address of the symbol
int enterSymbol(symbolType type, char *name, char *value, int level, int address)
{
    if (symbolTableIndex == MAX_SYMBOL_TABLE_SIZE) { errorMessage(2); }
    
    symbolTable[symbolTableIndex].type = type;
    strcpy(symbolTable[symbolTableIndex].name, name);
    strcpy(symbolTable[symbolTableIndex].value, value);
    symbolTable[symbolTableIndex].level = level;
    symbolTable[symbolTableIndex].address = address;
    
    symbolTableIndex++;
    
    return symbolTableIndex - 1;
}

// Finds a symbol with the given name
int findSymbolIndexWithName(char* identifier)
{
    int index;
    for(index = symbolTableIndex - 1; index >= 0 ; index--) //CULPRIT
        if (strcmp(symbolTable[index].value, identifier) == 0)
            return index;
    return -1;
}

//Error messages for the PL/0 Parser
void errorMessage(int error)
{
    // Since we posted an error message, the error flag must be set.
    errorFlag = 1;
    
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
            break;
        case 31:
            printf("Symbol table size went over maximum number of symbols");
            fprintf(errorFile, "Error. Too many symbols generated.");
            break;
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
    {
        fprintf(outputFile, "%d %d %d %d\n", code[i].OP, code[i].R, code[i].L, code[i].M);
    }

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
    parseProgram();

    if (errorFlag == 1)
        exit(0);

    printCode();

    fclose(inputFile);
    fclose(outputFile);
    fclose(updatedToken);

    return 0;
}
