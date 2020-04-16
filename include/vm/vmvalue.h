#ifndef VMVALUE_HEADER_INCLUDED
#define VMVALUE_HEADER_INCLUDED

#include "hvm.h"

enum { VValue, VFunc };

typedef struct {
  int ty;
  union {
    SexpObject *val;
    VMFunction *func;
  };
} VMValue;

VMValue *new_VMValue(int ty, void *e);
void free_VMValue(VMValue *v_ptr);
VMValue *dup_VMValue(VMValue *value);
VMValue *new_VMValueWithValue(SexpObject *obj);
VMValue *new_VMValueWithFunc(VMFunction *vmf);

VMValue *new_VMValueWithValue(SexpObject *obj);
VMValue *new_VMValueWithFunc(VMFunction *vmf);
sds show_VMValue(VMValue *val);

int cmp_VMValue(VMValue *v1, VMValue *v2);

SexpObject *get_SexpObject_VMValue(VMValue *vmv);
VMFunction *get_func_VMValue(VMValue *vmv);

#endif