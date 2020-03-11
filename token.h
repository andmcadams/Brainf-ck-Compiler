#ifndef TOKEN_H
#define TOKEN_H
typedef struct tokn {
	int    tokentype;
	struct tokn * operands;
	struct tokn * link;
	int whichval;
} *TOKEN;

//TOKEN TYPES
#define OPERATOR		1
#define NUMBER			2
#define IDENTIFIER 	3


//OPERATOR VALUES
#define IFOP			0
#define LABELOP		1
#define GOTOOP		2
#define PLUSOP		3
#define MINUSOP		4
#define EQOP			5
#define NEOP 			6
#define PROGNOP		7
#define DOTOP			8
#define COMMAOP 	9

//CHARACTER ASCII VALUES
#define STBRACK 	91
#define ENBRACK 	93
#define DOT 		  46
#define COMMA 		44
#define PLUS 		  43
#define MINUS 		45
#define RSHIFT 		62
#define LSHIFT 		60

#endif