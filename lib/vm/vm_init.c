#include "hvm.h"

void vm_init(void) {
  vm_exec_init();
  vm_compiler_init();
}