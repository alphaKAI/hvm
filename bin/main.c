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
  if (argc < 3) {
    fprintf(stderr, "usage: %s -x prog_path\n", argv[0]);
    exit(EXIT_FAILURE);
  }

#ifdef __USE_BOEHM_GC__
  gc_init();
#endif

  vm_init();

  if (argc == 3) {
    const char *opt = argv[1];

    if (strcmp(opt, "-x") == 0) { // execute
      Vector *read_data = read_file_from_llis(argv[2]);
      Vector *deserialized = vm_deserialize(read_data);
      printf("compiled instructions...\n");
      vm_ins_dump(deserialized);
      vm_exec(deserialized);
      free_vec(deserialized);
      free_vec(read_data);
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
