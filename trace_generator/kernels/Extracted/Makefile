# Builds the traceable scan_main (all 20 versions in one binary).
#   make            -> build/scan_main.trace
# -static: Pin 3.11 aborts on the .relr.dyn section of glibc >= 2.36's ld.so.
# -g: symbols for the ORCS_tracing_start/stop markers.
# Only SIMD*/ get AVX-512 flags, so other versions never execute AVX-512.
# The timing build (run.sh -> build/scan_main) is unchanged.
CC      ?= gcc
CFLAGS  ?= -std=c11 -O2 -g -Wall -Wextra -fopenmp -static
AVX512  := -mavx512f -mavx512vl -mavx512dq
OBJ     := build/trace

PLAIN := $(wildcard Scalar/scan_*.c Parallel/scan_*.c VIMA/scan_*.c VIMA/vima_isa.c)
SIMD  := $(wildcard SIMD/scan_*.c SIMD_Parallel/scan_*.c)
OBJS  := $(addprefix $(OBJ)/,$(notdir $(PLAIN:.c=.o) $(SIMD:.c=.o)))

all: build/scan_main.trace

build/scan_main.trace: scan_main.c orcs_tracing_c.h $(OBJS)
	$(CC) $(CFLAGS) scan_main.c $(OBJS) -o $@

$(OBJ)/%.o: Scalar/%.c   | $(OBJ) ; $(CC) $(CFLAGS) -c $< -o $@
$(OBJ)/%.o: Parallel/%.c | $(OBJ) ; $(CC) $(CFLAGS) -c $< -o $@
$(OBJ)/%.o: VIMA/%.c     | $(OBJ) ; $(CC) $(CFLAGS) -c $< -o $@
$(OBJ)/%.o: SIMD/%.c          | $(OBJ) ; $(CC) $(CFLAGS) $(AVX512) -c $< -o $@
$(OBJ)/%.o: SIMD_Parallel/%.c | $(OBJ) ; $(CC) $(CFLAGS) $(AVX512) -c $< -o $@

$(OBJ): ; mkdir -p $@

# Headers the pattern rules do not see: rebuild the VIMA objects when they change
$(filter $(OBJ)/%_vima.o $(OBJ)/vima_isa.o,$(OBJS)): VIMA/vima_common.h VIMA/scan_common.h

clean: ; rm -rf $(OBJ) build/scan_main.trace

.PHONY: all clean
