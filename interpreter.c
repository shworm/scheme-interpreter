#include "schemeitem.h"
#include "linkedlist.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "parser.h"
#include "talloc.h"

// Included this decleration because evalIf was having trouble with calling eval, but eval has to call evalIf
SchemeItem *eval(SchemeItem *tree, Frame *frame);

// Funciton that creates and returns a frame
// Uses talloc for memory managment
// Takes parent frame as parameter
Frame *makeFrame(Frame *parent){
    Frame *new_frame = talloc(sizeof(Frame));
    new_frame->parent = parent;
    new_frame->bindings = makeEmpty();
    return new_frame;
}

// Looks up a variable in the provided frame
// If it doesn't find it in the provided frame, it will look in the parent frame until the parent frame is NULL
// Uses pointers to check each binding in the frame, and see if its equal to the provided variable_name
SchemeItem *findVariableValue(Frame *frame, char *variableName) {
    Frame *current = frame;
    while (current != NULL) {
        // look in this frame before moving on to parent
        SchemeItem *current_binding = current->bindings;
        while (current_binding->type == CONS_TYPE) {
            SchemeItem *pair = current_binding->car;

            SchemeItem *name = pair->car;

            if (name->type == SYMBOL_TYPE && strcmp(name->s, variableName) == 0) {
                return pair->cdr; // the value of the variable
            }

            current_binding = current_binding->cdr;
        }

        current = current->parent;
    }

    printf("Evaluation error: symbol '%s' wasn't found\n", variableName);

    texit(1);
    return NULL;
}

// Funciton to bind provided bindings to the provided frame
// Will check if bindings are a linked list, and if each binding is valid
//
// Binding format is a cons cell where the car is the variable name and the cdr is the variable value
//
// Will prevent duplicate bindings ex. (let ((x 3) (x 5)) x)
void bindVariables(Frame *frame, SchemeItem *bindings) {
    if (bindings->type != CONS_TYPE && bindings->type != EMPTY_TYPE){
        printf("Evaluation error: let bindings must be a list\n");
        texit(1);
    }

    SchemeItem *current_binding = bindings; // pointer to follow bindings linked list

    while (current_binding->type == CONS_TYPE) {
        // some error checks
        if (current_binding->car->type != CONS_TYPE) {
            // let bind is not a list
            printf("Evaluation error: null binding in let.\n");
            texit(1);
        }

        SchemeItem *variable_name = current_binding->car->car;
        // name must be a symbol
        if (variable_name->type != SYMBOL_TYPE) {
            printf("Evaluation error: let variable symbol is not a symbol.\n");
            texit(1);
        }

        SchemeItem *duplicate_check_binding = frame->bindings; // checks the frame that we are currently trying to bind to
        while (duplicate_check_binding->type == CONS_TYPE) {
            SchemeItem *pointer_to_variable_cell = duplicate_check_binding->car;

            SchemeItem *var_symbol = pointer_to_variable_cell->car;
            if (var_symbol->type == SYMBOL_TYPE && strcmp(var_symbol->s, variable_name->s) == 0) {
                printf("Evaluation error: duplicate binding for '%s'\n", variable_name->s);
                texit(1);
            }

            duplicate_check_binding = duplicate_check_binding->cdr;
        }

        SchemeItem *value_of_binding = eval(current_binding->car->cdr->car, frame->parent); // evaluate the expression
        SchemeItem *pair = cons(variable_name, value_of_binding); // cons cell that represents binding

        frame->bindings = cons(pair, frame->bindings); // add it on to the linked list of bindings for that frame we want

        current_binding = current_binding->cdr;
    }
}

