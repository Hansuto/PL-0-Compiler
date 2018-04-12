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
#include <string.h>

#define identMax 11
#define numMax 5
#define MAX_CODE_SIZE 2000

//token struct
typedef struct {
    int class;
    char lexeme[identMax];
} token_t;

int encounteredError = 0, currentTokenIndex = 0;
int g = 0, lex[MAX_CODE_SIZE];
char codeArray[MAX_CODE_SIZE];
token_t tokenList[MAX_CODE_SIZE];

typedef enum {
    nocomment = 0,
    singleline = 1,
    multiline = 2
} comment_type;

char specialSymbols[] = {
    '+',
    '-',
    '*',
    '/',
    '(',
    ')',
    '=',
    ',',
    '.',
    '<',
    '>',
    ';',
    ':'
};

//state values
typedef enum {
    start = 0,
    nulsym = 1,
    identsym = 2,
    numbersym = 3,
    plussym = 4,
    minussym = 5,
    multsym = 6,
    slashsym = 7,
    oddsym = 8,
    eqlsym = 9,
    neqsym = 10,
    lessym = 11,
    leqsym = 12,
    gtrsym = 13,
    geqsym = 14,
    lparentsym = 15,
    rparentsym = 16,
    commasym = 17,
    semicolonsym = 18,
    periodsym = 19,
    becomessym = 20,
    beginsym = 21,
    endsym = 22,
    ifsym = 23,
    thensym = 24,
    whilesym = 25,
    dosym = 26,
    callsym = 27,
    constsym = 28,
    varsym = 29,
    procsym = 30,
    writesym = 31,
    readsym = 32,
    elsesym = 33,
    linecommentsym = 34,
    multilinecommentstartsym = 35,
    multilinecommentendsym = 36,
    assignmentsym = 37,
    ignoreUntilWhiteSpace = 1000
} token_type;

//reserved words
const char *reservedWord[ ] = {
    "null",
    "begin",
    "call",
    "const",
    "do",
    "else",
    "end",
    "if",
    "odd",
    "procedure",
    "read",
    "then",
    "var",
    "while",
    "write"
};

//reserved word #
const int wsym [ ] = {
    nulsym,
    beginsym,
    callsym,
    constsym,
    dosym,
    elsesym,
    endsym,
    ifsym,
    oddsym,
    procsym,
    readsym,
    thensym,
    varsym,
    whilesym,
    writesym
};

int isLetter(char c) {
    //checks lowercase
    int val = c - 'a';

    if(val >= 0 && val < 26)
        return 1;

    //checks uppercase
    int val2 = c - 'A';

    if(val2 >= 0 && val2 < 26)
        return 1;

    return 0;
}

int isDigit(char c) {
    int val = c - '0';

    if(val >=0 && val < 10)
        return 1;

    return 0;
}

int isSpecialSymbol(char c) {
    int i;

    for(i=0; i < 13; i++)
        if(c == specialSymbols[i])
            return 1;

    return 0;
}

int isWhiteSpace(char c) {
    if(c == ' ' || c == '\n' || c== '\r' || c == '\t' || c == '\r')
        return 1;
    return 0;
}

int isValidChar(char c) {
    return (isLetter(c) || isDigit(c) || isSpecialSymbol(c) || isWhiteSpace(c));
}

int grabToken(char lexeme[]) {
    int i;

    for(i = 0; i < 15; i++)
        if(strcmp(lexeme, reservedWord[i]) == 0)
            return wsym[i];

    return identsym;
}

void printToken(token_t tokenToPrint) {
    //prints to the lexemetable
    printf("%s\t\t%d\n", tokenToPrint.lexeme, tokenToPrint.class);
}

void printLexTable() {
    int i;
    
    printf("\nLexeme Table:\n");
    printf("lexeme\t\ttoken type\n");
    
    for(i = 0 ; i < currentTokenIndex ; i++) {
        printToken(tokenList[i]);
    }
}

void appendTokenToTokenList(token_t tokenToAdd) {
    tokenList[currentTokenIndex++] = tokenToAdd;
}

