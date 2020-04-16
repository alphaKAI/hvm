#include "hvm.h"
#include <stdlib.h>

inline Registers *new_Registers(void) {
  Registers *registers = xmalloc(sizeof(Registers));
  registers->pc = 0;
  return registers;
}

inline void free_Registers(Registers *registers) { free(registers); }