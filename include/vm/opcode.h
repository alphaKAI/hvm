#ifndef OPCODE_HEADER_INCLUDED
#define OPCODE_HEADER_INCLUDED
enum {
  OpPop,
  OpPush,
  OpAllocLvars,
  OpFreeLvars,
  OpGetLocal,
  OpSetLocal,
  OpSetArgLocal,
  OpAdd,
  OpSub,
  OpMul,
  OpDiv,
  OpMod,
  OpEq,
  OpNeq,
  OpLt,
  OpLeq,
  OpGt,
  OpGeq,
  OpPrint,
  OpPrintln,
  OpJumpRel,
  OpFuncDef,
  OpCall,
  OpReturn,
  OpVarDef,
  OpGetVar,
  OpSetVar,
  OpBranch,
  OpMakeList,
  OpSetArgFrom,
  OpDumpEnv
};

int get_op_width(int op);
const char *op_to_string(int op);
long long int size_of_op_to_string_table(void);

typedef long long int Opcode;

#endif