void resetToken(token_t *token) {
    int i;
    lex[g++] = token->class;
    
    if(token->class == identsym || token->class == numbersym)
        lex[g++] = token->lexeme[0];
    
    //reset the token
    for(i = 0; i < 11; i++)
        token->lexeme[i] = '\0';
    token->class = -1;
}

//separates tokens
void performLexAnalysis(char code[]) {
    int i, index = 0, state = 0, codeLength = strlen(code);

    //special symbols with associated values
    int ssym[256];
    ssym['+'] = plussym;
    ssym['-'] = minussym;
    ssym['*'] = multsym;
    ssym['/'] = slashsym;
    ssym['('] = lparentsym;
    ssym[')'] = rparentsym;
    ssym['='] = eqlsym;
    ssym[','] = commasym;
    ssym['.'] = periodsym;
    ssym['#'] = neqsym;
    ssym['<'] = lessym;
    ssym['>'] = gtrsym;
    ssym['$'] = leqsym;
    ssym['%'] = geqsym;
    ssym[';'] = semicolonsym;

    //initialization
    token_t token;
    token.class = -1;

    for(i = 0; i < 11; i++)
        token.lexeme[i] = '\0';

    while(index < codeLength)
    {
        char currentChar = code[index];

        switch(state)
        {
            //start state
            case start:
                if(isDigit(currentChar))
                    //NUM state
                    state = numbersym;

                else if(isLetter(currentChar))
                    //ID state
                    state = identsym;

                else if(isSpecialSymbol(currentChar)) {
                    //Assignment State
                    if(currentChar == ':') {
                        state = assignmentsym;
                        //this occurs if ':' is the final character
                        if(index == codeLength-1) {
                            encounteredError = 1;
                            printf("Error: Invalid symbol.");
                            return;
                        }
                    }
                    //Special Symbol state
                    else
                        state = ssym[currentChar];
                }

                else if(isWhiteSpace(currentChar))
                    //Ignore white space
                    break;
                else {
                    // Unhandled!
                    encounteredError = 1;
                    printf("Error: Invalid symbol.");
                    return;
                }

                //Fills lexeme with current char
                token.lexeme[strlen(token.lexeme)] = currentChar;
                break;

            //identifier state
            case identsym:
                //Return to beginning state once identifier is completed
                if(isSpecialSymbol(currentChar) || isWhiteSpace(currentChar)) {
                    token.class = state;
                    appendTokenToTokenList(token);
                    resetToken(&token);
                    state = 0;
                    index--;
                }

                else if(strlen(token.lexeme) > identMax) {
                    state = ignoreUntilWhiteSpace;
                    encounteredError = 1;
                    printf("Error: Identifier too long.");
                    return;
                }
                //Identifies reserved word or identifier
                else {
                    token.lexeme[strlen(token.lexeme)] = currentChar;
                    state = grabToken(token.lexeme);
                }

                break;

            //number state
            case numbersym:
                //Adds character to lexeme
                if(isDigit(currentChar)) {
                    // If we just went over the maximum length for a number
                    if(strlen(token.lexeme) > numMax) {
                        state = ignoreUntilWhiteSpace;
                        encounteredError = 1;
                        printf("Error: Number too large.");
                        return;
                    }
                    //Adds digit to lexeme
                    token.lexeme[strlen(token.lexeme)] = currentChar;
                } else if(isLetter(currentChar)) {
                    state = ignoreUntilWhiteSpace;
                    encounteredError = 1;
                    printf("Error: Invalid identifier.");
                    return;
                }

                //Return to beginning state once number is completed
                else {
                    token.class = state;
                    appendTokenToTokenList(token);
                    resetToken(&token);
                    state = 0;

                    index--;
                }
                break;

            //reserved words cases
            case nulsym:
            case oddsym:
            case beginsym:
            case endsym:
            case ifsym:
            case thensym:
            case whilesym:
            case dosym:
            case callsym:
            case constsym:
            case varsym:
            case procsym:
            case writesym:
            case readsym:

            case elsesym:
                if(isDigit(currentChar) || isLetter(currentChar)) {
                    //Adds character to lexeme
                    token.lexeme[strlen(token.lexeme)] = currentChar;
                    state = identsym;
                }

                //Goes back too initial state is reserved word is a token
                else {
                    token.class = state;
                    appendTokenToTokenList(token);
                    resetToken(&token);
                    state = 0;
                    index--;
                }
                break;

            //special character cases
            case slashsym:
            case plussym:
            case minussym:
            case multsym:
            case eqlsym:
            case neqsym:
            case leqsym:
            case geqsym:
            case lparentsym:
            case rparentsym:
            case commasym:
            case semicolonsym:
            case periodsym:

            case becomessym:
                token.class = state;
                appendTokenToTokenList(token);
                resetToken(&token);
                state = 0;
                index--;
                break;

            case lessym:
                // "<="
                if(currentChar == '=') {
                    token.lexeme[strlen(token.lexeme)] = currentChar;
                    state = leqsym;
                }

                // "<>"
                else if(currentChar == '>') {
                    token.lexeme[strlen(token.lexeme)] = currentChar;
                    state = neqsym;
                }

                // "<"
                else {
                    token.class = state;
                    appendTokenToTokenList(token);
                    resetToken(&token);
                    state = 0;
                    index--;
                }
                break;

            case gtrsym:
                // ">="
                if(currentChar == '=') {
                    token.lexeme[strlen(token.lexeme)] = currentChar;
                    state = geqsym;
                }

                // ">"
                else {
                    token.class = state;
                    appendTokenToTokenList(token);
                    resetToken(&token);
                    state = 0;
                    index--;
                }
                break;

            // Have ":". Need "=" to assign properly.
            case assignmentsym:
                // ":=" is the only valid following sequence
                if(currentChar == '=') {
                    token.lexeme[strlen(token.lexeme)] = currentChar;
                    state = becomessym;
                } else {
                    encounteredError = 1;
                    printf("Error: Invalid symbol.");
                    return;
                }

                break;
            case ignoreUntilWhiteSpace:
                if(isWhiteSpace(currentChar)) {
                    state = start;
                }
                break;
        }

        index++;
    }
    if(state != 0) {
        //prints last token
        token.class = state;
        appendTokenToTokenList(token);
        resetToken(&token);
    }
}

