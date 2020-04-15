#include "hvm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __USE_BOEHM_GC__
#include <gc.h>
#endif

void c_version_compiler_is_deprecated_warn() {
  printf("[WARNING - deprecated] C version of Lisp Compiler is deprecated. You "
         "should use hcc(https://github.com/alphaKAI/hcc) instead of hcc's "
         "compiler function.\n");
}

int main(int argc, char const *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "usage: %s prog_path\n", argv[0]);
    exit(EXIT_FAILURE);
  }

#ifdef __USE_BOEHM_GC__
  GC_init();
#endif

  vm_init();

  if (argc == 2) {
    sds code = readText(sdsnew(argv[1]));
    Vector *parsed = sexp_parse(code);
    c_version_compiler_is_deprecated_warn();
    Vector *compiled = vm_compile(parsed);
    printf("compiled instructions...\n");
    printf("compiled->len : %ld\n", compiled->len);
    // for(size_t i = 0; i< compiled->len)
    vm_ins_dump(compiled);
    vm_exec(compiled);
  } else if (argc == 3) {
    const char *opt = argv[1];

    if (strcmp(opt, "-c") == 0) { // compile
      sds code = readText(sdsnew(argv[2]));
      Vector *parsed = sexp_parse(code);
      c_version_compiler_is_deprecated_warn();
      Vector *compiled = vm_compile(parsed);
      Vector *binarized = vm_binarize(compiled);

      sds compiled_name = sdscat(sdsnew(argv[2]), ".compiled");
      write_llis_to_file(compiled_name, binarized);

      printf("compiled finish. saved: %s\n", compiled_name);
    } else if (strcmp(opt, "-x") == 0) { // execute
      Vector *read_data = read_file_from_llis(argv[2]);
      Vector *deserialized = vm_deserialize(read_data);
      printf("compiled instructions...\n");
      vm_ins_dump(deserialized);
      vm_exec(deserialized);
    } else {
      fprintf(stderr, "Invalid argument!\n");
      exit(EXIT_FAILURE);
    }

  } else {
    fprintf(stderr, "Invalid argument!\n");
    exit(EXIT_FAILURE);
  }

  return 0;
}
