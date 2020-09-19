CFLAGS=-std=c11 -g -static

myc: myc.c

test: myc 
	./test.sh

clean:
	rm -f myc *.o *~ tmp*

.PHONY: test clean

