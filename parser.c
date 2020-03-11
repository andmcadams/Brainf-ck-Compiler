#include "lexan.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <regex.h>
#include "codegen.h"

TOKEN savedToken = NULL;
TOKEN parseResult = NULL;
int tokenCounter = 0;
int processedToks = 0;
int mylabel = 0;
char* operators[] = {"IF", "LABEL", "GOTO", "+", "-", "==", "!=", "PROGN", ".", ","};
char* identifiers[] = {"I", "cell[I]"};
TOKEN parseAllTokens();
TOKEN parsetoken();

TOKEN gettok()
{
	tokenCounter++;
	if(savedToken == NULL) return gettoken();

	TOKEN tok = savedToken;
	savedToken = NULL;
	return tok;
}

TOKEN peektok()
{
	if(savedToken == NULL) return savedToken = gettoken();

	return savedToken;
}


TOKEN makeoperator(int op)
{
	TOKEN tok = talloc();
	tok->tokentype = OPERATOR;
	tok->whichval = op;
	return tok;
}

TOKEN makecellcounter()
{
	TOKEN tok = talloc();
	tok->tokentype = IDENTIFIER;
	tok->whichval = 0;
	return tok;
}

TOKEN makecell()
{
	TOKEN tok = talloc();
	tok->tokentype = IDENTIFIER;
	tok->whichval = 1;
	return tok;
}

TOKEN makeinteger(int intval)
{
	TOKEN tok = talloc();
	tok->tokentype = NUMBER;
	tok->whichval = intval;
	return tok;
}

TOKEN makelabel()
{
	TOKEN tok = talloc();
	tok->tokentype = OPERATOR;
	tok->operands = makeinteger(mylabel++);
	tok->whichval = LABELOP;
	return tok;
}

TOKEN makegoto(TOKEN label)
{
	TOKEN tok = talloc();
	tok->tokentype = OPERATOR;
	tok->whichval = GOTOOP;
	tok->operands = makeinteger(label->operands->whichval);
	return tok;
}

TOKEN makeprogn()
{
	TOKEN tok = talloc();
	tok->tokentype = OPERATOR;
	tok->whichval = PROGNOP;
	return tok;
}

TOKEN makebinop(TOKEN op, TOKEN lhs, TOKEN rhs)
{
	op->operands = lhs;
	rhs->link = NULL;
	lhs->link = rhs;
	return op;
}

TOKEN parsemath(TOKEN tok)
{
	int sum = 0;
	TOKEN next, amount, cell;
	cell = makecell();
	while(tok->tokentype == OPERATOR && (tok->whichval == MINUS || tok->whichval == PLUS))
	{
		if(tok->whichval == PLUS)
			sum++;
		else
			sum--;
		next = peektok();
		if(next->tokentype == OPERATOR && (next->whichval == MINUS || next->whichval == PLUS))
		{
			tok = gettok();
		}
		else
			break;
	}
	if(sum < 0)
	{
		tok->whichval = MINUSOP;
		sum *= -1;
		amount = makeinteger(sum);
		makebinop(tok, cell, amount);
		return tok;
	}
	else if(sum > 0)
	{
		tok->whichval = PLUSOP;
		amount = makeinteger(sum);
		makebinop(tok, cell, amount);
		return tok;
	}
	else
		return NULL;
}

TOKEN parseshift(TOKEN tok)
{
	int sum = 0;
	TOKEN next, amount, cell;
	cell = makecellcounter();

	while(tok->tokentype == OPERATOR && (tok->whichval == LSHIFT || tok->whichval == RSHIFT))
	{
		if(tok->whichval == RSHIFT)
			sum++;
		else
			sum--;
		next = peektok();
		if(next->tokentype == OPERATOR && (next->whichval == LSHIFT || next->whichval == RSHIFT))
		{
			tok = gettok();
		}
		else
			break;
	}
	if(sum < 0)
	{
		tok->whichval = MINUSOP;
		sum *= -1;
		amount = makeinteger(sum);
		makebinop(tok, cell, amount);
		return tok;
	}
	else if(sum > 0)
	{
		tok->whichval = PLUSOP;
		amount = makeinteger(sum);
		makebinop(tok, cell, amount);
		return tok;
	}
	else
		return NULL;
}

