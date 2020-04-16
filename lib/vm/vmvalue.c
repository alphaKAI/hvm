#include "hvm.h"
#include <stdlib.h>

VMValue *new_VMValue(int ty, void *e) {
  VMValue *vmvalue = xmalloc(sizeof(VMValue));
  vmvalue->ty = ty;

  switch (ty) {
  case VValue:
    vmvalue->val = e;
    break;
  case VFunc:
    vmvalue->func = e;
    break;
  default:
    fprintf(stderr, "Invalid type tag\n");
    exit(EXIT_FAILURE);
  }

  return vmvalue;
}

void free_VMValue(VMValue *v_ptr) {
  switch (v_ptr->ty) {
  case VValue:
    free_SexpObject(v_ptr->val);
    break;
  case VFunc:
    free_VMFunction(v_ptr->func);
    break;
  default:
    fprintf(stderr, "Invalid type tag\n");
    exit(EXIT_FAILURE);
  }
  free(v_ptr);
}

VMValue *dup_VMValue(VMValue *value) {
  switch (value->ty) {
  case VValue:
    return new_VMValue(VValue, dup_SexpObject(value->val));
    break;
  case VFunc:
    return new_VMValue(VFunc, value->func);
  default:
    fprintf(stderr, "Invalid type tag\n");
    exit(EXIT_FAILURE);
  }
}

inline VMValue *new_VMValueWithValue(SexpObject *obj) {
  return new_VMValue(VValue, obj);
}

inline VMValue *new_VMValueWithFunc(VMFunction *vmf) {
  return new_VMValue(VFunc, vmf);
}

sds show_VMValue(VMValue *val) {
  switch (val->ty) {
  case VValue: {
    switch (val->val->ty) {
    case float_ty:
    case bool_ty:
    case symbol_ty:
    case string_ty:
#ifdef HVM_DEBUG
      return sdscatprintf(sdsempty(), "%s (ptr: %p)",
                          show_sexp_object_impl(val->val, false), val);
#else
      return sdscatprintf(sdsempty(), "%s",
                          show_sexp_object_impl(val->val, false));
#endif
    case list_ty: {
      sds ret = sdsnew("(");
      Vector *v = val->val->list_val;

      for (size_t i = 0; i < v->len; i++) {
        if (i == 0) {
          ret = sdscatprintf(ret, "%s", show_VMValue(v->data[i]->ptr));
        } else {
          ret = sdscatprintf(ret, " %s", show_VMValue(v->data[i]->ptr));
        }
      }

      ret = sdscatprintf(ret, ")");
      return ret;
    }
    case object_ty:
    case quote_ty:
      fprintf(stderr, "unimplemented!\n");
      exit(EXIT_FAILURE);
    }

    fprintf(stderr, "Should not reach here\n");
    exit(EXIT_FAILURE);
  }
  case VFunc:
    return sdscatprintf(sdsempty(), "func<%s>", val->func->name);
  default:
    return NULL;
  }
}

int cmp_VMValue(VMValue *v1, VMValue *v2) {
  if (v1->ty != v2->ty) {
    return -2;
  }
  switch (v1->ty) {
  case VValue: {
    switch (v1->val->ty) {
    case float_ty: {
      double l = v1->val->float_val;
      double r = v2->val->float_val;
      if (l == r) {
        return 0;
      }
      if (l < r) {
        return -1;
      }
      return 1;
    }
    case bool_ty: {
      bool l = v1->val->bool_val;
      bool r = v2->val->bool_val;
      if (l == r) {
        return 0;
      }
      return -2;
    }
    case symbol_ty:
      return varcmp((void *)v1->val->symbol_val, (void *)v2->val->symbol_val);
    case string_ty:
      return varcmp((void *)v1->val->string_val, (void *)v2->val->string_val);
    case list_ty:
    case object_ty:
    case quote_ty:
      fprintf(stderr, "unimplemented!\n");
      exit(EXIT_FAILURE);
    }

    fprintf(stderr, "Should not reach here\n");
    exit(EXIT_FAILURE);
  }
  case VFunc:
  default:
    fprintf(stderr, "Should not reach here\n");
    exit(EXIT_FAILURE);
  }
}

inline SexpObject *get_SexpObject_VMValue(VMValue *vmv) {
  assert(vmv->ty == VValue);
  return vmv->val;
}

inline VMFunction *get_func_VMValue(VMValue *vmv) {
  assert(vmv->ty == VFunc);
  return vmv->func;
}