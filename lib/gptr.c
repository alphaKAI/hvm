#include "hvm.h"
#include "stdlib.h"

GeneralPointer *new_GeneralPointer(void *ptr, GP_DESTRUCTOR destructor) {
  GeneralPointer *gp_ptr = xnew(GeneralPointer);

  gp_ptr->ptr = ptr;
  gp_ptr->destructor = destructor;

  return gp_ptr;
}

void free_GeneralPointer(GeneralPointer *gp) {
  if (gp == NULL) {
    fprintf(stderr, "[ERROR] free_GeneralPointer\n");
    exit(EXIT_FAILURE);
  }

  printf("free_GeneralPointer\n");
  printf("----- gp               : %p\n", gp);
  printf("----- gp->ptr         : %p\n", gp->ptr);
  printf("----- gp->destructor : %p\n", gp->destructor);

  if (gp != NULL && gp->destructor != NULL) {
    gp->destructor(gp->ptr);
  }

  free(gp);
}