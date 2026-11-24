CXX      ?= g++
# -fno-tree-vectorize keeps the scalar baseline truly scalar (no SSE/AVX).
# -static: Pin 3.11 aborts on the .relr.dyn section of glibc >= 2.36's ld.so.
CXXFLAGS ?= -O2 -g -fno-tree-vectorize -static
CXXFLAGS += -I../common -fopenmp

all: ompsync.x86 ompsync.vima

ompsync.x86: ompsync.cpp ../common/orcs_tracing.h
	$(CXX) $(CXXFLAGS) $< -o $@

ompsync.vima: ompsync.cpp ../common/orcs_tracing.h
	$(CXX) $(CXXFLAGS) -DUSE_VIMA $< -o $@

clean:
	rm -f ompsync.x86 ompsync.vima

.PHONY: all clean
