#include "hvm.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

inline Stack *new_Stack() {
  Stack *stack = xmalloc(sizeof(Stack));
  stack->data = new_vec_with(STACK_DEFAULT_CAPACITY);
  stack->elem_count = 0;

  return stack;
}

inline void free_Stack(Stack *stack) {

  for (size_t i = 0; i < stack->elem_count; i++) {
    free_GeneralPointer(stack->data->data[i]);
  }

  free(stack);
}

inline void push_Stack(Stack *stack, GeneralPointer *val) {
  stack->elem_count++;
  vec_push(stack->data, val);
}

inline GeneralPointer *pop_Stack(Stack *stack) {
  if (stack->elem_count == 0) {
    fprintf(stderr, "<pop_Stack> Stack is empty!\n");
    exit(EXIT_FAILURE);
  }
  stack->elem_count--;
  return vec_pop(stack->data);
}

GeneralPointer *peek_Stack(Stack *stack) {
  if (stack->elem_count == 0) {
    fprintf(stderr, "<peek_Stack> Stack is empty!\n");
    exit(EXIT_FAILURE);
  }

  return stack->data->data[stack->elem_count - 1];
}

inline bool isempty_Stack(Stack *stack) { return stack->elem_count == 0; }

void print_Stack(Stack *stack, S_DATA_SHOW show) {
  printf("[");
  for (size_t i = 0; i < stack->elem_count; i++) {
    if (i > 0) {
      printf(", ");
    }
    printf("%s", show(stack->data->data[i]->ptr));
  }
  printf("]\n");
}
