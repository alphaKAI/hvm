#ifndef VM_EXEC_HEADER_INCLUDED
#define VM_EXEC_HEADER_INCLUDED

#include "hvm.h"

void vm_exec_init(void);
SexpObject *vm_exec(Vector *);

#endif