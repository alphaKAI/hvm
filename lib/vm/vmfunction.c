#include "hvm.h"

VMFunction *new_VMFunction(sds name, Vector *code, Vector *arg_names) {
  VMFunction *func = xmalloc(sizeof(VMFunction));
  func->name = name;
  func->code = code;
  func->arg_names = arg_names;
#ifdef __ENABLE_DIRECT_THREADED_CODE__
  func->ops_ptr = NULL;
#endif
  return func;
}

void free_VMFunction(VMFunction *vmf_ptr) {
  sdsfree(vmf_ptr->name);
  // TODO: Vector destructor
  free_vec(vmf_ptr->arg_names);
  free_vec(vmf_ptr->code);
  xfree(vmf_ptr);
}