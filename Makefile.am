EXTRA_DIST = CHANGES LICENSE README.rst CMakeLists.txt cmake android examples
SUBDIRS = doc src test

# "make distcheck" builds the dvi target, so use it to check that the
# documentation is built correctly.
dvi:
	$(MAKE) SPHINXOPTS_EXTRA=-W html

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = jansson.pc


# Add fuzzing support
LIB_FUZZING_ENGINE ?= standaloneengine.o

ossfuzz/%.o: ossfuzz/%.cc
	$(CXX) -c -Isrc $(CXXFLAGS) $< -o $@

.PHONY: json_load_fuzzer
json_load_fuzzer: ossfuzz/json_load_fuzzer.o src/.libs/libjansson.a
	$(CXX) -c $(CXXFLAGS) ossfuzz/standaloneengine.cc -o standaloneengine.o
	$(CXX) $(CXXFLAGS) $(LIB_FUZZING_ENGINE) $^ -o $@$(EXT)
