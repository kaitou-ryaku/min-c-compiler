CC      := gcc
CFLAGS  := -std=c99 -O0 -Wall

TARGET  := minc.out
ALL_C   := $(wildcard src/*.c)
ALL_CH  := $(wildcard src/*.c include/*.h)
ALL_T_C := $(wildcard utest/PART_*.c)
ALL_T_O := $(patsubst utest/PART_%.c,utest/complete/TEST_%.out,$(ALL_T_C))
ALL_O   := $(patsubst src/%.c,object/%.o,$(ALL_C))

minc.out: $(ALL_CH) $(ALL_T_O)
	@cd object && $(MAKE) "CC=$(CC)" "CFLAGS=$(CFLAGS)"
	$(CC) $(CFLAGS) $(ALL_O) -o $(TARGET)

notest:   $(ALL_CH)
	@cd object && $(MAKE) "CC=$(CC)" "CFLAGS=$(CFLAGS)"
	$(CC) $(CFLAGS) $(ALL_O) -o $(TARGET)

utest/complete/%.out: $(ALL_CH) $(ALL_T_C)
	@cd utest/complete && $(MAKE) $(subst utest/complete/,,$@) "CC=$(CC)" "CFLAGS=$(CFLAGS)"

.PHONY: clean
clean:
	@rm -rf *.out *.stackdump tmp* object/*.o object/*.d utest/complete/TEST_*