// Helper function to bind variables in a letrec frame
//
// First initializes variables to a unassigned value object
//
// Then, reassignes by evaluating the expression. This ensures recursive definitions
//
// Will throw errors if attempt to call another defined variable ex. (x 3) (y x)
void bindVariablesLetRec(Frame *frame, SchemeItem *bindings) {
    SchemeItem *current_binding = bindings;

    while (current_binding->type == CONS_TYPE) {

        SchemeItem *variable_name = current_binding->car->car;

        SchemeItem *duplicate_check_binding = frame->bindings;
        while (duplicate_check_binding->type == CONS_TYPE) {
            SchemeItem *pointer_to_variable_cell = duplicate_check_binding->car;

            SchemeItem *var_symbol = pointer_to_variable_cell->car;
            if (var_symbol->type == SYMBOL_TYPE && strcmp(var_symbol->s, variable_name->s) == 0) {
                printf("Evaluation error: duplicate binding for '%s'\n", variable_name->s);
                texit(1);
            }

            duplicate_check_binding = duplicate_check_binding->cdr;
        }

        SchemeItem *unspecified_item = makeEmpty();
        unspecified_item->type = UNSPECIFIED_TYPE;
        SchemeItem *pair = cons(variable_name, unspecified_item);

        frame->bindings = cons(pair, frame->bindings);

        current_binding = current_binding->cdr;
    }


    current_binding = bindings; // reset it back to the front
    while (current_binding->type == CONS_TYPE) {
        SchemeItem *variable_name = current_binding->car->car;

        SchemeItem *expression = current_binding->car->cdr->car;
        
        if ((expression != variable_name) && (expression->type == SYMBOL_TYPE)) {
            printf("Evaluation Error\n");
            texit(1);
        }

        SchemeItem *value = eval(expression, frame);

        if (value->type == SYMBOL_TYPE || value->type == UNSPECIFIED_TYPE) {
            printf("Evaluation Error\n");
            texit(1);
        }

        SchemeItem *binding_to_check = frame->bindings;
        while(binding_to_check->type == CONS_TYPE) {

            SchemeItem *pair = binding_to_check->car;
            SchemeItem *pair_name = pair->car;


            if (strcmp(pair_name->s, variable_name->s) == 0) {
                pair->cdr = value;
            }
            binding_to_check = binding_to_check->cdr;
        }

        current_binding = current_binding->cdr;
    }
}

// Helper function to evaluate if function
// First, checks if the provided amount of args is valid
// 
// Then evaluates the test expression. If anything but #f, evaluate true_expression
// Otherwise, evaluate false_expression
SchemeItem *evalIf(SchemeItem *args, Frame *frame) {
    if (length(args) != 3) {
        printf("Evaluation error: args length isn't 3.\n");
        texit(1);
    }
    SchemeItem *test = args->car; // condition we have to meet
    SchemeItem *true_express  = args->cdr->car;
    SchemeItem *false_express  = args->cdr->cdr->car;

    SchemeItem *test_evaluted = eval(test, frame);

    if (test_evaluted->type == BOOL_TYPE && strcmp(test_evaluted->s, "#f") == 0) {
        return eval(false_express, frame);
    } else {
        return eval(true_express, frame);
    }
}

// Helper function to evaluate let statements
// First, checks to see if the body statement is valid
//
// Will then bind variables to a new frame, using bindVariables helper function
//
// Will use a pointer (current) to loop through and execute each body statement after binding
//
// Returns the final value/statement after body expressions are executed
SchemeItem *evalLet(SchemeItem *args, Frame *frame) {
    SchemeItem *bindings_list = args->car;
    SchemeItem *body_list = args->cdr;

    if (body_list->type == EMPTY_TYPE) {
        printf("Evaluation error: let body is empty.\n");
        texit(1);
    }

    Frame *new_frame = makeFrame(frame); // parent = frame

    bindVariables(new_frame, bindings_list);

    // evaluate the bodys
    SchemeItem *current = body_list;

    SchemeItem *last = NULL;

    while (current->type == CONS_TYPE) {
        last = eval(current->car, new_frame);

        current = current->cdr;
    }

    return last;
}

// Helper function to evaluate let rec statments
//
// Creates a new frame
//
// Then, uses bindVariablesLetRec to specifically bind variables
//
// Returns the last body
SchemeItem *evalLetRec(SchemeItem *args, Frame *frame) {
    // create a new frame with parent env
    SchemeItem *bindings_list = args->car;
    SchemeItem *body_list = args->cdr;

    Frame *new_frame = makeFrame(frame);

    bindVariablesLetRec(new_frame, bindings_list);

    // evaluate the bodys
    SchemeItem *current = body_list;

    SchemeItem *last = NULL;

    while (current->type == CONS_TYPE) {
        last = eval(current->car, new_frame);

        current = current->cdr;
    }

    return last;
}

