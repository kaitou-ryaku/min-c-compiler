CC      := gcc
CFLAGS  := -std=c99 -O0 -Wall

.PHONY: dummy
dummy:
	@cd complete && $(MAKE) "CC=$(CC)" "CFLAGS=$(CFLAGS)"

.PHONY: clean
clean:
	@rm -rf complete/TEST_* complete/tmp*
