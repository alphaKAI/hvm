#ifndef REGISTERS_HEADER_INCLUDED
#define REGISTERS_HEADER_INCLUDED

typedef struct {
  size_t pc;
} Registers;

Registers *new_Registers(void);
void free_Registers(Registers *registers);

#endif