// Helper function to evaluate set
//
// Will check for two arguments
//
// Expression is evaluated in the provided frame (current frame)
//
// Will check to see if variable name is in current frame
//
// If not, will check through parents
//
// Reassignes that variable names value to the evaluated expression
//
// If not found once reaches parent frame, throws an error
SchemeItem *evalSet(SchemeItem *args, Frame *frame) {
    if (length(args) != 2) {
        printf("Evaluation error: set! takes 2 arguments\n");
        texit(1);
    }

    SchemeItem *name  = args->car;
    SchemeItem *value = eval(args->cdr->car, frame);

    Frame *current = frame;
    while (current != NULL) {
        SchemeItem *binding = current->bindings;

        while (binding->type == CONS_TYPE) {
            SchemeItem *pair = binding->car;
            SchemeItem *var_symbol = pair->car;

            if ((var_symbol->type == SYMBOL_TYPE) && (strcmp(var_symbol->s, name->s) == 0)) {
                pair->cdr = value;
                SchemeItem *void_thing = makeEmpty();
                void_thing->type = VOID_TYPE;
                return void_thing;
            }
            binding = binding->cdr;
        }
        current = current->parent;
    }
    printf("Evaluation error\n");
    texit(1);

    return NULL;
}


// Helper function to evaluate quote statements
//
// Does not wrap the quoted expression in a closure
//
// Instead, just returns the already structed CONS linked list in the car of the args
//
// If more or less than 1 argument is passed, will produce an error
SchemeItem *evalQuote (SchemeItem *args, Frame *frame) {
    if (length(args) != 1) {
        printf("Evaluation error\n");
        texit(1);
    }
    return args->car;
}

// Helper function to evaluate define statements
//
// Checks that name is a symbol and that there are no duplicate bindings for frame we are in
//
// Evaluates provided expr and assigns value to variable / binding name
//
// Returns an object that is void_type, per assignment
SchemeItem *evalDefine (SchemeItem *args, Frame *frame) {
    if (length(args) != 2) {
        printf("Evaluation error\n");
        texit(1);
    }

    SchemeItem *name = args->car;
    if (name->type != SYMBOL_TYPE) {
        printf("Evaluation error: define name must be a symbol\n");
        texit(1);
    }

    // check if we are duplicate binding in the provided frame (frame we are defining in)
    SchemeItem *duplicate_check_binding = frame->bindings;
    while (duplicate_check_binding->type == CONS_TYPE) {
        SchemeItem *pointer_to_variable_cell = duplicate_check_binding->car;

        SchemeItem *var_symbol = pointer_to_variable_cell->car;
        if (var_symbol->type == SYMBOL_TYPE && strcmp(var_symbol->s, name->s) == 0) {
            printf("Evaluation error: duplicate binding for '%s'\n", name->s);
            texit(1);
        }

        duplicate_check_binding = duplicate_check_binding->cdr;
    }

    SchemeItem *expr = args->cdr->car;
    SchemeItem *value = eval(expr, frame); // evaluate the expression
    SchemeItem *pair = cons(name, value);
    frame->bindings = cons(pair, frame->bindings); // add on to the bindings in the current frame

    SchemeItem *void_thing = makeEmpty();
    void_thing->type = VOID_TYPE;
    return void_thing;
}


// Helper function to evaluate lambda expressions
//
// Creates a closure that takes parameters and has the code for the function
//
// Checks to make sure the parameter list is valid (symbols, and no duplicates)
//
// Stores paramNames and body on the closure, then returns it
SchemeItem *evalLambda (SchemeItem *args, Frame *frame) {
    if (length(args) < 2) {
        printf("Evaluation error\n");
        texit(1);
    }
    // make closure
    SchemeItem *closure = makeEmpty();
    closure->type = CLOSURE_TYPE;

    if (args->car->type == CONS_TYPE) {
        SchemeItem *current = args->car;
        while (current->type == CONS_TYPE) {
            if (current->car->type != SYMBOL_TYPE) {
                printf("Evaluation error\n");
                texit(1);
            }

            SchemeItem *rest = current->cdr;

            while (rest->type == CONS_TYPE) {
                if (strcmp(current->car->s, rest->car->s) == 0) {
                    printf("Evaluation error: duplicate identifier\n");
                    texit(1);
                }
                rest = rest->cdr;
            }

            current = current->cdr;
        }
    }

    closure->paramNames = args->car;
    
    closure->frame = frame;

    closure->functionCode = args->cdr;

    return closure;
}

// Binds the actual parameter values for a frame that is created on function call to the parameter names
//
// Will bind differently depending on lambda format
void bindParameters(Frame *frame, SchemeItem *paramNames, SchemeItem *args) {
    // (lambda (a1 a2 ... an) body1 body2 ... bodym)
    if (paramNames->type == CONS_TYPE || paramNames->type == EMPTY_TYPE) {
        while (paramNames->type == CONS_TYPE && args->type == CONS_TYPE) {
            SchemeItem *pair = cons(paramNames->car, args->car);
            frame->bindings = cons(pair, frame->bindings);
            paramNames  = paramNames->cdr;
            args  = args->cdr;
        }
        return;
    }
    // (lambda args body1 body2 ... bodym)
    if (paramNames->type == SYMBOL_TYPE) {
        SchemeItem *pair = cons(paramNames, args);
        frame->bindings = cons(pair, frame->bindings);
        return;
    }
}

