#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "schemeitem.h"
#include "talloc.h"
#include <assert.h>
#include <string.h>

// Creates a scheme item with the type EMPTY_TPE
SchemeItem *makeEmpty() {
    SchemeItem *newItem = talloc(sizeof(SchemeItem));
    newItem->type = EMPTY_TYPE;
    return newItem;
};

// Create a scheme item with the type CONS_TYPE and the provided car and cdr values of that new node.
SchemeItem *cons(SchemeItem *newCar, SchemeItem *newCdr) {
    SchemeItem *newItem = makeEmpty();
    newItem->type = CONS_TYPE;

    newItem->car = newCar;
    newItem->cdr = newCdr;

    return newItem;
};

// Print out the contents of the provided linked list (list).
//
// ex. [1.000, 2, hi, 7]
void display(SchemeItem *list) {
    printf("[");

    SchemeItem *current = list;

    while (current != NULL && current->type == CONS_TYPE) {
        SchemeItem *the_car = current->car;
        switch (the_car->type) {
            case INT_TYPE:
                printf("%i", the_car->i);
                break;
            case DOUBLE_TYPE:
                printf("%f", the_car->d);
                break;
            case STR_TYPE:
                printf("%s", the_car->s);
                break;
            case CONS_TYPE:
                display(the_car);
                break;
            case EMPTY_TYPE:
                printf("'()");
                break;
            case PTR_TYPE:
                break;
            case OPEN_TYPE:
                printf("(");
            case CLOSE_TYPE:
                printf(")");
            case SYMBOL_TYPE:
                printf("%s", the_car->s);
            case BOOL_TYPE:
                break;
            case SINGLEQUOTE_TYPE:
                break;
            case DOT_TYPE:
                break;
            case OPENBRACKET_TYPE:
                break;
            case CLOSEBRACKET_TYPE:
                break;
            case VOID_TYPE:
                break;
            case CLOSURE_TYPE:
                break;
            case PRIMITIVE_TYPE:
                break;
            case UNSPECIFIED_TYPE:
                break;
        }

        if (current->cdr->type != EMPTY_TYPE){
            printf(", "); // add a comma between them
        }

        current = current->cdr;
    }

    printf("]\n");
};

// Copies elements to a new list while reversing their pointer order 
// thus reversing the order of the given list, without copying any data
SchemeItem *reverse(SchemeItem *list) {
    SchemeItem *reversed = makeEmpty();

    SchemeItem *current = list;

    while (current != NULL && current->type == CONS_TYPE) {
        SchemeItem *copied = current->car;

        reversed = cons(copied, reversed); //add it on to the overall
    
        current = current->cdr;
    }

    return reversed;
};

// Returns a pointer to the car value of a list. First checks to make sure that list is a valid CONS cell.
SchemeItem *car(SchemeItem *list) {
    assert(list != NULL && list->type == CONS_TYPE);
    return list->car;
};

// Returns a pointer to the cdr value of a list. First checks to make sure that list is a valid CONS cell.
SchemeItem *cdr(SchemeItem *list) {
    assert(list != NULL && list->type == CONS_TYPE);
    return list->cdr;
};

// Returns a bool whether or not a provided SchemeItem (item) is of type EMPTY_TYPE
bool isEmpty(SchemeItem *item) {
    assert(item != NULL);

    if (item->type == EMPTY_TYPE) {
        return true;
    } else { 
        return false;
    }
};

// Returns an int (length) which is the length of a provided list (item)
// Does so by creating a pointer (current) as to not reassign item.
//
// ex. length([1, 2, 3]) -> 3
int length(SchemeItem *item) {
    assert(item != NULL);

    int length = 0;
    SchemeItem *current = item;

    while (current != NULL && current->type != EMPTY_TYPE && current->type == CONS_TYPE) {
        length++;
        current = current->cdr;
    }

    return length;
};