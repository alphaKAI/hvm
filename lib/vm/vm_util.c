#include "hvm.h"
#include <stdio.h>
#include <stdlib.h>

static void vm_ins_dump_impl(Vector *v_ins, size_t depth) {
  sds sp = sdsempty();

  for (size_t i = 0; i < depth; i++) {
    sp = sdscatprintf(sp, "  ");
  }

  for (size_t i = 0; i < v_ins->len;) {
    printf("%s[ins: %ld] ", sp, i);
    Opcode op = (Opcode)v_ins->data[i++]->ptr;
    switch (op) {
    case OpPop:
      printf("OpPop\n");
      break;
    case OpPush:
      printf("OpPush %s\n", show_VMValue(v_ins->data[i++]->ptr));
      break;
    case OpAllocLvars:
      printf("OpAllocLvars %lld\n", (long long int)v_ins->data[i++]->ptr);
      break;
    case OpFreeLvars:
      printf("OpFreeLvars\n");
      break;
    case OpGetLocal:
      printf("OpGetLocal %lld\n", (long long int)v_ins->data[i++]->ptr);
      break;
    case OpSetLocal:
      printf("OpSetLocal %lld\n", (long long int)v_ins->data[i++]->ptr);
      break;
    case OpSetArgLocal:
      printf("OpSetArgLocal %lld\n", (long long int)v_ins->data[i++]->ptr);
      break;
    case OpAdd: {
      printf("OpAdd\n");
      break;
    }
    case OpSub: {
      printf("OpSub\n");
      break;
    }
    case OpMul: {
      printf("OpMul\n");
      break;
    }
    case OpDiv: {
      printf("OpDiv\n");
      break;
    }
    case OpMod: {
      printf("OpMod\n");
      break;
    }
    case OpEq: {
      printf("OpEq\n");
      break;
    }
    case OpNeq: {
      printf("OpNeq\n");
      break;
    }
    case OpLt: {
      printf("OpLt\n");
      break;
    }
    case OpLeq: {
      printf("OpLeq\n");
      break;
    }
    case OpGt: {
      printf("OpGt\n");
      break;
    }
    case OpGeq: {
      printf("OpGeq\n");
      break;
    }
    case OpPrint: {
      printf("OpPrint\n");
      break;
    }
    case OpPrintln: {
      printf("OpPrintln\n");
      break;
    }
    case OpJumpRel: {
      long long int offset = (long long int)(intptr_t)v_ins->data[i++]->ptr;
      printf("OpJumpRel %lld\n", offset);
      break;
    }
    case OpFuncDef: {
      VMValue *vfptr = v_ins->data[i++]->ptr;
      VMFunction *vmf = get_func_VMValue(vfptr);
      printf("OpFuncDef %s\n", vmf->name);
      vm_ins_dump_impl(vmf->code, depth + 1);
      break;
    }
    case OpCall: {
      sds func_name = (sds)v_ins->data[i++]->ptr;
      size_t argc = (size_t)v_ins->data[i++]->ptr;
      printf("OpCall %s, %ld\n", func_name, argc);
      break;
    }
    case OpReturn: {
      printf("OpReturn\n");
      break;
    }
    case OpVarDef: {
      sds var_name = (sds)v_ins->data[i++]->ptr;
      printf("OpVarDef %s\n", var_name);
      break;
    }
    case OpGetVar: {
      sds var_name = (sds)v_ins->data[i++]->ptr;
      printf("OpGetVar %s\n", var_name);
      break;
    }
    case OpSetVar: {
      sds var_name = (sds)v_ins->data[i++]->ptr;
      printf("OpSetVar %s\n", var_name);
      break;
    }
    case OpBranch: {
      long long int tBlock_len = (long long int)(intptr_t)v_ins->data[i++]->ptr;
      printf("OpBranch %lld\n", tBlock_len);
      break;
    }
    case OpDumpEnv: {
      printf("OpDumpEnv\n");
      break;
    }
    case OpMakeList: {
      size_t list_len = (intptr_t)v_ins->data[i++]->ptr;
      printf("OpMakeList %ld\n", list_len);
      break;
    }
    case OpSetArgFrom: {
      sds arg_name = (sds)v_ins->data[i++]->ptr;
      size_t arg_idx = (size_t)(intptr_t)v_ins->data[i++]->ptr;
      printf("OpSetArgFrom %s %ld\n", arg_name, arg_idx);
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
}

void vm_ins_dump(Vector *v_ins) { vm_ins_dump_impl(v_ins, 0); }