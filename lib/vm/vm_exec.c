#include "hvm.h"
#include <stdlib.h>

static inline SexpObject *pop_SexpObject_from_stack(Stack *stack) {
  VMValue *v = pop_Stack(stack)->ptr;
  assert(v->ty == VValue);
  return v->val;
}

static inline VMFunction *pop_func_from_stack(Stack *stack) {
  VMValue *v = pop_Stack(stack)->ptr;
  assert(v->ty == VFunc);
  return v->func;
}

static double dmod(double x, double y) { return x - ((x / y) * y); }

static inline void push_Stack_VValue(Stack *stack, SexpObject *val) {
  push_Stack(stack, new_GeneralPointer(new_VMValueWithValue(val),
                                       (ELEM_DESTRUCTOR)&free_VMValue));
}

static inline void push_Stack_VFunc(Stack *stack, VMFunction *vmf) {
  push_Stack(stack, new_GeneralPointer(new_VMValueWithFunc(vmf),
                                       (ELEM_DESTRUCTOR)&free_VMValue));
}

static void dump_stack(Stack *stack) {
  Vector *v = stack->data;
  for (size_t i = 0; i < v->len; i++) {
    void *e = v->data[i]->ptr;
    printf("stack[%ld] %p : \n", i, e);
  }
}

static AVLTree *builtin_functions;
static bool vm_initialized;

void vm_exec_init(void) {
  if (vm_initialized) {
    return;
  }
  vm_initialized = true;

#ifdef __ENABLE_DIRECT_THREADED_CODE__
  printf("[INTERNAL VM INFO] Direct Threaded Code : Enabled\n");
#else
  printf("[INTERNAL VM INFO] Direct Threaded Code : Disabled\n");
#endif

  builtin_functions = new_AVLTree(&varcmp);

  avl_insert(builtin_functions, sdsnew("print"), (void *)(intptr_t)OpPrint);
  avl_insert(builtin_functions, sdsnew("println"), (void *)(intptr_t)OpPrintln);
}

static inline int get_builtin(sds name) {
  bool e = avl_exists(builtin_functions, name);
  if (!e) {
    return -1;
  } else {
    return (int)(intptr_t)avl_find(builtin_functions, name);
  }
}

#include "vm_exec.generated"
