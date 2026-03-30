BUILD_DIR=build
CFLAGS=-std=c11 -g -Wall -Wextra -Werror -fstack-protector
SRCS=$(filter-out test_helper.c, $(wildcard *.c))
OBJS=$(addprefix $(BUILD_DIR)/, $(SRCS:.c=.o))
TEST_HELPER=$(BUILD_DIR)/test_helper.o

all: $(BUILD_DIR)/myc $(TEST_HELPER)

$(BUILD_DIR)/myc: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

$(BUILD_DIR)/%.o: %.c myc.h
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(TEST_HELPER): test_helper.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

test: $(BUILD_DIR)/myc $(TEST_HELPER)
	gosh test.scm

clean:
	rm -rf $(BUILD_DIR) *~ tmp*

.PHONY: test clean all

