#ifndef VMFUNCTION_HEADER_INCLUDED
#define VMFUNCTION_HEADER_INCLUDED

#include "hvm.h"

typedef struct {
  sds name;
  Vector *code;
  Vector *arg_names;
#ifdef __ENABLE_DIRECT_THREADED_CODE__
  void **ops_ptr;
#endif
} VMFunction;

VMFunction *new_VMFunction(sds name, Vector *code, Vector *arg_names);
void free_VMFunction(VMFunction *vmf_ptr);

#endif