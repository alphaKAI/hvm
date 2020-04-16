#include "hvm.h"
#include "stdlib.h"

GeneralPointer *new_GeneralPointer(void *ptr, GP_DESTRUCTOR destructor) {
  GeneralPointer *gp = xnew(GeneralPointer);

  gp->ptr = ptr;
  gp->destructor = destructor;

#ifdef HVM_DEBUG
  printf("new_GeneralPointer\n");
  printf("----- gp               : %p\n", gp);
  printf("----- gp->ptr         : %p\n", gp->ptr);
  printf("----- gp->destructor : %p\n", gp->destructor);
#endif

  return gp;
}

void free_GeneralPointer(GeneralPointer *gp) {
  if (gp == NULL) {
    fprintf(stderr, "[ERROR] free_GeneralPointer\n");
    exit(EXIT_FAILURE);
  }

#ifdef HVM_DEBUG
  printf("free_GeneralPointer\n");
  printf("----- gp               : %p\n", gp);
  printf("----- gp->ptr         : %p\n", gp->ptr);
  printf("----- gp->destructor : %p\n", gp->destructor);
#endif

  if (gp != NULL && gp->destructor != NULL) {
    gp->destructor(gp->ptr);
  }

  gp->ptr = NULL;
  gp->destructor = NULL;

  free(gp);
}