// Applies a function to evalauted arguments
//
// Creates a frame for the function call with the closure's parent as the parent
//
// Evaluates body(s) in that frame
//
// Returns the final value of the body list
SchemeItem *apply(SchemeItem *function, SchemeItem *args) {
    if (function->type == CLOSURE_TYPE) {
        // make a frame for the function call, and bind parameters
        // parent is the same as where the function was defined
        Frame *frame = makeFrame(function->frame);
        bindParameters(frame, function->paramNames, args); 

        // evaluate body, then return 
        SchemeItem *body = function->functionCode;

        SchemeItem *last = NULL;
        while (body->type == CONS_TYPE) {
            last = eval(body->car, frame);
            body = body->cdr;
        }
        return last;
    } else if (function->type == PRIMITIVE_TYPE) {
        return function->pf(args);
    } else {
        return NULL;
    }
}

/*
 *****************************************************************************
 *                                                                           *
 *                              Primitives                                   *
 *                                                                           *
 *****************************************************************************
 */

// Primitive functin for the less than function in scheme '<'
//
// Will ensure there are two arguments
// 
// Checks to make sure type INT or DOUBLE, accessing proper scheme item value for the type
//
// Returns a bool scheme item of the result
SchemeItem *primitiveLessThan(SchemeItem *args) {
    if (length(args) != 2) {
        printf("Evaluation error: too many args\n");
        texit(1);
    }

    SchemeItem *bool_item = makeEmpty();
    bool_item->type = BOOL_TYPE;
    bool_item->s = talloc(sizeof(2));

    if ((args->car->type == INT_TYPE && args->cdr->car->type == INT_TYPE) || (args->car->type == DOUBLE_TYPE && args->cdr->car->type == DOUBLE_TYPE)) {
        if (args->car->type == INT_TYPE) {
            if (args->car->i < args->cdr->car->i) {
                strcpy(bool_item->s, "#t");
            } else {
                strcpy(bool_item->s, "#f");
            }
        } else if (args->car->type == DOUBLE_TYPE) {
            if (args->car->d < args->cdr->car->d) {
                strcpy(bool_item->s, "#t");
            } else {
                strcpy(bool_item->s, "#f");
            }
        } else {
            printf("Evaluation error\n");
            texit(1);
        }
    } else {
        printf("Evaluation error\n");
        texit(1);
    }

    return bool_item;

}

// Primitive function equal in scheme "equal?"
//
// Ensures it has two arguments
//
// Currently only able to compare of type INT, DOUBLE, or STR. (not cons)
//
// Returns a scheme item bool with the result
SchemeItem *primitiveEqual(SchemeItem *args) {
    if (length(args) != 2) {
        printf("Evaluation error: too many args\n");
        texit(1);
    }

    SchemeItem *bool_item = makeEmpty();
    bool_item->type = BOOL_TYPE;
    bool_item->s = talloc(sizeof(2));

    if ((args->car->type == INT_TYPE && args->cdr->car->type == INT_TYPE) || (args->car->type == DOUBLE_TYPE && args->cdr->car->type == DOUBLE_TYPE)) {
        if (args->car->type == INT_TYPE) {
            if (args->car->i == args->cdr->car->i) {
                strcpy(bool_item->s, "#t");
            } else {
                strcpy(bool_item->s, "#f");
            }
        } else if (args->car->type == DOUBLE_TYPE) {
            if (args->car->d == args->cdr->car->d) {
                strcpy(bool_item->s, "#t");
            } else {
                strcpy(bool_item->s, "#f");
            }
        }
    } else if (args->car->type == STR_TYPE && args->cdr->car->type == STR_TYPE) {
        if (strcmp(args->car->s, args->cdr->car->s) == 0) {
            strcpy(bool_item->s, "#t");
        } else {
            strcpy(bool_item->s, "#f");
        }   
    } else if (args->car->type == CONS_TYPE && args->cdr->car->type == CONS_TYPE) {
        // do something for cons
    } else {
        strcpy(bool_item->s, "#f");
    }

    return bool_item;
}


