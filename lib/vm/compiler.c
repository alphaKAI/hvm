#include "hvm.h"
#include <stdlib.h>
#include <string.h>

static sds ADD_STR;
static sds SUB_STR;
static sds MUL_STR;
static sds DIV_STR;
static sds MOD_STR;
static sds EQ_STR;
static sds NEQ_STR;
static sds LT_STR;
static sds LEQ_STR;
static sds GT_STR;
static sds GEQ_STR;
static sds DEF_VAR_STR;
static sds DEF_FUN_STR;
static sds IF_STR;
static sds BEGIN_STR;
static sds WHILE_STR;
static sds SET_STR;

static Vector *vm_compile_SexpObject(SexpObject *obj);

static inline void vm_compile_binary_fun(Opcode op, Vector *v, Vector *ret) {
  assert(v->len == 3);
  SexpObject *e1 = (SexpObject *)v->data[1]->ptr;
  SexpObject *e2 = (SexpObject *)v->data[2]->ptr;
  vec_append(ret, vm_compile_SexpObject(e1));
  vec_append(ret, vm_compile_SexpObject(e2));
  vec_pushi(ret, op);
}

static ssize_t check_var_name_in_arg_names(sds var_name, Vector *arg_names) {
  ssize_t found = -1;
  for (size_t i = 0; i < arg_names->len; i++) {
    sds arg_name = (sds)arg_names->data[arg_names->len - i - 1]->ptr;
    if (strcmp(var_name, arg_name) == 0) {
      found = i;
      break;
    }
  }
  return found;
}

