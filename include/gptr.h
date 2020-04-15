#ifndef GPTR_HEADER_INCLUDED
#define GPTR_HEADER_INCLUDED

typedef void (*GP_DESTRUCTOR)(void *);

typedef struct {
  void *ptr;
  GP_DESTRUCTOR destructor;
} GeneralPointer;

GeneralPointer *new_GeneralPointer(void *ptr, GP_DESTRUCTOR destructor);
void free_GeneralPointer(GeneralPointer *gp_ptr);

#endif