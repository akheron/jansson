CFLAGS = -I../include -std=c99 -Wall -Wextra -Werror -g -O0

OBJS = dump.o load.o value.o hashtable.o

LIB = libjansson.a

all: $(LIB)

$(LIB): $(OBJS)
	ar crsv $@ $^

clean:
	rm -f -- $(OBJS)
	rm -f -- $(LIB)
