JANSSON_CFLAGS := $(shell pkg-config --cflags jansson)
JANSSON_LIBS := $(shell pkg-config --libs jansson)

all: test

test-bin: test.cpp jansson.hpp jansson-impl.hpp Makefile
	$(CXX) -o $@ -g -O0 -Wall $(JANSSON_CFLAGS) $< $(JANSSON_LIBS)

test: test-bin
	./test-bin

clean:
	rm -f test-bin
