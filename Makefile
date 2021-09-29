CFLAGS=-std=c11 -g -static
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

all : myc

myc: $(OBJS)
	$(CC) -o myc $(OBJS) $(LDFLAGS)

$(OBJS): myc.h


test: myc 
	./test.sh

clean:
	rm -f myc *.o *~ tmp*

.PHONY: test clean all

