.PHONY: all test clean

CC := cc
CFLAGS := -Wextra -Wall -O3 -lgc

TARGET = cplayground
SRCS = \
	$(shell find ./bin -name "*.c") \
	$(shell find ./lib -name "*.c") \
	$(shell find ./sds -name "*.c")
OBJS = $(shell find ./ -name "*.o")

GENERATED = generated

TEST_TARGET = cplayground_test
TEST_SRCS = \
	$(shell find ./lib -name "*.c") \
	$(shell find ./sds -name "*.c") \
	$(shell find ./tests -name "*.c")

all: $(TARGET)

test: build_test run_test

build_test: $(TEST_TARGET)

run_test:
	$(GENERATED)/$(TEST_TARGET)

$(TARGET): $(SRCS) | $(GENERATED)
	$(CC) -o $(addprefix $(GENERATED)/, $@) $^ $(CFLAGS) -I ./include -I .

$(TEST_TARGET): $(TEST_SRCS) | $(GENERATED)
	$(CC) -o $(addprefix $(GENERATED)/, $@) $^ $(CFLAGS)  -I ./ -I ./include -I sds

$(GENERATED):
	@mkdir -p $(GENERATED)

clean:
	$(RM) $(OBJS) $(addprefix $(GENERATED)/, $(TARGET) $(TEST_TARGET))
