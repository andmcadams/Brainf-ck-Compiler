#ifndef LEXAN_H
#define LEXAN_H

#include "token.h"
#include <stdio.h>
#include <stdlib.h>

TOKEN gettoken();
void printtoken(TOKEN tok);
TOKEN talloc();
int EOFFLG;
int tokcount;

#endif