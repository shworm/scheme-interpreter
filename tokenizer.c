#include "schemeitem.h"
#include "linkedlist.h"
#include "talloc.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>


// Reads an input file through the command like using fgetc.
// Tokenizes the given scheme file by catagorizing each token into a type based on the first character read
// Creates a linked list of all the tokens in the file.
//
// Important variables:
// - current_token -> character array of the current token we are creating/reading
// - index -> helper int index for the index of the current_token string
// - state -> tells us what kind of token we are creating, and how we should proceed (DEFAULT if we are unsure)
SchemeItem *tokenize() {
    char state[16] = "DEFAULT"; // boolean, number (integer or double), string, symbol, quote, comment, open, close, 
    char current_token[300];
    int index = 0;

    int charRead;
    SchemeItem *list = makeEmpty();
    
    charRead = fgetc(stdin);

    while (charRead != EOF) {
        //printf("%c\n", charRead);

        if (strcmp(state, "COMMENT") == 0) { // if we are in a comment
            if (charRead == '\n') {
                strcpy(state, "DEFAULT"); // end of comment
            }
        }

        else if (strcmp(state, "STRING") == 0) {
            current_token[index++] = charRead;
            if (charRead == '"') {  // closing quotations, string is over
                current_token[index] = '\0';

                // Add it to the linked list
                SchemeItem *new_node = makeEmpty();
                new_node->type = STR_TYPE;
                new_node->s = talloc(strlen(current_token) + 1);
                strcpy(new_node->s, current_token);

                list = cons(new_node, list);

                index = 0;
                strcpy(state, "DEFAULT");
            }
        } 

        else if (strcmp(state, "NUMBER") == 0) {
            if ((charRead >= '0' && charRead <= '9') || charRead == '.') {
                current_token[index++] = charRead;
                // here we should probably make sure that we can't add more than one '.'
            } else {
                current_token[index] = '\0';

                char *endptr;
                int num = atoi(current_token);
                double num_d = strtod(current_token, &endptr);
                if (strchr(current_token, '.') != NULL) {
                    SchemeItem *new_node = makeEmpty();
                    new_node->type = DOUBLE_TYPE;
                    new_node->d = num_d;

                    list = cons(new_node, list);
                } else {
                    SchemeItem *new_node = makeEmpty();
                    new_node->type = INT_TYPE;
                    new_node->i = num;

                    list = cons(new_node, list);
                }

                index = 0;
                strcpy(state, "DEFAULT");
                ungetc(charRead, stdin);
            }
        }

        else if (strcmp(state, "BOOL") == 0) {
            if (charRead == 't' || charRead == 'f') {
                current_token[index++] = charRead;
                current_token[index] = '\0';   

                SchemeItem *new_node = makeEmpty();
                new_node->type = BOOL_TYPE;
                new_node->s = talloc(strlen(current_token)+1);
                strcpy(new_node->s, current_token);    
                
                list = cons(new_node, list);

                index = 0;
                
                strcpy(state, "DEFAULT");
            } else {
                printf("Syntax error (readBoolean): boolean was not #t or #f\n");
                texit(1);
            }
        }

        else if (strcmp(state, "SYMBOL") == 0) {
            if (charRead == '(' || charRead == ')' || charRead == '"' || isspace(charRead) || charRead == ';') {
                current_token[index] = '\0';

                // Add it to the linked list
                SchemeItem *new_node = makeEmpty();
                new_node->type = SYMBOL_TYPE;
                new_node->s = talloc(strlen(current_token) + 1);
                strcpy(new_node->s, current_token);

                list = cons(new_node, list);

                index = 0;
                strcpy(state, "DEFAULT");
                ungetc(charRead, stdin); // adds same symbol back, so you pull the same thing again.
            } else {
                current_token[index++] = charRead;
            }
        } 
        
        else if (strcmp(state, "DEFAULT") == 0) {
            if (charRead == ';') {
                strcpy(state, "COMMENT");
            } else if (charRead == '"') {
                index = 0;
                current_token[index++] = charRead;
                strcpy(state, "STRING");
            } else if (charRead >= '0' && charRead <= '9') {
                index = 0;
                current_token[index++] = charRead;
                strcpy(state, "NUMBER");
            } else if (charRead == '(') {
                current_token[index++] = charRead;

                SchemeItem *new_node = makeEmpty();
                new_node->type = OPEN_TYPE;
                list = cons(new_node, list);

                index = 0;
                strcpy(state, "DEFAULT");
            } else if (charRead == ')') {
                current_token[index++] = charRead;

                SchemeItem *new_node = makeEmpty();
                new_node->type = CLOSE_TYPE;
                list = cons(new_node, list);

                index = 0;
                strcpy(state, "DEFAULT");
            } else if (charRead == '\'') {
                current_token[index++] = charRead;

                SchemeItem *new_node = makeEmpty();
                new_node->type = SINGLEQUOTE_TYPE;
                list = cons(new_node, list);

                index = 0;
                strcpy(state, "DEFAULT");
            } else if (charRead == '#') {
                index = 0;
                current_token[index++] = charRead;
                strcpy(state, "BOOL");
            } else if (charRead == '-' || charRead == '+') {
                int previous = charRead;

                charRead = fgetc(stdin);

                if (charRead >= '0' && charRead <= '9') {
                    ungetc(charRead, stdin);
                    charRead = previous;
                    
                    index = 0;
                    current_token[index++] = charRead;
                    strcpy(state, "NUMBER");
                } else {
                    ungetc(charRead, stdin);
                    charRead = previous;

                    index = 0;
                    current_token[index++] = charRead;

                    strcpy(state, "SYMBOL");
                }
            } else if (charRead == '@') {
                printf("Syntax error (readSymbol): symbol @ does not start with an allowed first character.\n");
            } else if (charRead == '{') {
                printf("Syntax error (readSymbol): symbol { does not start with an allowed first character.\n");
            } else if (isspace(charRead)) {
                // do nothing
            } else {
                // we have a symbol
                index = 0;

                current_token[index++] = charRead;
                strcpy(state, "SYMBOL");
            }
        }

        charRead = fgetc(stdin);
    }

    SchemeItem *revList = reverse(list);
    return revList;
}

// Recieves a linked list as input 
// Prints out each node in the linked list depending on its token type
// Will print the token and its type
// Won't modify the linked list.
void displayTokens(SchemeItem *list) {
    SchemeItem *current = list;

    while (current->type == CONS_TYPE) {
        SchemeItem *my_car = current->car;

        if (my_car->type == OPEN_TYPE) {
            printf("(:open\n");
        } else if (my_car->type == CLOSE_TYPE) {
            printf("):close\n");
        } else if (my_car->type == STR_TYPE) {
            printf("%s:string\n", my_car->s);
        } else if (my_car->type == INT_TYPE) {
            printf("%d:integer\n", my_car->i);
        } else if (my_car->type == SYMBOL_TYPE) {
            printf("%s:symbol\n", my_car->s);
        } else if (my_car->type == BOOL_TYPE) {
            printf("%s:boolean\n", my_car->s);
        } else if (my_car->type == DOUBLE_TYPE){
            printf("%f:double\n", my_car->d);
        } else if (my_car->type == SINGLEQUOTE_TYPE){
            printf("':quote\n");
        }
        current = current->cdr;
    }
}