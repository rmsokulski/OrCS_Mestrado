CXX      ?= g++
# -fno-tree-vectorize keeps the scalar baseline truly scalar (no SSE/AVX).
CXXFLAGS ?= -O2 -g -fno-tree-vectorize
CXXFLAGS += -I../common

all: vecsum.x86 vecsum.vima

vecsum.x86: vecsum.cpp ../common/orcs_tracing.h
	$(CXX) $(CXXFLAGS) $< -o $@

vecsum.vima: vecsum.cpp ../common/orcs_tracing.h
	$(CXX) $(CXXFLAGS) -DUSE_VIMA $< -o $@

clean:
	rm -f vecsum.x86 vecsum.vima

.PHONY: all clean
