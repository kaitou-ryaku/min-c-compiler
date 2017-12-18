CC      := gcc
CFLAGS  := -std=c99 -O0 -Wall
CPP     := $(CC) $(CFLAGS) -E

ALL_PART_C   := $(wildcard ../PART_*.c)
ALL_TEST_C   := $(patsubst ../PART_%.c,TEST_%.c,$(ALL_PART_C))
ALL_TEST_OUT := $(patsubst %.c,%.out,$(ALL_TEST_C))
ALL_TEST_D   := $(patsubst %.c,%.d,$(ALL_TEST_C))

ALL_SRC_C    := $(subst ../../src/main.c,,$(wildcard ../../src/*.c))
ALL_OBJ_O    := $(patsubst ../../src/%.c,../../object/%.o,$(ALL_SRC_C))
ALL_INC_H    := $(wildcard ../../include/*.h)

.PHONY: dummy
dummy: $(ALL_TEST_OUT)

%.out: %.o
	@cd ../../object && $(MAKE) "CC=$(CC)" "CFLAGS=$(CFLAGS)"
	@$(CC) $(CFLAGS) $< $(patsubst ../../object/$(subst TEST_,,$<),,$(ALL_OBJ_O)) -o $@
	@./$@ || rm $@

%.o: %.c
	@$(CC) $(CFLAGS) -c $< -o $@

# -include $(ALL_C)で依存関係が上書きされる
TEST_%.c: ../PART_%.c
	@sh generate_test_c.sh $<

%.d: %.c $(ALL_SRC_C) $(ALL_INC_H)
	@$(CPP) -MM $< -MF $@

-include $(ALL_TEST_D)