/**
	* TOK is an operator tok with a whichval of STBRACK
	* returns a TOK with progn((label 0) (if (i == 0) (goto 1) (progn(elsepart body))) (label 1)) where elsepartbody
	* ends in an (GOTO 0)
**/
TOKEN parseloop(TOKEN tok)
{

/**
*	PEEPHOLE OPTIMIZATION
* 	if a loop contains only - or only + (overflows will eventually loop)
*				either return a warning (possible endless loop)
*				or optimize to be a (cell val = 0) if there is only 1 minus sign (possibly works for other primes, need to be looked into)
*/

	TOKEN progn = makeprogn();
	TOKEN elseprogn = makeprogn();

	TOKEN stlabel = makelabel();
	TOKEN endlabel = makelabel();

	TOKEN gotostart = makegoto(stlabel);
	TOKEN gotoend = makegoto(endlabel);

	TOKEN begif = makeoperator(IFOP);
	TOKEN eqop = makeoperator(EQOP);
	TOKEN begcell = makecell();
	TOKEN begzero = makeinteger(0);

	makebinop(eqop, begcell, begzero);


	int end = 0;
	TOKEN front, next, chain;
	front = NULL;
	chain = front;
	while(!end)
	{
		next = parsetoken();
		if(next->whichval == ENBRACK)
			end = 1;
		else
		{
			if(front == NULL)
			{
				front = next;
				chain = front;
			}
			else {
				chain->link = next;
				chain = chain->link;
			}
		}

	}
/**
	* TOK is an operator tok with a whichval of STBRACK
	* returns a TOK with progn( (if (i == 0) (goto 1) (progn(elsepart body))) (label 1)) where elsepartbody
	* ends in an if(i != 0 goto LABEL 0)
**/
	// (PROGN (LABEL 0) (IF (x == 0) (then GOTO 1) (PROGN( (body) (GOTO 0))) (LABEL 1) ))
	progn->operands = stlabel;
	stlabel->link = begif;
	begif->link = endlabel;

	//front = the front of the chain
	//next is garbage
	//chain = last in the chain
	begif->operands = eqop;
	eqop->link = gotoend;
	gotoend->link = elseprogn;
	chain->link = gotostart;
	elseprogn->operands = front;
	return progn;
}

TOKEN parsetoken()
{
	int which;
	TOKEN next = gettok();
		//printf("tokentype: %d\twhichval: %d\n", next->tokentype, next->whichval);

	if(next == NULL)
		exit(-1);
	which = next->whichval;
	if(which == PLUS || which == MINUS)
		next = parsemath(next);
	else if(which == RSHIFT || which == LSHIFT)
		next = parseshift(next);
	else if(which == STBRACK)
		next = parseloop(next);
	else if(which == DOT)
		next->whichval = DOTOP;
	else if(which == COMMA)
		next->whichval = COMMAOP;
	return next;
}

TOKEN parseAllTokens()
{
	TOKEN front, next, last;
	front = NULL;
	int cbrackflg = 0;
	while((next = peektok()) != NULL && cbrackflg == 0)
	{
		next = parsetoken();

		if(next != NULL)
		{
			if(front == NULL)
				{
					front = next;
					last = front;
				}
			else
				{
					last->link = next;
					last = next;
				}
		}
	}
	return front;
}

void printChain(TOKEN front)
{
	int done = 0;
	int i;
	TOKEN next = front;
	if(front == NULL)
		done = 1;
	while(done == 0)
	{
		tokcount++;
		printf("(");
		switch(next->tokentype)
		{
			case IDENTIFIER: printf("%s", identifiers[next->whichval]);
			break;
			case OPERATOR: printf("%s ", operators[next->whichval]);
											printChain(next->operands);
			break;
			case NUMBER: printf("%d", next->whichval);
			break;
		}
		printf(")");
		next = next->link;
		if(next == NULL)
		{			
			done = 1;
		}
	}
}

bool flag_asm, flag_obj, flag_name;
char* filename;
char* name_regex = "([A-Za-z0-9]+)[.]";
size_t maxMatches = 2;
size_t maxGroups = 3;
  
regex_t regexCompiled;
regmatch_t groupArray[3];

