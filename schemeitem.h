#ifndef _SCHEMEITEM
#define _SCHEMEITEM

typedef enum {
   INT_TYPE, DOUBLE_TYPE, STR_TYPE, CONS_TYPE, EMPTY_TYPE, PTR_TYPE,
   OPEN_TYPE, CLOSE_TYPE, BOOL_TYPE, SYMBOL_TYPE, SINGLEQUOTE_TYPE,
   VOID_TYPE, CLOSURE_TYPE, PRIMITIVE_TYPE, UNSPECIFIED_TYPE,
    
   // Types below are only for bonus work
   DOT_TYPE, OPENBRACKET_TYPE, CLOSEBRACKET_TYPE
} itemType;

typedef struct SchemeItem {
    itemType type;
    union {
        int i;
        double d;
        char *s;
        struct {
            struct SchemeItem *car;
            struct SchemeItem *cdr;
        }; // For CONS_TYPE
        struct {
            struct SchemeItem *paramNames;
            struct SchemeItem *functionCode;
            struct Frame *frame;
        }; // For CLOSURE_TYPE
        void *ptr;
        // A primitive style function; just a pointer to it, with the right
        // signature (pf = primitive function)
        struct SchemeItem *(*pf)(struct SchemeItem *);
    };
} SchemeItem;

// A frame is a linked list of bindings, and a pointer to another frame.  A
// binding is a variable name (represented as a string), and a pointer to the
// Object it is bound to. Specifically how you implement the list of bindings
// is up to you.
typedef struct Frame {
    SchemeItem *bindings;
    struct Frame *parent;
} Frame;

#endif