// Primitive implementation of function car
//
// Will check to see only one argument provided, and that the type is a list
SchemeItem *primitiveCar(SchemeItem *args) {
    if (length(args) != 1 || args->car->type != CONS_TYPE) {
        printf("Evaluation error\n");
        texit(1);
    }
    return args->car->car;
}

// Primitive implementation of function cdr
//
// Will check to see only one argument provided, and that the type is a list
SchemeItem *primitiveCdr(SchemeItem *args) {
    if (length(args) != 1 || args->type != CONS_TYPE) {
        printf("Evaluation error\n");
        texit(1);
    }
    return args->car->cdr;
}

// Primitive implementation of function null?
//
// Will check to see only one argument provided
//
// Creates / returns bool scheme item with whether or not the list is of EMPTY_TYPE
SchemeItem *primitiveNull(SchemeItem *args) {
    if (length(args) != 1) {
        printf("Evaluation error\n");
        texit(1);
    }
    SchemeItem *bool_item = makeEmpty();
    bool_item->type = BOOL_TYPE;
    bool_item->s = talloc(sizeof(2));

    if (args->car->type == EMPTY_TYPE) {
        strcpy(bool_item->s, "#t");
    } else {
        strcpy(bool_item->s, "#f");
    }

    return bool_item;
}

// Primitive implementation of function +
//
// Will add up to the value total as it goes through the list
//
// Properly handles both double and int types, casting to int at the end if int type (double by default)
//
// Will throw errors if an element of the list is not a double or int type
//
// Will return 0 if no args provided
SchemeItem *primitiveAdd(SchemeItem *args) {
    if (length(args) == 0) {
        SchemeItem *total_item = makeEmpty();
        total_item->type = INT_TYPE; // change this
        total_item->i = 0;
        return total_item;
    }

    double total = 0;
    bool is_int = true;

    SchemeItem *current = args;
    while (current->type == CONS_TYPE) {
        SchemeItem *number_item = current->car;
        if (number_item->type != DOUBLE_TYPE && number_item->type != INT_TYPE) {
            printf("Evaluation error\n");
            texit(1);
        }

        if (number_item->type == INT_TYPE) {
            total = total + number_item->i;
        }
        if (number_item->type == DOUBLE_TYPE) {
            is_int = false;
            total = total + number_item->d;
        }

        current = current->cdr;
    }

    SchemeItem *total_item = makeEmpty();
    if (is_int) {
        total_item->type = INT_TYPE;
        total_item->i = (int)total;
    } else {
        total_item->type = DOUBLE_TYPE;
        total_item->d = total;
    }

    return total_item;
}

// Primitive implementation of function cons
//
// Will check to see that exactly two arguments provided
SchemeItem *primitiveCons(SchemeItem *args) {
    if (length(args) != 2) {
        printf("Evaluation error\n");
        texit(1);
    }
    SchemeItem *cons_item = makeEmpty();
    cons_item->type = CONS_TYPE;
    cons_item = cons(args->car, args->cdr->car);

    return cons_item;
}

// Primitive implementation of function append
//
// Will check to see exactly two arguments provided
//
// Will make a copy of the first provided list, but not the second
SchemeItem *primitiveAppend(SchemeItem *args) {
    if (length(args) != 2) {
        printf("Evaluation error\n");
        texit(1);
    }

    if (args->car->type != CONS_TYPE && args->car->type != EMPTY_TYPE) {
        printf("Evaluation error\n");
        texit(1);
    }
    SchemeItem *total_first = makeEmpty();

    SchemeItem *first  = args->car;
    SchemeItem *second = args->cdr->car;

    if (first->type == EMPTY_TYPE) {
        return second;
    }

    SchemeItem *result_head = NULL;
    SchemeItem *result_tail = NULL;

     while (first->type == CONS_TYPE) {
        SchemeItem *new_node = cons(first->car, makeEmpty());

        if (result_head == NULL) {
            result_head = new_node;
        } else {
            result_tail->cdr = new_node;
        }
        result_tail = new_node;

        first = first->cdr;
    }

    result_tail->cdr = second;

    return result_head;
}

// Binds provided primitive function name to function in C
//
// Used to add primitive functions to the home frame (top level) bindings
void bind(char *name, SchemeItem *(*function)(SchemeItem *), Frame *frame) {
    SchemeItem *name_object = makeEmpty();
    name_object->type = SYMBOL_TYPE;
    name_object->s = name;

    SchemeItem *pointer = makeEmpty();
    pointer->type = PRIMITIVE_TYPE;
    pointer->pf = function;


    SchemeItem *pair = cons(name_object, pointer);

    frame->bindings = cons(pair, frame->bindings);
}



