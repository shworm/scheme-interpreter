# Scheme Interpreter

Scheme interpreter in C with lexical scoping and recursive evaluation.

This interpreter is seperated into multiple pieces. The interpretation process follows these steps:

- tokenizer.c (tokenizer.h)
    - Reads an input file (.scm) and tokenizes each character. Creates a SchemeItem struct for each token (see schemeitem.h), based on the token's type. 
    - Creates a single linked list using SchemeItem of type CONS (has a value, and a tail), and passes this list on to the parser.

- parser.c (parser.h)
    - The parser recieves the linked list of tokens, and creates a parse tree based on function calls and parentheses.
    - Creates a linked list stack to help group tokens toghether in the same context.
    - /parse.png
    
# Overview


