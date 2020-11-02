BIN_NAME    := render
TESTER_NAME := testrunner

BIN_FILES    := src/common.c src/main.c src/render.c
TESTER_FILES := src/common.c src/render.c src/unit_tests.c src/test_main.c
HEADERS      := $(wildcard include/*.h)

TEST_SCRIPT := test/run_tests.py
Q ?= @

DEBUG   := -O0 -g -fsanitize=address -fsanitize=undefined
OPT     := -O3

CFLAGS  += -I include -Wall -Wextra -pedantic
LDFLAGS += -lm

SDL2     = sdl2
CFLAGS  += $(shell pkg-config --cflags $(SDL2))
LDFLAGS += $(shell pkg-config --libs $(SDL2))

.PHONY: all check clean

all: bin/$(BIN_NAME)_opt bin/$(BIN_NAME)_debug bin/$(TESTER_NAME)

bin/$(BIN_NAME)_opt: $(patsubst src/%.c, build/%.opt.o, $(BIN_FILES))
	$(Q)mkdir -p $(@D)
	@echo "===> LD $@"
	$(Q)$(CC) -o $@ $(CFLAGS) $(OPT) $+ $(LDFLAGS)

bin/$(BIN_NAME)_debug: $(patsubst src/%.c, build/%.debug.o, $(BIN_FILES))
	$(Q)mkdir -p $(@D)
	@echo "===> LD $@"
	$(Q)$(CC) -o $@ $(CFLAGS) $(DEBUG) $+ $(LDFLAGS)

bin/$(TESTER_NAME): $(patsubst src/%.c, build/%.debug.o, $(TESTER_FILES))
	$(Q)mkdir -p $(@D)
	@echo "===> LD $@"
	$(Q)$(CC) -o $@ $(CFLAGS) $(DEBUG) $+ $(LDFLAGS)

build/%.opt.o: src/%.c $(HEADERS)
	$(Q)mkdir -p $(@D)
	@echo "===> CC $@"
	$(Q)$(CC) -o $@ -c $(CFLAGS) $(OPT) $<

build/%.debug.o: src/%.c $(HEADERS)
	$(Q)mkdir -p $(@D)
	@echo "===> CC $@"
	$(Q)$(CC) -o $@ -c $(CFLAGS) $(DEBUG) $<

check: bin/$(TESTER_NAME)
	@echo "===> CHECK"
	$(Q)$(TEST_SCRIPT)

clean:
	@echo "===> CLEAN"
	$(Q)rm -rf bin build

extract_maps.tgz:
	@make -C maps > /dev/null

all: extract_maps.tgz

extract_ref_images.tgz:
	@make -C test/ref_images > /dev/null

all: extract_ref_images.tgz

