#include "hvm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  Vector *serialized;
  size_t read_len;
} DeserializeResult;

typedef struct {
  sds str;
  size_t read_len;
} DeserializeStringResult;

static inline DeserializeStringResult deserialize_string(Vector *serialized,
                                                         size_t first_idx) {
  size_t idx = first_idx;
  size_t str_len = (size_t)serialized->data[idx++]->ptr;

  char *buf = xmalloc(str_len + 1);
  for (size_t i = 0; i < str_len; i++) {
    char c = (char)serialized->data[idx++]->ptr;
    buf[i] = c;
  }
  buf[str_len] = '\0';

  DeserializeStringResult dsr = {.str = sdsnew(buf),
                                 .read_len = idx - first_idx};
  return dsr;
}

static DeserializeResult deserialize_vmv(Vector *serialized, size_t first_idx) {
  Vector *ret = new_vec();
  size_t idx = first_idx;

  int ty = (intptr_t)serialized->data[idx++]->ptr;
  switch (ty) {
  case VValue: {
    int obj_ty = (intptr_t)serialized->data[idx++]->ptr;

    switch (obj_ty) {
    case float_ty: {
      long long int lv = (long long int)serialized->data[idx++]->ptr;
      double dv;
      memcpy(&dv, &lv, sizeof(double));
      vec_push(ret, new_GeneralPointer(
                        new_VMValueWithValue(new_SexpObject_float(dv)),
                        (ELEM_DESTRUCTOR)&free_VMValue));
      break;
    }
    case bool_ty: {
      unimplemented();
      break;
    }
    case string_ty:
    case symbol_ty: {
      DeserializeStringResult dsr = deserialize_string(serialized, idx);
      idx += dsr.read_len;

      if (obj_ty == string_ty) {
        vec_push(ret, new_GeneralPointer(
                          new_VMValueWithValue(new_SexpObject_string(dsr.str)),
                          (ELEM_DESTRUCTOR)&free_VMValue));
      } else {
        vec_push(ret, new_GeneralPointer(
                          new_VMValueWithValue(new_SexpObject_symbol(dsr.str)),
                          (ELEM_DESTRUCTOR)&free_VMValue));
      }

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
    DeserializeStringResult dsr = deserialize_string(serialized, idx);
    idx += dsr.read_len;
    sds func_name = dsr.str;

    Vector *func_body_serialized = new_vec();
    long long int code_len = (long long int)serialized->data[idx++]->ptr;
    for (long long int i = 0; i < code_len; i++) {
      vec_push(func_body_serialized, serialized->data[idx++]);
    }
    Vector *func_body = vm_deserialize(func_body_serialized);

    size_t arg_count = (size_t)serialized->data[idx++]->ptr;
    Vector *args = NULL;
    for (size_t i = 0; i < arg_count; i++) {
      if (i == 0) {
        args = new_vec();
      }

      DeserializeStringResult arg_dsr = deserialize_string(serialized, idx);
      idx += arg_dsr.read_len;

      vec_push(args,
               new_GeneralPointer(arg_dsr.str, (ELEM_DESTRUCTOR)&sdsfree));
    }

    VMFunction *vmf = new_VMFunction(func_name, func_body, args);
    vec_push(ret, new_GeneralPointer(new_VMValueWithFunc(vmf),
                                     (ELEM_DESTRUCTOR)&free_VMValue));
    break;
  }
  default: {
    fprintf(stderr, "unkown type given : %d\n", ty);
    exit(EXIT_FAILURE);
    break;
  }
  }

  DeserializeResult result = {.serialized = ret, .read_len = idx - first_idx};
  return result;
}

static DeserializeResult deserialize_sds_size(Vector *serialized,
                                              size_t first_idx) {
  Vector *ret = new_vec();
  size_t idx = first_idx;

  DeserializeStringResult dsr = deserialize_string(serialized, idx);
  idx += dsr.read_len;
  vec_push(ret, new_GeneralPointer(dsr.str, (ELEM_DESTRUCTOR)&sdsfree));
  vec_pushi(ret, (size_t)serialized->data[idx++]->ptr);

  DeserializeResult result = {.serialized = ret, .read_len = idx - first_idx};
  return result;
}

static DeserializeResult *new_DeserializeResult(Vector *serialized) {
  DeserializeResult *ctx = xnew(DeserializeResult);
  ctx->serialized = serialized;
  ctx->read_len = 0;
  return ctx;
}

Vector *vm_deserialize(Vector *serialized) {
  Vector *code = new_vec();

  for (size_t i = 0; i < serialized->len;) {
    Opcode op = (Opcode)serialized->data[i++]->ptr;

    switch (op) {
    case OpPop:
      vec_pushi(code, op);
      break;
    case OpPush: {
      vec_pushi(code, OpPush);
      DeserializeResult result = deserialize_vmv(serialized, i);
      vec_append(code, result.serialized);
      free(result.serialized);
      i += result.read_len;
      break;
    }
    case OpAllocLvars:
    case OpGetLocal:
    case OpSetLocal:
    case OpSetArgLocal: {
      vec_pushi(code, op);
      long long int lv = (long long int)serialized->data[i++]->ptr;
      vec_pushlli(code, lv);
      break;
    }
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
      vec_pushi(code, op);
      break;
    case OpJumpRel: {
      vec_pushi(code, op);
      long long int lv = (long long int)serialized->data[i++]->ptr;
      vec_pushlli(code, lv);
      break;
    }
    case OpFuncDef: {
      vec_pushi(code, op);
      DeserializeResult result = deserialize_vmv(serialized, i);
      vec_append(code, result.serialized);
      free(result.serialized);
      i += result.read_len;
      break;
    }
    case OpCall: {
      vec_pushi(code, op);
      DeserializeResult result = deserialize_sds_size(serialized, i);
      vec_append(code, result.serialized);
      i += result.read_len;
      break;
    }
    case OpReturn: {
      vec_pushi(code, op);
      break;
    }
    case OpSetVar:
    case OpVarDef:
    case OpGetVar: {
      vec_pushi(code, op);
      DeserializeStringResult dsr = deserialize_string(serialized, i);
      vec_push(code, new_GeneralPointer(dsr.str, (ELEM_DESTRUCTOR)&sdsfree));
      i += dsr.read_len;
      break;
    }
    case OpBranch:
    case OpMakeList: {
      vec_pushi(code, op);
      long long int lv = (long long int)serialized->data[i++]->ptr;
      vec_pushlli(code, lv);
      break;
    }
    case OpSetArgFrom: {
      vec_pushi(code, op);
      DeserializeResult result = deserialize_sds_size(serialized, i);
      vec_append(code, result.serialized);
      i += result.read_len;
      break;
    }
    case OpDumpEnv: {
      vec_pushi(code, op);
      break;
    }
    default:
      if (0 <= op && op < size_of_op_to_string_table()) {
        fprintf(stderr, "[ERROR in vm_deserialize] Unkown op given. op: %s\n",
                op_to_string(op));
      } else {
        fprintf(stderr, "[ERROR in vm_deserialize] Unkown op given. op: %lld\n",
                op);
      }
      exit(EXIT_FAILURE);
      break;
    }
  }

  return code;
}
