#include "hvm.h"

static const int op_width_table[] = {
    [OpPop] = 1,         [OpPush] = 2,     [OpAllocLvars] = 2,
    [OpFreeLvars] = 1,   [OpGetLocal] = 2, [OpSetLocal] = 2,
    [OpSetArgLocal] = 2, [OpAdd] = 1,      [OpSub] = 1,
    [OpMul] = 1,         [OpDiv] = 1,      [OpMod] = 1,
    [OpEq] = 1,          [OpNeq] = 1,      [OpLt] = 1,
    [OpLeq] = 1,         [OpGt] = 1,       [OpGeq] = 1,
    [OpPrint] = 1,       [OpPrintln] = 1,  [OpJumpRel] = 2,
    [OpFuncDef] = 2,     [OpCall] = 3,     [OpReturn] = 1,
    [OpVarDef] = 2,      [OpGetVar] = 2,   [OpSetVar] = 2,
    [OpBranch] = 2,      [OpMakeList] = 2, [OpSetArgFrom] = 3,
    [OpDumpEnv] = 1};

#define SpecifyPlaceAndSetNameItSelf(e) [e] = #e

static const char *op_to_string_table[] = {
    SpecifyPlaceAndSetNameItSelf(OpPop),
    SpecifyPlaceAndSetNameItSelf(OpPush),
    SpecifyPlaceAndSetNameItSelf(OpAllocLvars),
    SpecifyPlaceAndSetNameItSelf(OpFreeLvars),
    SpecifyPlaceAndSetNameItSelf(OpGetLocal),
    SpecifyPlaceAndSetNameItSelf(OpSetLocal),
    SpecifyPlaceAndSetNameItSelf(OpSetArgLocal),
    SpecifyPlaceAndSetNameItSelf(OpAdd),
    SpecifyPlaceAndSetNameItSelf(OpSub),
    SpecifyPlaceAndSetNameItSelf(OpMul),
    SpecifyPlaceAndSetNameItSelf(OpDiv),
    SpecifyPlaceAndSetNameItSelf(OpMod),
    SpecifyPlaceAndSetNameItSelf(OpEq),
    SpecifyPlaceAndSetNameItSelf(OpNeq),
    SpecifyPlaceAndSetNameItSelf(OpLt),
    SpecifyPlaceAndSetNameItSelf(OpLeq),
    SpecifyPlaceAndSetNameItSelf(OpGt),
    SpecifyPlaceAndSetNameItSelf(OpGeq),
    SpecifyPlaceAndSetNameItSelf(OpPrint),
    SpecifyPlaceAndSetNameItSelf(OpPrintln),
    SpecifyPlaceAndSetNameItSelf(OpJumpRel),
    SpecifyPlaceAndSetNameItSelf(OpFuncDef),
    SpecifyPlaceAndSetNameItSelf(OpCall),
    SpecifyPlaceAndSetNameItSelf(OpReturn),
    SpecifyPlaceAndSetNameItSelf(OpVarDef),
    SpecifyPlaceAndSetNameItSelf(OpGetVar),
    SpecifyPlaceAndSetNameItSelf(OpSetVar),
    SpecifyPlaceAndSetNameItSelf(OpBranch),
    SpecifyPlaceAndSetNameItSelf(OpMakeList),
    SpecifyPlaceAndSetNameItSelf(OpSetArgFrom),
    SpecifyPlaceAndSetNameItSelf(OpDumpEnv)};

inline int get_op_width(int op) { return op_width_table[op]; }

inline const char *op_to_string(int op) { return op_to_string_table[op]; }

inline long long int size_of_op_to_string_table(void) {
  return (long long int)sizeof(op_to_string_table);
}