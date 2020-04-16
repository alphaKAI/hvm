#ifndef COMPILER_HEADER_INCLUDED
#define COMPILER_HEADER_INCLUDED

#include "hvm.h"

Vector *vm_compile(Vector *parsed);

void vm_compiler_init(void);
#endif