BUILD_DIR=build
CFLAGS=-std=c11 -g -Wall -Wextra -Werror -fstack-protector
SRCS=$(wildcard *.c)
OBJS=$(addprefix $(BUILD_DIR)/, $(SRCS:.c=.o))

all: $(BUILD_DIR)/myc

$(BUILD_DIR)/myc: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

$(BUILD_DIR)/%.o: %.c myc.h
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

test: $(BUILD_DIR)/myc
	./test.sh

clean:
	rm -rf $(BUILD_DIR) *~ tmp*

.PHONY: test clean all

