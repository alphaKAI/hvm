#include "hvm.h"
#include <stdlib.h>
#include <string.h>

Env *new_Env(void) {
  Env *env = xmalloc(sizeof(Env));
  env->vars = new_AVLTree(&varcmp);
  env->parent = NULL;
  env->copied = false;
  for (size_t i = 0; i < FUNC_CACHE_LEN; i++) {
    env->cached_funcs[i] = NULL;
  }
  return env;
}

static inline Vector *keys_Env(Env *env) {
  return avl_keys(env->vars, (ELEM_FREE)&sdsfree);
}

static inline VMValue *check_func_cache(Env *env, sds name) {
  VMValue *ret = NULL;

  for (size_t i = 0; i < FUNC_CACHE_LEN; i++) {
    VMValue *cached = env->cached_funcs[i];
    if (cached != NULL && cached->ty == VFunc &&
        strcmp(cached->func->name, name) == 0) {
      return cached;
    }
  }

  return ret;
}

static inline VMValue *get_Env_impl(Env *env, sds name) {
  if (env->parent == NULL) {
    return avl_find(env->vars, name);
  } else {
    if (env->copied == false) {
      for (Env *e = env->parent; e != NULL; e = e->parent) {
        VMValue *r = avl_find(e->vars, name);
        if (r) {
          return r;
        }
      }
      return NULL;
    } else {
      return avl_find(env->vars, name);
    }
  }
}

inline VMValue *get_Env(Env *env, sds name) {
  VMValue *ret = check_func_cache(env, name);
  if (ret == NULL) {
    return get_Env_impl(env, name);
  } else {
    return ret;
  }
}

static inline void insert_Env_impl(Env *env, sds name, VMValue *val) {
  if (env->parent != NULL && env->copied == false) {
    Vector *keys = keys_Env(env->parent);
    for (size_t i = 0; i < keys->len; i++) {
      sds key = keys->data[i]->ptr;
      avl_insert(env->vars, sdsdup(key),
                 dup_VMValue(get_Env(env->parent, key)));
    }
    env->copied = true;
    free_vec(keys);
  }

  avl_insert(env->vars, name, val);
}

static inline void update_func_cache(Env *env, sds name, VMValue *val) {
  size_t i = 0;
  for (; i < FUNC_CACHE_LEN && env->cached_funcs[i] != NULL; i++) {
    VMValue *cached = env->cached_funcs[i];
    if (cached != NULL && cached->ty == VFunc &&
        strcmp(cached->func->name, name) == 0) {
      env->cached_funcs[i] = val;
      return;
    }
  }

  env->cached_funcs[i == FUNC_CACHE_LEN ? 0 : i] = val;
}

inline void insert_Env(Env *env, sds name, VMValue *val) {
  if (val->ty == VFunc) {
    update_func_cache(env, name, val);
  }
  return insert_Env_impl(env, name, val);
}

Env *dup_Env(Env *env) {
  Env *new_env = new_Env();
  new_env->parent = env;

  for (size_t i = 0; i < FUNC_CACHE_LEN; i++) {
    new_env->cached_funcs[i] = env->cached_funcs[i];
  }

  return new_env;
}

// TODO: impl free AVL
void free_Env(Env *env) {
  free_AVLTree(env->vars, (ELEM_FREE)&sdsfree, (ELEM_FREE)&free_VMValue);
  free(env);
}