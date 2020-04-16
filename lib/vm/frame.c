#include "hvm.h"
#include <stdlib.h>

Frame *new_Frame(void) {
  Frame *frame = xmalloc(sizeof(Frame));
  frame->registers = new_Registers();
  frame->env = new_Env();
  frame->args = new_vec();
  frame->parent = NULL;
  frame->lvars = NULL;
  frame->v_ins = NULL;
  return frame;
}

void free_Frame(Frame *frame) {
  free_Registers(frame->registers);
  free_Env(frame->env);
  // TODO: Free Vector
  /* Don't touch memories which is shallow copied from caller
  free_vec(frame->args);
  free_vec(frame->v_ins);
  */
  free(frame);
}