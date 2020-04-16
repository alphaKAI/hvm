#ifndef __STACK_HEADER_INCLUDED__
#define __STACK_HEADER_INCLUDED__

#include "hvm.h"
#include <stdbool.h>
#include <stddef.h>

#define STACK_DEFAULT_CAPACITY 128

typedef struct {
  Vector *data;
  size_t elem_count;
} Stack;

typedef void (*S_DATA_FREE)(void);
typedef sds (*S_DATA_SHOW)(void *);

Stack *new_Stack(void);
void free_Stack(Stack *s_ptr);
void push_Stack(Stack *stack, GeneralPointer *val);
GeneralPointer *pop_Stack(Stack *stack);
GeneralPointer *peek_Stack(Stack *stack);
bool isempty_Stack(Stack *stack);
void print_Stack(Stack *stack, S_DATA_SHOW show);

#define POP_STACK_WITH_FREE(stack, lhs)                                        \
  do {                                                                         \
    GeneralPointer *__pswf = pop_Stack(stack);                                 \
    lhs = __pswf->ptr;                                                         \
    free(__pswf);                                                              \
  } while (0)

#endif
