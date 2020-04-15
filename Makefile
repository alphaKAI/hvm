.PHONY: all test clean

CC := clang
CFLAGS := -Wextra -Wall -g  -lgc 

VM_EXEC_GENERATED = lib/vm_exec.generated
VM_EXEC_GEN_SRCS = \
									 ./gen_vm_exec.rb \
									 ./gen_vm_exec_generated.sh

TARGET = hvm
SRCS = \
	$(shell find ./bin -name "*.c") \
	$(shell find ./lib -name "*.c") \
	$(shell find ./sds -name "*.c")
OBJS = $(shell find ./ -name "*.o")

GENERATED = generated

TEST_TARGET = hvm_lib_test
TEST_SRCS = \
	$(shell find ./lib -name "*.c") \
	$(shell find ./sds -name "*.c") \
	$(shell find ./tests -name "*.c")

all: $(TARGET)

test: build_test run_test

build_test: $(TEST_TARGET)

run_test:
	$(GENERATED)/$(TEST_TARGET)

gen_vm_exec: 
	echo "GENERATE vm_exec.generated"
	./gen_vm_exec_generated.sh

$(TARGET): $(SRCS) | $(GENERATED)
	$(CC) -o $(addprefix $(GENERATED)/, $@) $^ $(CFLAGS) -I ./include -I .

$(TEST_TARGET): $(TEST_SRCS) | $(GENERATED)
	$(CC) -o $(addprefix $(GENERATED)/, $@) $^ $(CFLAGS)  -I ./ -I ./include -I sds

$(GENERATED):
	@mkdir -p $(GENERATED)

clean:
	$(RM) $(OBJS) $(addprefix $(GENERATED)/, $(TARGET) $(TEST_TARGET))