static Vector *vm_compile_SexpObject(SexpObject *obj) {
  Vector *ret = new_vec();

  switch (obj->ty) {
  case float_ty:
  case bool_ty:
  case string_ty:
    vec_pushi(ret, OpPush);
    vec_push(ret, new_GeneralPointer(new_VMValueWithValue(obj),
                                     (ELEM_DESTRUCTOR)&free_VMValue));
    break;
  case symbol_ty:
    vec_pushi(ret, OpGetVar);
    vec_push(ret,
             new_GeneralPointer(obj->symbol_val, (ELEM_DESTRUCTOR)&sdsfree));
    break;
  case list_ty: {
    Vector *v = obj->list_val;
    assert(v->len > 0);
    SexpObject *e1 = (SexpObject *)v->data[0]->ptr;
    assert(e1->ty == symbol_ty);
    sds func_name = e1->symbol_val;

    if (sdscmp(func_name, ADD_STR) == 0) { // Add
      vm_compile_binary_fun(OpAdd, v, ret);
    } else if (sdscmp(func_name, SUB_STR) == 0) { // Sub
      vm_compile_binary_fun(OpSub, v, ret);
    } else if (sdscmp(func_name, MUL_STR) == 0) { // Mul
      vm_compile_binary_fun(OpMul, v, ret);
    } else if (sdscmp(func_name, DIV_STR) == 0) { // Div
      vm_compile_binary_fun(OpDiv, v, ret);
    } else if (sdscmp(func_name, MOD_STR) == 0) { // Mod
      vm_compile_binary_fun(OpMod, v, ret);
    } else if (sdscmp(func_name, EQ_STR) == 0) { // Eq
      vm_compile_binary_fun(OpEq, v, ret);
    } else if (sdscmp(func_name, NEQ_STR) == 0) { // Neq
      vm_compile_binary_fun(OpNeq, v, ret);
    } else if (sdscmp(func_name, LT_STR) == 0) { // Lt
      vm_compile_binary_fun(OpLt, v, ret);
    } else if (sdscmp(func_name, LEQ_STR) == 0) { // Leq
      vm_compile_binary_fun(OpLeq, v, ret);
    } else if (sdscmp(func_name, GT_STR) == 0) { // Gt
      vm_compile_binary_fun(OpGt, v, ret);
    } else if (sdscmp(func_name, GEQ_STR) == 0) { // Geq
      vm_compile_binary_fun(OpGeq, v, ret);
    } else if (sdscmp(func_name, DEF_VAR_STR) == 0) { // def-var
      assert(v->len == 3);
      SexpObject *e1 = (SexpObject *)v->data[1]->ptr;
      SexpObject *e2 = (SexpObject *)v->data[2]->ptr;
      assert(e1->ty == symbol_ty);
      vec_append(ret, vm_compile_SexpObject(e2));
      vec_pushi(ret, OpVarDef);
      vec_push(ret,
               new_GeneralPointer(e1->symbol_val, (ELEM_DESTRUCTOR)&sdsfree));
    } else if (sdscmp(func_name, DEF_FUN_STR) == 0) { // def-fun
      assert(v->len >= 4);
      SexpObject *e1 = (SexpObject *)v->data[1]->ptr;
      SexpObject *e2 = (SexpObject *)v->data[2]->ptr;
      assert(e1->ty == symbol_ty); // func_name
      assert(e2->ty == list_ty);   // args

      Vector *arg_names = NULL;
      Vector *args = e2->list_val;
      for (size_t i = 0; i < args->len; i++) {
        if (i == 0) {
          arg_names = new_vec();
        }
        SexpObject *arg = (SexpObject *)args->data[i]->ptr;
        assert(arg->ty == symbol_ty);
        vec_push(arg_names, new_GeneralPointer(arg->symbol_val,
                                               (ELEM_DESTRUCTOR)&sdsfree));
      }

      Vector *func_body = new_vec();

      for (size_t i = 3; i < v->len; i++) {
        SexpObject *e = (SexpObject *)v->data[i]->ptr;
        vec_append(func_body, vm_compile_SexpObject(e));
      }

      Vector *func_body_opt = new_vec();

      Vector *lvars = new_vec();

      // TODO : Need to remove memory leaks(func_body ->
      // func_body_optにコピーしていない部分をfreeする必要がある)
      for (size_t i = 0; i < func_body->len;) {
        Opcode op = (Opcode)func_body->data[i++]->ptr;
        const int op_width = get_op_width(op) - 1;

        switch (op) {
        default: {
          vec_pushi(func_body_opt, op);
          for (int j = 0; j < op_width; j++) {
            vec_push(func_body_opt, func_body->data[i++]);
          }
          break;
        }
        case OpVarDef:
        case OpGetVar:
        case OpSetVar: {
          sds var_name = (sds)func_body->data[i++]->ptr;

          ssize_t found = -1;
          size_t arg_names_len = 0;
          if (arg_names != NULL) {
            found = check_var_name_in_arg_names(var_name, arg_names);
            arg_names_len = arg_names->len;
          }
          if (found == -1) {
            if (op == OpVarDef || op == OpSetVar) {
              ssize_t lvar_idx_1 = -1;
              if (op == OpVarDef) {
                vec_push(lvars, new_GeneralPointer(var_name,
                                                   (ELEM_DESTRUCTOR)&sdsfree));
                lvar_idx_1 = lvars->len - 1;
              } else {
                for (size_t j = 0; j < lvars->len; j++) {
                  sds lvar_name = (sds)lvars->data[j]->ptr;
                  if (strcmp(var_name, lvar_name) == 0) {
                    lvar_idx_1 = j;
                    break;
                  }
                }
              }

              if (lvar_idx_1 == -1) {
                vec_pushi(func_body_opt, op);
                vec_push(
                    func_body_opt,
                    new_GeneralPointer(var_name, (ELEM_DESTRUCTOR)&sdsfree));
              } else {
                size_t lvar_idx = arg_names_len + lvar_idx_1;
                vec_pushi(func_body_opt, OpSetLocal);
                vec_pushi(func_body_opt, lvar_idx);
              }
            } else {
              ssize_t found_lvar = -1;
              for (size_t j = 0; j < lvars->len; j++) {
                sds lvar_name = (sds)lvars->data[j]->ptr;
                if (strcmp(var_name, lvar_name) == 0) {
                  found_lvar = j;
                  break;
                }
              }
              if (found_lvar == -1) {
                // if not found, raise exception by OpGetVar
                vec_pushi(func_body_opt, op);
                vec_push(
                    func_body_opt,
                    new_GeneralPointer(var_name, (ELEM_DESTRUCTOR)&sdsfree));
              } else {
                ssize_t lvar_idx = arg_names_len + found_lvar;
                vec_pushi(func_body_opt, OpGetLocal);
                vec_pushi(func_body_opt, lvar_idx);
              }
            }
          } else {
            vec_pushi(func_body_opt, op == OpVarDef ? OpSetLocal : OpGetLocal);
            vec_pushi(func_body_opt, found);
          }

          break;
        }
        }
      }

      func_body = new_vec();

      size_t alloca_size = 0;

      if (arg_names != NULL) {
        alloca_size += arg_names->len;
      }
      alloca_size += lvars->len;

      vec_pushi(func_body, OpAllocLvars);
      vec_pushi(func_body, alloca_size);

      if (arg_names != NULL) {
        for (size_t i = 0; i < arg_names->len; i++) {
          vec_pushi(func_body, OpSetArgLocal);
          vec_pushi(func_body, i);
        }
      }

      vec_append(func_body, func_body_opt);

      if (alloca_size) {
        vec_pushi(func_body, OpFreeLvars);
      }
      vec_pushi(func_body, OpReturn);

      VMFunction *vmf = new_VMFunction(e1->symbol_val, func_body, arg_names);
      vec_pushi(ret, OpFuncDef);
      vec_push(ret, new_GeneralPointer(new_VMValueWithFunc(vmf),
                                       (ELEM_DESTRUCTOR)&free_VMValue));

      free_vec(lvars);
    } else if (sdscmp(func_name, IF_STR) == 0) { // IF
      assert(v->len >= 3);
      SexpObject *cond = v->data[1]->ptr;
      SexpObject *tBlock = v->data[2]->ptr;

      vec_append(ret, vm_compile_SexpObject(cond));

      Vector *tBlock_ins = vm_compile_SexpObject(tBlock);

      long long int tBlock_len = tBlock_ins->len;
      vec_pushi(ret, OpBranch);
      vec_pushi(ret, tBlock_len + (v->len == 4 ? 2 : 0)); // skip fBlock

      vec_append(ret, tBlock_ins);

      if (v->len == 4) { // exist fBlock
        Vector *fBlock_ins = vm_compile_SexpObject(v->data[3]->ptr);
        long long int fBlock_len = fBlock_ins->len;
        vec_pushi(ret, OpJumpRel);
        vec_pushi(ret, fBlock_len);

        vec_append(ret, fBlock_ins);
      }
    } else if (sdscmp(func_name, BEGIN_STR) == 0) { // begin
      assert(v->len >= 2);
      for (size_t i = 1; i < v->len; i++) {
        vec_append(ret, vm_compile_SexpObject(v->data[i]->ptr));
      }
    } else if (sdscmp(func_name, WHILE_STR) == 0) { // while
      assert(v->len == 3);
      SexpObject *cond = v->data[1]->ptr;
      SexpObject *expr = v->data[2]->ptr;

      Vector *cond_ins = vm_compile_SexpObject(cond);
      Vector *expr_ins = vm_compile_SexpObject(expr);
      vec_pushi(expr_ins, OpJumpRel);
      int joffset =
          -(expr_ins->len + 1 /* this jump */ + cond_ins->len + 2 /* branch */);
      vec_pushi(expr_ins, joffset);

      vec_append(ret, cond_ins);
      vec_pushi(ret, OpBranch);
      vec_pushi(ret, expr_ins->len);
      vec_append(ret, expr_ins);
    } else if (sdscmp(func_name, SET_STR) == 0) {
      assert(v->len == 3);
      SexpObject *var_name = v->data[1]->ptr;
      assert(var_name->ty == symbol_ty);
      SexpObject *expr = v->data[2]->ptr;
      Vector *expr_ins = vm_compile_SexpObject(expr);

      vec_append(ret, expr_ins);
      vec_pushi(ret, OpSetVar);
      vec_push(ret, new_GeneralPointer(var_name->symbol_val,
                                       (ELEM_DESTRUCTOR)&sdsfree));
    } else {
      size_t i = 0;

      for (; i < v->len - 1; i++) {
        SexpObject *e = (SexpObject *)v->data[i + 1]->ptr;
        vec_append(ret, vm_compile_SexpObject(e));
      }

      vec_pushi(ret, OpCall);
      vec_push(ret, new_GeneralPointer(func_name, (ELEM_DESTRUCTOR)&sdsfree));
      vec_pushi(ret, i);
    }
    break;
  }
  case object_ty:
    fprintf(stderr, "unimplemented!\n");
    exit(EXIT_FAILURE);
  case quote_ty: {
    SexpObject *iobj = obj->quote_val;

    switch (iobj->ty) {
    case list_ty: {
      Vector *v = iobj->list_val;
      size_t i = 0;

      for (; i < v->len; i++) {
        SexpObject *e = (SexpObject *)v->data[i]->ptr;
        vec_append(ret, vm_compile_SexpObject(e));
      }

      vec_pushi(ret, OpMakeList);
      vec_pushi(ret, i);
      break;
    }
    case float_ty:
    case bool_ty:
    case string_ty:
    case symbol_ty:
    case object_ty:
      fprintf(stderr, "unimplemented!\n");
      exit(EXIT_FAILURE);
    }
  }
  }
  return ret;
}

Vector *vm_compile(Vector *parsed) {
  Vector *ret = new_vec();

  for (size_t i = 0; i < parsed->len; i++) {
    SexpObject *obj = (SexpObject *)parsed->data[i]->ptr;
    vec_append(ret, vm_compile_SexpObject(obj));
  }

  return ret;
}

void vm_compiler_init(void) {
  ADD_STR = sdsnew("+");
  SUB_STR = sdsnew("-");
  MUL_STR = sdsnew("*");
  DIV_STR = sdsnew("/");
  MOD_STR = sdsnew("%%");
  EQ_STR = sdsnew("==");
  NEQ_STR = sdsnew("!=");
  LT_STR = sdsnew("<");
  LEQ_STR = sdsnew("<=");
  GT_STR = sdsnew(">");
  GEQ_STR = sdsnew(">=");
  DEF_VAR_STR = sdsnew("def-var");
  DEF_FUN_STR = sdsnew("def-fun");
  IF_STR = sdsnew("if");
  BEGIN_STR = sdsnew("begin");
  WHILE_STR = sdsnew("while");
  SET_STR = sdsnew("set");
}