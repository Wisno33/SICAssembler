//  Stack.h

/*
 * A very simple stack implementation. Elements are of any type. The stack is array based.
 */

#ifndef Stack_h
#define Stack_h

//Structures.

//A element on the stack.
typedef struct stack_element
{
    void* item;
} stack_element;

//The stack data structure.
typedef struct stack
{
    int capacity;
    int size;
    stack_element** stack;
} stack;

//Public functions.

//Creates a stack with a specified capacity. All space on the stack is set to NULL.
stack* stack_init(int capacity);

//Places an item on the stack. O(1)
void stack_push(stack* s, stack_element* item);

//Removes an item from the stack and returns it. O(1)
void* stack_pop(stack* s);

//Deallocates all memory used by a stack structure.
void stack_destroy(stack* s);

//Prints the stack info and the stack in a top down order.
void stack_print(stack* s);

#endif /* Stack_h */
