#include "schemeitem.h"

#ifndef _PARSER
#define _PARSER

// Takes a list of tokens from a Scheme program, and returns a pointer to a
// parse tree representing that program.
SchemeItem *parse(SchemeItem *tokens);


// Prints the tree to the screen in a readable fashion. It should look just like
// Scheme code; use parentheses to indicate subtrees.
void printTree(SchemeItem *tree);

void printItem(SchemeItem *item);


#endif
