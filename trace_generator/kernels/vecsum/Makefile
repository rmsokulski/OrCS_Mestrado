CXX      ?= g++
# -fno-tree-vectorize keeps the scalar baseline truly scalar (no SSE/AVX).
# -static: Pin 3.11 aborts on the .relr.dyn section of glibc >= 2.36's ld.so.
CXXFLAGS ?= -O2 -g -fno-tree-vectorize -static
CXXFLAGS += -I../common

all: vecsum.x86 vecsum.vima

vecsum.x86: vecsum.cpp ../common/orcs_tracing.h
	$(CXX) $(CXXFLAGS) $< -o $@

vecsum.vima: vecsum.cpp ../common/orcs_tracing.h
	$(CXX) $(CXXFLAGS) -DUSE_VIMA $< -o $@

clean:
	rm -f vecsum.x86 vecsum.vima

.PHONY: all clean