void printCodeOfSize(int size, char *withName) {
    int i;
    
    printf("Source Program:%s\n", withName);
    for(i = 0; i < size; i++)
        printf("%c", codeArray[i]);
    
    for(i = 0; i < size; i++)
        if(codeArray[i] == '\t' || codeArray[i] == '\r') codeArray[i] = ' ';
    
    printf("\n");
}

int getCodeFromFile(FILE* file) {
    char temp, previousCharacter = '_';
    int codeSize = 0;
    // The current way we're scanning (no_comment, singleline, multiline)
    comment_type scanningType = nocomment;
    
    while(fscanf(file, "%c", &temp) != EOF) {
        // If we have no comment, check if a comment was made. If not, store the character.
        if (scanningType == nocomment) {
            if (previousCharacter == '/' && temp == '*') {
                codeSize--;
                scanningType = multiline;
            } else {
                codeArray[codeSize++] = temp;
            }
        } else { // Otherwise, check if the comment ended. Do not store results.
            if (scanningType == multiline && previousCharacter == '*' && temp == '/') {
                scanningType = nocomment;
            }
        }
        
        previousCharacter = temp;
    }
    
    return codeSize;
}

int lexer(char * file) {
    FILE *ifp = fopen(file, "r");

    int codeLength = getCodeFromFile(ifp);

    fclose(ifp);
    
    printCodeOfSize(codeLength, file);

    performLexAnalysis(codeArray);

    if (!encounteredError) {
        printLexTable();
        
        printf("\nLexeme List:\n");
        for(g = 0 ; g < currentTokenIndex ; g++) {
            token_t tokenToTest = tokenList[g];
            
            printf("%d ", tokenToTest.class);
            if (tokenToTest.class <= 3)
                printf("%s ", tokenToTest.lexeme);
        }
        printf("\n");
    }

    return 0;
}

