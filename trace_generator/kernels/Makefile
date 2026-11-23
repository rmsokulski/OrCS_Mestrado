KERNELS := $(patsubst %/Makefile,%,$(wildcard */Makefile))

all clean:
	@for k in $(KERNELS); do $(MAKE) -C $$k $@ || exit 1; done

.PHONY: all clean
