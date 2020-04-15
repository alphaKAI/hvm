require "tomlrb"

CLANG_FORMAT_CMD="clang-format"


def gen_DIRECT_THREADED(instructions)
  helper = %Q[
static inline void **gen_table(void **table, long long int table_len,
                               Vector *v_ins, void *L_end) {
  void **ops_ptr = xmalloc(sizeof(void *) * (v_ins->len + 1));
  for (size_t j = 0; j < v_ins->len; j++) {
    long long int idx = (long long int)v_ins->data[j]->ptr;
    if (idx < table_len) {
      ops_ptr[j] = table[idx];
    }
  }
  ops_ptr[v_ins->len] = L_end;

  return ops_ptr;
}
  ]

  prologue = %Q[
  if (!vm_initialized) {
    vm_init();
  }

  SexpObject *ret = NULL;
  register Stack *stack = new_Stack();
  register Stack *frame_stack = new_Stack();
  Frame *frame = new_Frame();
  frame->v_ins = v_ins;
  register Registers *reg = frame->registers;
  size_t bop_argc = 0;

  static void *table[] = {
      &&L_OpPop,      &&L_OpPush,       &&L_OpAllocLvars,  &&L_OpFreeLvars,
      &&L_OpGetLocal, &&L_OpSetLocal,   &&L_OpSetArgLocal, &&L_OpAdd,
      &&L_OpSub,      &&L_OpMul,        &&L_OpDiv,         &&L_OpMod,
      &&L_OpEq,       &&L_OpNeq,        &&L_OpLt,          &&L_OpLeq,
      &&L_OpGt,       &&L_OpGeq,        &&L_OpPrint,       &&L_OpPrintln,
      &&L_OpJumpRel,  &&L_OpFuncDef,    &&L_OpCall,        &&L_OpReturn,
      &&L_OpVarDef,   &&L_OpGetVar,     &&L_OpSetVar,      &&L_OpBranch,
      &&L_OpMakeList, &&L_OpSetArgFrom, &&L_OpDumpEnv};
  static const long long int table_len = sizeof(table) / sizeof(table[0]);
  void **ops_ptr = gen_table(table, table_len, v_ins, &&L_end);

//    printf("op: %s, reg: %p, reg->pc: %ld\\n", #op_name, reg, reg->pc);

  // start
  long long int op = (long long int)(intptr_t)v_ins->data[reg->pc++]->ptr;
  goto *table[op];

L_MAIN_LOOP:
  reg->pc++;
  goto *table[reg->pc];

L_OP_SELECT:
  goto *table[op];
  ]
  epilogue=%Q[
L_end:
  // 戻るべきフレームが存在する．
  if (frame->parent != NULL) {
    free_Frame(frame);
    POP_STACK_WITH_FREE(frame_stack, stack); // フレームを復元
    reg = frame->registers;
    POP_STACK_WITH_FREE(frame_stack, ops_ptr);
    goto L_MAIN_LOOP;
  }

  if (!isempty_Stack(stack)) {
    ret = (SexpObject *)pop_Stack(stack);
  }

  return ret;
  ]

  puts "#ifdef __ENABLE_DIRECT_THREADED_CODE__"

  puts helper

  puts "SexpObject *vm_exec(Vector *v_ins) {"
  puts prologue

  instructions.each do |op_name, contents|
    puts "L_#{op_name}: {"
    puts contents["code"]
    puts "goto *ops_ptr[reg->pc++];"
    puts "}"
  end

  puts epilogue
  puts "}"
  puts "#endif"
end

def gen_NON_DIRECT_THREADED(instructions)
  prologue = %Q[
  if (!vm_initialized) {
    vm_init();
  }

  SexpObject *ret = NULL;
  Stack *stack = new_Stack();
  Stack *frame_stack = new_Stack();
  Frame *frame = new_Frame();
  frame->v_ins = v_ins;
  Registers *reg = frame->registers;
  size_t bop_argc = 0;

MAIN_LOOP:
  for (; reg->pc < frame->v_ins->len;) {
    Opcode op = (Opcode)frame->v_ins->data[reg->pc++]->ptr;
    // printf("op: %lld, reg: %p, reg->pc: %lld\\n", op, reg, reg->pc);
  OP_SELECT:
    switch (op) {
  ]

  epilogue = %Q[
    default:
      fprintf(stderr, "Unkown op given. op: %lld\\n", op);
      exit(EXIT_FAILURE);
      break;
    }
  }

  // 戻るべきフレームが存在する．
  if (frame->parent != NULL) {
    free_Frame(frame);
    POP_STACK_WITH_FREE(frame_stack, stack); // フレームを復元
    reg = frame->registers;
    goto MAIN_LOOP;
  }

  if (!isempty_Stack(stack)) {
    ret = (SexpObject *)pop_Stack(stack);
  }

  return ret;
  ]

  puts "#ifndef __ENABLE_DIRECT_THREADED_CODE__"
  puts "SexpObject *vm_exec(Vector *v_ins) {"
  puts prologue

  instructions.each do |op_name, contents|
    puts "case #{op_name}: #{contents["code"]} break;"
  end

  puts epilogue
  puts "}"
  puts "#endif"
end

def main()
  instructions = Tomlrb.load_file("instructions.toml")

  instructions.each do |op, content|
    code = content["code"]
    block_formatted = `echo '#{code}' | #{CLANG_FORMAT_CMD}`
    instructions[op]["code"] = block_formatted
  end


  # Code Gen
  gen_DIRECT_THREADED(instructions)
  gen_NON_DIRECT_THREADED(instructions)
end

main