int main(int argc, char *argv[])
{
	//./bfc filename.bf [-So] [outputname]
	//output name is inferred to be filename if there is no other
	//output to stdin
	//if S flag, then save it as a file
	int c, len;
	if(argc < 2)
	{
		printf("NEED MOAR ARGS MY DUDE\n");
		exit(0);
	}
	else
	{
		c = 1;
		char* arg;
		while(c < argc)
		{
			arg = argv[c];
			if(c == 1)
			{
				if((len = strlen(arg)) > 3 && (!strcmp(&arg[len-3], ".bf") || !strcmp(&arg[len-2], ".b")))
				{

					if (regcomp(&regexCompiled, name_regex, REG_EXTENDED))
			    {
			      printf("Could not compile regular expression.\n");
			      exit(0);
			    }
			    if (!regexec(&regexCompiled, arg, maxGroups, groupArray, 0))
			    {
			    	char name[groupArray[0].rm_eo - groupArray[0].rm_so];
			    	strncpy(name, &arg[groupArray[0].rm_so], sizeof(name) - 1);
			    	name[sizeof(name) - 1] = 0;
			    	filename = malloc(strlen(name));
			    	strcpy(filename, name);
			    	printf("Filename: %s\n", filename);
			    }
					FILE* f = fopen(arg, "r");
					dup2(fileno(f), STDIN_FILENO);
					fclose(f);
				}
				else
				{
					printf("Please pass in a valid Brainfuck file (.bf or .b)\n");
					exit(0);
				}
			}
			else if(c < 5)
			{
				if(arg[0] == '-' && (arg[1] == 'S' || arg[1] == 'o') && strlen(arg) == 2)
				{
					if(arg[1] == 'S')
					{
						if(flag_asm)
							exit(0);
						flag_asm = true;
					}
					else if(arg[1] == 'o')
					{
						if(flag_obj)
							exit(0);
						flag_obj = true;
					}				
				}
				else
				{
					if(flag_name)
						exit(0);
					flag_name = true;
					filename = arg;
				}
			}
			c++;
		}
	}

	char* newname;
	if(flag_asm)
	{
	newname = malloc(strlen(filename) + 3);
	printf("%s: %lu\n", filename, strlen(filename) + 2);
	strcpy(newname, filename);

	strcpy(&newname[strlen(newname)], ".s");
	printf("file.bf: %s\n", newname);
		FILE *fp = fopen(newname, "w");
		dup2(fileno(fp), STDOUT_FILENO);
		fclose(fp);
	}
	else
	{
		newname = malloc(8);
		strcpy(newname, "t7oi7.s");
		FILE *dummy = fopen("t7oi7.s", "w");
		dup2(fileno(dummy), STDOUT_FILENO);
		fclose(dummy);
	}


	//Need to grab file name as argument 1
	//Take the name of that argument and use it to create an object file of the same name
	//might need to exec ("gcc x.s -o x") where x is the name of the file (w/o path)
	//Possibly add a feature to print out the assembly code, similar to gcc
	//List optimizations somewhere and possibly give option to disable them, as some people will want to see their
	//code as is in order to understand the brainfuck language.
	//Suggest using interpreter and debugger combo before compiling, since compilation cannot easily show errors
	//Give way to contact so that in case there is an error with an optimization
	//Add option to change the number of cells in the tape.
	//Make the tape loop (look into quick modulo methods in assembly)
	//PEEPHOLE OPTIMIZATION: omit any <,>,+,- that evaluate to a total of 0 ("<><>", "+-", "<<<>>>", etc.)
 	EOFFLG = 0;
 	tokcount = 0;
 	TOKEN parseResult = makeprogn();
 	parseResult->operands = parseAllTokens();
 	//printChain(parseResult);
 	// printf("\n");
 	gencode(parseResult, mylabel);
 	if(!flag_asm)
	{

		dup2(STDIN_FILENO, STDOUT_FILENO);
	}
 	if(flag_obj)
 	{
	 	char cmd[9 + strlen(newname) + strlen(filename)];
	 	strcpy(cmd, "gcc ");
	 	strcpy(&cmd[strlen(cmd)], newname);	
		strcpy(&cmd[strlen(cmd)], " -o ");
		strcpy(&cmd[strlen(cmd)], filename);
	 	system(cmd);
 	}
 	if(!flag_asm)
 		system("rm t7oi7.s");
}
