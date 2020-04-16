#include "hvm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static inline void serialize_string(Vector *ret, sds str) {
  size_t str_len = strlen(str);
  vec_pushlli(ret, str_len);
  for (size_t i = 0; i < str_len; i++) {
    char c = str[i];
    vec_pushi(ret, c);
  }
}

static Vector *binarize_SexpObject(VMValue *value) {
  Vector *ret = new_vec();

  vec_pushi(ret, value->ty);
  switch (value->ty) {
  case VValue: {
    SexpObject *obj = value->val;
    vec_pushi(ret, obj->ty);

    switch (obj->ty) {
    case float_ty: {
      double fv = obj->float_val;
      long long int *ptr = (long long int *)&fv;
      vec_pushlli(ret, *ptr);
      break;
    }
    case bool_ty: {
      bool bv = obj->bool_val;
      vec_pushi(ret, bv);
      break;
    }
    case string_ty:
    case symbol_ty: {
      sds str;
      if (obj->ty == string_ty) {
        str = obj->string_val;
      } else {
        str = obj->symbol_val;
      }

      serialize_string(ret, str);

      break;
    }
    case list_ty:
      unimplemented();
      break;
    case object_ty:
      unimplemented();
      break;
    case quote_ty:
      unimplemented();
      break;
    default:
      break;
    }

    break;
  }
  case VFunc: {
    VMFunction *vmf = value->func;
    serialize_string(ret, vmf->name);

    Vector *code = vm_binarize(vmf->code);
    vec_pushlli(ret, code->len);
    vec_append(ret, code);

    Vector *arg_names = vmf->arg_names;
    if (arg_names == NULL) {
      vec_pushlli(ret, 0);
    } else {
      vec_pushlli(ret, arg_names->len);
      for (size_t i = 0; i < arg_names->len; i++) {
        sds arg_name = (sds)arg_names->data[i]->ptr;
        serialize_string(ret, arg_name);
      }
    }
    break;
  }
  }

  return ret;
}

typedef struct {
  Vector *ret;
} SerializeContext;

SerializeContext *new_SerializeContext(void) {
  SerializeContext *ctx = xnew(SerializeContext);
  ctx->ret = new_vec();
  return ctx;
}

static inline void emit_op(SerializeContext *ctx, Opcode op) {
  vec_pushlli(ctx->ret, op);
}

static inline void emit_op_1arg_vmv(SerializeContext *ctx, Opcode op,
                                    VMValue *vmvalue) {
  emit_op(ctx, op);
  vec_append(ctx->ret, binarize_SexpObject(vmvalue));
}

static inline void emit_op_1arg_llv(SerializeContext *ctx, Opcode op,
                                    long long int llv) {
  emit_op(ctx, op);
  vec_pushlli(ctx->ret, llv);
}

static inline void emit_op_1arg_sds(SerializeContext *ctx, Opcode op, sds str) {
  emit_op(ctx, op);
  serialize_string(ctx->ret, str);
}

static inline void emit_op_2arg_sds_size(SerializeContext *ctx, Opcode op,
                                         sds str, size_t size) {
  emit_op_1arg_sds(ctx, op, str);
  vec_pushlli(ctx->ret, size);
}

Vector *vm_binarize(Vector *v_ins) {
  SerializeContext *ctx = new_SerializeContext();

  for (size_t i = 0; i < v_ins->len;) {
    Opcode op = (Opcode)v_ins->data[i++]->ptr;

    switch (op) {
    case OpPop:
      emit_op(ctx, op);
      break;
    case OpPush:
      emit_op_1arg_vmv(ctx, op, (VMValue *)v_ins->data[i++]->ptr);
      break;
    case OpAllocLvars:
    case OpGetLocal:
    case OpSetLocal:
    case OpSetArgLocal:
      emit_op_1arg_llv(ctx, op, (long long int)v_ins->data[i++]->ptr);
      break;
    case OpFreeLvars:
    case OpAdd:
    case OpSub:
    case OpMul:
    case OpDiv:
    case OpMod:
    case OpEq:
    case OpNeq:
    case OpLt:
    case OpLeq:
    case OpGt:
    case OpGeq:
    case OpPrint:
    case OpPrintln:
      emit_op(ctx, op);
      break;
    case OpJumpRel: {
      long long int llv = (long long int)v_ins->data[i++]->ptr;
      emit_op_1arg_llv(ctx, op, llv);
      break;
    }
    case OpFuncDef: {
      VMValue *vfptr = v_ins->data[i++]->ptr;
      emit_op_1arg_vmv(ctx, op, vfptr);
      break;
    }
    case OpCall: {
      sds func_name = (sds)v_ins->data[i++]->ptr;
      size_t argc = (size_t)v_ins->data[i++]->ptr;
      emit_op_2arg_sds_size(ctx, op, func_name, argc);
      break;
    }
    case OpReturn: {
      emit_op(ctx, op);
      break;
    }
    case OpVarDef: {
      sds var_name = (sds)v_ins->data[i++]->ptr;
      emit_op_1arg_sds(ctx, op, var_name);
      break;
    }
    case OpGetVar: {
      sds var_name = (sds)v_ins->data[i++]->ptr;
      emit_op_1arg_sds(ctx, op, var_name);
      break;
    }
    case OpBranch: {
      long long int tBlock_len = (long long int)(intptr_t)v_ins->data[i++]->ptr;
      emit_op_1arg_llv(ctx, op, tBlock_len);
      break;
    }
    case OpMakeList: {
      long long int list_len = (long long int)(intptr_t)v_ins->data[i++]->ptr;
      emit_op_1arg_llv(ctx, op, list_len);
      break;
    }
    case OpSetArgFrom: {
      sds arg_name = (sds)v_ins->data[i++]->ptr;
      size_t arg_idx = (size_t)(intptr_t)v_ins->data[i++]->ptr;
      emit_op_2arg_sds_size(ctx, op, arg_name, arg_idx);
      break;
    }
    case OpDumpEnv: {
      emit_op(ctx, op);
      break;
    }
    default:
      if (0 <= op && op < size_of_op_to_string_table()) {
        fprintf(stderr, "Unkown op given. op: %s\n", op_to_string(op));
      } else {
        fprintf(stderr, "Unkown op given. op: %lld\n", op);
      }
      exit(EXIT_FAILURE);
      break;
    }
  }

  return ctx->ret;
}