#include "schemeitem.h"
#include "linkedlist.h"
#include "talloc.h"
#include "tokenizer.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <assert.h>

// Helper function to print a syntax error.
void syntaxError() {
    printf("Syntax Error!");
}

// Adds a Scheme Item (item) to the front of the provided stack (stack)
// If begins with quote '(..), will remove and replace with "quote"
// Otherwise will just add
SchemeItem *push(SchemeItem *stack, SchemeItem *item) {
    if (stack->type == CONS_TYPE && car(stack)->type == SINGLEQUOTE_TYPE) {
        // remove quote
        stack = cdr(stack);


        SchemeItem *quote_item = makeEmpty();
        quote_item->type = SYMBOL_TYPE;
        quote_item->s = talloc(6);
        strcpy(quote_item->s, "quote");

        SchemeItem *list_w_quote = makeEmpty();
        list_w_quote = cons(item, list_w_quote);
        list_w_quote = cons(quote_item, list_w_quote);
        return cons(list_w_quote, stack);
    }

    return cons(item, stack);
}

// Helper function to build tree from provided parse_stack and token
// Groups tokens based on matching parenthesis
// When open is found it pushes token onto stack
// When closed is found it pops until matching open is found, then pushes list onto stack
// 
// Handles quotes by removing ' and replacing it with quote
// Syntax error if unbalanced parenthesis
SchemeItem *addToParseTree(SchemeItem *parse_stack, int *current_depth, SchemeItem *token) { //token = car(current), 
    if (token->type == OPEN_TYPE) {
        *current_depth = *current_depth + 1; // add one to depth, one (
        return cons(token, parse_stack);
    }
    
    if (token->type == CLOSE_TYPE) {
        if (*current_depth == 0) {
            syntaxError();
        }

        SchemeItem *inner_list = makeEmpty();
        bool matched = false;

        while (parse_stack->type == CONS_TYPE) {
            SchemeItem *top = car(parse_stack);
            parse_stack = cdr(parse_stack);

            if ((top->type == OPEN_TYPE) || (top->type == OPENBRACKET_TYPE)) {
                matched = true;
                break;
            }

            inner_list = cons(top, inner_list);
        }

        if (matched == false) { // we never matched with an open type
            syntaxError();
        }

        *current_depth = *current_depth - 1;

        return push(parse_stack, inner_list);
    }

    if (token->type == SINGLEQUOTE_TYPE) {
        return cons(token, parse_stack);
    }

    // no open or closed parthesis
    return push(parse_stack, token);
}

// Converts a linked list of tokens into a parse tree
// Goes through each token using the helper function addToParseTree for each token
// Throws syntax error if not all open parthesis are closed
// Reverses stack for original order of tokens
SchemeItem *parse(SchemeItem *tokens) {
    int *current_depth = talloc(sizeof(int));
    *current_depth = 0;
    SchemeItem *parse_stack = makeEmpty();

    SchemeItem *current = tokens;
    assert(current != NULL && "Error (parse): null pointer");

    // Go through each token
    while (current->type != EMPTY_TYPE) {
        SchemeItem *token = car(current);
        parse_stack = addToParseTree(parse_stack, current_depth, token);
        current = cdr(current);
    }
    if (*current_depth != 0) {
        syntaxError();
    }

    parse_stack = reverse(parse_stack);

    return parse_stack;
}

// A helper function that prints the provided SchemeItem pointed to by (item)
// For a list of cons cells, Will print each item with a space between them until reaches end of list
void printItem(SchemeItem *item) {
    switch (item->type){
        case CONS_TYPE: 
            printf("(");
            
            SchemeItem *current = item;
            bool first = true;

            while (current->type == CONS_TYPE){
                if (first == false) {
                    printf(" ");
                }

                printItem(car(current));
                first = false;

                current = cdr(current);
            }
            if (current->type != EMPTY_TYPE) {
                printf(" . ");
                printItem(current);
            }
            printf(")");
            break;
        case EMPTY_TYPE:
            printf("()"); 
            break;
        case INT_TYPE:
            printf("%i", item->i); 
            break;
        case DOUBLE_TYPE:
            printf("%f", item->d); 
            break;
        case STR_TYPE:
            printf("%s", item->s); 
            break;
        case SYMBOL_TYPE:
            printf("%s", item->s); 
            break;
        case BOOL_TYPE:
            printf("%s", item->s); 
            break;
        case CLOSURE_TYPE:
            printf("#<procedure>");
            break;
        case VOID_TYPE:
            break;
        default: 
            break;
    }
}

// Prints the tree like it is scheme code
void printTree(SchemeItem *tree) {
    SchemeItem *current = tree;
    
    while (current->type == CONS_TYPE) {
        printItem(car(current));
        if (cdr(current)->type == CONS_TYPE) { // we have something next
            printf(" ");
        }

        current = cdr(current);
    }
    printf("\n");
}