#ifndef _INTERPRETER
#define _INTERPRETER

#include "schemeitem.h"

void interpret(SchemeItem *tree);
SchemeItem *eval(SchemeItem *tree, Frame *frame);

#endif