// Evaluates a SchemeItem in the given frame
//
// Will just return atoms
//
// Will lookup symbols using the findVariableValue helper function
//
// Currently only handles if and let statement
SchemeItem *eval(SchemeItem *tree, Frame *frame) {
    switch (tree->type)  {
        case INT_TYPE: {
            return tree;
        }
        case BOOL_TYPE: {
            return tree;
        }
        case DOUBLE_TYPE: {
            return tree;
        }
        case STR_TYPE: {
            return tree;
        }
        case SYMBOL_TYPE: {
            // the value of our symbol is dependent on the frame we are in
            // thus, use a helper function in orde to check based on frame
            return findVariableValue(frame, tree->s);
        }
        case CONS_TYPE: {
            SchemeItem *first = car(tree);
            SchemeItem *args = cdr(tree);
            if (first->type == SYMBOL_TYPE) {
                if (strcmp(first->s,"if") == 0) {
                    SchemeItem *result = evalIf(args, frame);
                    return result;
                } else if (strcmp(first->s, "let") == 0) {
                    SchemeItem *result = evalLet(args, frame);
                    return result;
                } else if (strcmp(first->s, "quote") == 0) {
                    SchemeItem *result = evalQuote(args, frame);
                    return result;
                } else if (strcmp(first->s, "define") == 0) {
                    SchemeItem *result = evalDefine(args, frame);
                    return result;
                } else if (strcmp(first->s, "lambda") == 0) {
                    SchemeItem *result = evalLambda(args, frame);
                    return result;
                } else if (strcmp(first->s, "letrec") == 0) {
                    SchemeItem *result = evalLetRec(args, frame);
                    return result;
                } else if (strcmp(first->s, "set!") == 0) {
                    SchemeItem *result = evalSet(args, frame);
                    return result;
                } else {
                    // user-defined operator: evaluate operator and args, then apply
                    SchemeItem *evaluated_operator = eval(first, frame);

                    SchemeItem *evaluated_args = makeEmpty();
                    SchemeItem *current = args;
                    while (current->type == CONS_TYPE) {
                        SchemeItem *evaluated_argument = eval(current->car, frame);
                        evaluated_args = cons(evaluated_argument, evaluated_args);
                        current = current->cdr;
                    }
                    evaluated_args = reverse(evaluated_args);
                    return apply(evaluated_operator, evaluated_args);
                }
            }
            // we have a cons type, will two sets of parenthesis. ie, car is not a symbol, it is another list
            // we have to include this again for double parenthesis ((lambda () ...))
            // not sure if this is a good practice, should maybe just make function in the future
            SchemeItem *evaluated_operator = eval(first, frame);

            SchemeItem *evaluated_args = makeEmpty();
            SchemeItem *current = args;
            while (current->type == CONS_TYPE) {
                SchemeItem *evaluated_argument = eval(current->car, frame);
                evaluated_args = cons(evaluated_argument, evaluated_args);
                current = current->cdr;
            }
            evaluated_args = reverse(evaluated_args);
            return apply(evaluated_operator, evaluated_args);
        }
        case EMPTY_TYPE: {
            printf("Evaluation error: cannot evaluate empty list\n");
            texit(1);
        }
        default: {
            printf("Evaluation error: SchemeItem doesn't have a type\n");
            texit(1);
        }
    }
    // to compile
    return NULL;
}

// Main function that is called to interpret provided parse tree
//
// First creates a home frame with a null parent frame
//
// Then evaluates each s-expression, before printing it using the parser.c printItem funciton.
//
// Finally, exits the program to clear memory using texit
void interpret(SchemeItem *tree) {
    Frame *home_frame = makeFrame(NULL);

    // Then, bind primitive functions
    bind("car", primitiveCar, home_frame);
    bind("cdr", primitiveCdr, home_frame);
    bind("+", primitiveAdd, home_frame);
    bind("null?", primitiveNull, home_frame);
    bind("cons", primitiveCons, home_frame);
    bind("append", primitiveAppend, home_frame);
    bind("equal?", primitiveEqual, home_frame);
    bind("<", primitiveLessThan, home_frame);

    SchemeItem *line_reader = tree;
    while (line_reader->type == CONS_TYPE) {
        SchemeItem *evaluated = eval(line_reader->car, home_frame);
        if (evaluated->type != VOID_TYPE) {
            printItem(evaluated);
            printf("\n");
        }

        line_reader = line_reader->cdr;
    }

    texit(0);
}