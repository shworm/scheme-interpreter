#include <stdio.h>
#include "tokenizer.h"
#include "schemeitem.h"
#include "linkedlist.h"
#include "parser.h"
#include "talloc.h"
#include "interpreter.h"

int main() {

    SchemeItem *list = tokenize();
    SchemeItem *tree = parse(list);
    interpret(tree);

    tfree();
    return 0;
}
