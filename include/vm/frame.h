#ifndef FRAME_HEADER_INCLUDED
#define FRAME_HEADER_INCLUDED

#include "hvm.h"

typedef struct Frame {
  Registers *registers;
  Env *env;
  Vector *args;
  struct Frame *parent;
  Vector *lvars;
  Vector *v_ins;
} Frame;

Frame *new_Frame(void);

void free_Frame(Frame *frame);

#endif