#include <stdbool.h>
#include "schemeitem.h"

#ifndef _LINKEDLIST
#define _LINKEDLIST

// Create a new EMPTY_TYPE value node.
SchemeItem *makeEmpty();

// Create a new CONS_TYPE value node.
SchemeItem *cons(SchemeItem *newCar, SchemeItem *newCdr);

// Display the contents of the linked list to the screen in some kind of
// readable format
void display(SchemeItem *list);

// Return a new list that is the reverse of the one that is passed in. No stored
// data within the linked list should be duplicated; rather, a new linked list
// of CONS_TYPE nodes should be created, that point to items in the original
// list.
SchemeItem *reverse(SchemeItem *list);

// Utility to make it less typing to get car value. Use assertions to make sure
// that this is a legitimate operation.
SchemeItem *car(SchemeItem *list);

// Utility to make it less typing to get cdr value. Use assertions to make sure
// that this is a legitimate operation.
SchemeItem *cdr(SchemeItem *list);

// Utility to check if pointing to a EMPTY_TYPE value. Use assertions to make sure
// that this is a legitimate operation.
bool isEmpty(SchemeItem *value);

// Measure length of list. Use assertions to make sure that this is a legitimate
// operation.
int length(SchemeItem *value);


#endif
