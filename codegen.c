/* codgen.c       Generate Assembly Code for x86         15 May 13   */

/* Copyright (c) 2013 Gordon S. Novak Jr. and The University of Texas at Austin
    */

/* Starter file for CS 375 Code Generation assignment.           */
/* Written by Gordon S. Novak Jr.                  */

/* This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License (file gpl.text) for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA. */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "token.h"
#include "genasm.h"
#include "codegen.h"

void genc(TOKEN code);

/* Set DEBUGGEN to 1 for debug printouts of code generation */
#define DEBUGGEN 0

int nextlabel;    /* Next available label number */
int stkframesize;   /* total stack frame size */

int regMask = 0;
int qregMask = 0;
/* Top-level entry for code generator.
   pcode    = pointer to code:  (program foo (output) (progn ...))
   varsize  = size of local storage in bytes
   maxlabel = maximum label number used so far

Add this line to the end of your main program:
    gencode(parseresult, blockoffs[blocknumber], labelnumber);
The generated code is printed out; use a text editor to extract it for
your .s file.
         */

void gencode(TOKEN code, int maxlabel)
{
 nextlabel = maxlabel + 1;
 stkframesize = asmentry("_main", 16);
 genc(code);
 asmexit("_main");
}

/* Trivial version: always returns RBASE + 0 */
/* Get a register.   */
/* Need a type parameter or two versions for INTEGER or REAL */
int getreg()
{
	int i;
    for(i = 0; i <= RMAX; i++)
    {
      if(((1 << i) & regMask) == 0)
      {
        used(i);
        return i;
      }
    }
    return RBASE;
}

void clearreg()
{
  regMask = 0;
}

void unused(int reg)
{
  regMask ^= (1 << reg);
}

void used(int reg)
{
  regMask |= (1 << reg);
}


/* Trivial version */
/* Generate code for arithmetic expression, return a register number */
int genarith(TOKEN code)
{   
  int num, reg;
  int stored = 0;

  if (DEBUGGEN)
  { 
    printf("genarith\n");
  };
  switch ( code->tokentype )
  { 

  };
  return reg;
}


/* Generate code for a Statement from an intermediate-code form */
void genc(TOKEN code)
{  
  TOKEN tok, lhs, rhs;
  int reg, reg2, offs, offs2;
  if (DEBUGGEN)
  { 
    printf("genc\n");
  };
  if ( code->tokentype != OPERATOR )
  { 
    printf("Bad code token");
  };
  switch ( code->whichval )
  { 
    case PROGNOP:
      tok = code->operands;
      while ( tok != NULL )
      {  
      	genc(tok);
        tok = tok->link;
      };
    break;
    //plusop, minusop, eqop, neop
    case LABELOP:
    	asmlabel(code->operands->whichval);
    break;
    case GOTOOP:
    	asmjump(JMP, code->operands->whichval);
    break;
    case COMMAOP:
	    printf("movb	$0, %%al\n");
      printf("callq	_getchar\n");
      printf("xorl	%%ecx, %%ecx\n");
      printf("movb	%%al, %%dl\n");
      printf("movq	_ptr(%%rip), %%rsi\n");
      printf("movb	%%dl, (%%rsi)\n");
      printf("movl	%%ecx, %%eax\n");
    break;
    case PLUSOP:
    	if(code->operands->whichval == 0)
    	{
 //  movq	_i@GOTPCREL(%rip), %rax
	// movq	_tape@GOTPCREL(%rip), %rcx
    		//save the current value in the current cell
    		//update the cell counter
    		printf("movq	_ptr(%%rip), %%rax\n");
        printf("addq	$%d, %%rax\n", code->operands->link->whichval);
        printf("movq	%%rax, _ptr(%%rip)\n");
    		//load the current value in the current cell in EBX
    	}
    	else
    	{
    			printf("movq	_ptr(%%rip), %%rax\n");
          printf("movb	(%%rax), %%cl\n");
          printf("addb	$%d, %%cl\n", code->operands->link->whichval);
          printf("movb	%%cl, (%%rax)\n");
    	}
    break;
    case MINUSOP:
	    if(code->operands->whichval == 0)
	  	{
	//  movq	_i@GOTPCREL(%rip), %rax
	// movq	_tape@GOTPCREL(%rip), %rcx
	  		//save the current value in the current cell
	  		//update the cell counter
    		printf("movq	_ptr(%%rip), %%rax\n");
        printf("subq	$%d, %%rax\n", code->operands->link->whichval);
        printf("movq	%%rax, _ptr(%%rip)\n");
	  		//load the current value in the current cell in EBX
        // if ptr > tape + size
        //    ptr = ptr % size
        // should be an aux func at first, optimization should allow for in line
	  	}
	  	else
	  	{
    			printf("movq	_ptr(%%rip), %%rax\n");
          printf("movb	(%%rax), %%cl\n");
          printf("subb	$%d, %%cl\n", code->operands->link->whichval);
          printf("movb	%%cl, (%%rax)\n");
	  	}
    //   if(code->operands->whichval == 0)
    // 	{
    // 		asmstrrm(MOVL, ECX, 0, EBX, 8, "cell[i]");
    // 		asmimmed(SUBL, code->operands->link->whichval, EBX);
				// asmldrrm(MOVL, 0, EBX, 8, ECX, "cell[I]");
    // 		//save the current value in the current cell
    // 		//update the cell counter
    // 		//load the current value in the current cell in EBX
    // 	}
    // 	else
    // 		asmimmed(SUBQ, code->operands->link->whichval, EDX);
    break;
    case DOTOP:
    /* Example:
   asmldrrm(MOVL, -8, RAX, 4, ECX, code->stringval); -8(%rbp,%rax,4) --> %ecx */
			printf("movq _ptr(%%rip), %%rax\n");
      printf("movsbl (%%rax), %%edi\n");
    	asmcall("_putchar");
    break;
    case IFOP:
/**
	* TOK is an operator tok with a whichval of STBRACK
	* returns a TOK with progn( (if (i == 0) (goto 1) (progn(elsepart body))) (label 1)) where elsepartbody
	* ends in an if(i != 0 goto LABEL 0)
**/
    	//gen == i 0
    	//gen goto int
    	//genc body
    	//gen label int

    printf("movq	_ptr(%%rip), %%rdx\n");
    printf("movsbl	(%%rdx), %%eax\n");
    printf("cmpl	$0, %%eax\n");
    asmjump(code->operands->whichval == EQOP? JE: JNE , code->operands->link->operands->whichval);
    if(code->operands->link->link != NULL)
	  	genc(code->operands->link->link);
    break;
  };
}