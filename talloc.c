#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdlib.h>
#include "schemeitem.h"
SchemeItem *active_list = NULL;

// Allocates memory of desired size using malloc and returns its ponter
// Additionally, creates a cell that is added to a global linked list which tracks memory
void *talloc(size_t size) {
    SchemeItem *pointer = malloc(size);

    SchemeItem *node = malloc(sizeof(SchemeItem));
    node->type = PTR_TYPE;
    node->ptr = pointer;
    node->cdr = active_list;

    active_list = node;

    // just return the new memory pointer
    return pointer;
}

// Frees the memory of the pointers in the global linked list
// and the memory locations they are pointing to
void tfree() {
    while (active_list != NULL) {
        SchemeItem *next = active_list->cdr;

        SchemeItem *current = active_list->ptr;

        free(current);

        free(active_list);
        active_list = next;
    }
}


// Calls tfree function before terminating the program 
void texit(int status) {
    tfree();
    exit(status);
}
