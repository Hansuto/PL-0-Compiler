#ifndef STATE_H_
#define STATE_H_

// Different operation types
typedef enum {
    LIT = 1,
    RET = 2,
    LOD = 3,
    STO = 4,
    CAL = 5,
    INC = 6,
    JMP = 7,
    JPC = 8,
    SIO = 9,
    NEG = 10,
    ADD = 11,
    SUB = 12,
    MUL = 13,
    DIV = 14,
    ODD = 15,
    MOD = 16,
    EQL = 17,
    NEQ = 18,
    LSS = 19,
    LEQ = 20,
    GTR = 21,
    GEQ = 22
} OP;

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
    eqsym = 9,
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

#endif
