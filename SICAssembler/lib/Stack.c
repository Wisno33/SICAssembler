//  Stack.c

/*
 * A very simple stack implementation. Elements are of any type. The stack is array based.
 */

//STD C libraries.
#include <stdlib.h>
#include <stdio.h>

//Header for function prototypes.
#include "Stack.h"

//Creates a stack with a specified capacity. All space on the stack is set to NULL.
stack* stack_init(int capacity)
{
    stack* s = (stack*) malloc(sizeof(stack));
    
    s->capacity = capacity;
    s->size = 0;
    s->stack = malloc(sizeof(stack_element) * capacity);
    
    int i = 0;
    for(; i<s->capacity; i++)
    {
        s->stack[i] = NULL;
    }
    
    return s;
}

//Places an item on the stack. O(1)
void stack_push(stack* s, stack_element* item)
{
    //Checks if stack is full.
    if(s->size == s->capacity)
    {
        return;
    }
        
    s->stack[s->size] = item;
    
    s->size += 1;
}

//Removes an item from the stack and returns it. O(1)
void* stack_pop(stack* s)
{
    //Checks if stack is empty.
    if(s->size == 0)
    {
        return NULL;
    }
    s->size -= 1;
    
    return s->stack[s->size]->item;
}

//Deallocates all memory used by a stack structure.
void stack_destroy(stack* s)
{
    int i = 0;
    for(; i<s->capacity; i++)
    {
        free(s->stack[i]->item);
        free(s->stack[i]);
    }
    
    free(s->stack);
    free(s);
}

//Prints the stack info and the stack in a top down order.
void stack_print(stack* s)
{
    printf("Size: %d, Capacity: %d\n", s->size, s->capacity);
    
    int i = s->size - 1;
    for(; i >= 0; i--)
    {
        printf("Item at %p\n", s->stack[i]);
    }
}
