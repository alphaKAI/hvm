#ifndef ENV_HEADER_INCLUDED
#define ENV_HEADER_INCLUDED

#define FUNC_CACHE_LEN 5

#include "hvm.h"

typedef struct Env {
  AVLTree *vars;
  struct Env *parent;
  bool copied;
  VMValue *cached_funcs[FUNC_CACHE_LEN];
} Env;

Env *new_Env(void);
VMValue *get_Env(Env *env, sds name);
void insert_Env(Env *env, sds name, VMValue *val);
Env *dup_Env(Env *env);
void free_Env(Env *env);

#endif