#ifndef __MINILISP_HEADER_INCLUDED__
#define __MINILISP_HEADER_INCLUDED__

#include "hvm.h"

// ----------------------------- Registers ----------------------------- //

#include "vm/registers.h"

// ----------------------------- VMFunction ----------------------------- //
#include "vm/opcode.h"

// ----------------------------- VMFunction ----------------------------- //

#include "vm/vmfunction.h"

// ----------------------------- VMValue ----------------------------- //

#include "vm/vmvalue.h"

// ----------------------------- Env ----------------------------- //

#include "vm/env.h"

// ----------------------------- Inline Functions  ----------------------- //
static inline int varcmp(void *lhs, void *rhs) {
  char *ls = (char *)lhs;
  char *rs = (char *)rhs;
  int ret = sdscmp(ls, rs);

  if (ret < 0) {
    return -1;
  }
  if (ret > 0) {
    return 1;
  }
  return 0;
}

// ----------------------------- Frame  ----------------------------- //

#include "vm/frame.h"

// ------------------------- VM-Init ------------------------- //

#include "vm/vm_init.h"

// ------------------------- VM-Exec ------------------------- //

#include "vm/vm_exec.h"

// ------------------------- VM-Util ------------------------- //

#include "vm/vm_util.h"

// ------------------------- VM-Binarize ------------------------- //

#include "vm/vm_binarize.h"

// ------------------------- VM-Deserialize ------------------------- //

#include "vm/vm_deserialize.h"

#endif
