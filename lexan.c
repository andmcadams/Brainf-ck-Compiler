#include "lexan.h"


/**
	* LEXAN should parse the text of the file and create tokens for each character.
	* Each token should have its tokentype and whichval set based on the character.
**/


TOKEN talloc()           /* allocate a new token record */
{ TOKEN tok;
	tok = (TOKEN) calloc(1,sizeof(struct tokn));
	if ( tok != NULL ) return (tok);
	else printf("talloc failed.");
	return NULL;
}

char peekchar()
{
	char c = getchar();
	ungetc(c, stdin);
	return c;
}

void skipothers()
{
	char c = peekchar();
	while (c != EOF && c != PLUS && c != MINUS && c != RSHIFT && c != LSHIFT && c != STBRACK && c != ENBRACK && c != DOT && c != COMMA)
	{
		getchar();
		c = peekchar();
	}
}

TOKEN gettoken()
{
	skipothers();
	TOKEN tok = (TOKEN) talloc();
	char c = peekchar();
	if(c != EOF)
	{
		c = getchar();
		tok->tokentype = OPERATOR;
		tok->whichval = c;
	}
	else
		{
			EOFFLG = 1;
			return NULL;
		}
	return tok;
}

void printtoken(TOKEN tok)
{
	printf("tokentype = %d\ttoken = %c\n", tok->tokentype, tok->whichval);
}

