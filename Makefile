MAKEFLAGS += -j2 -rR --include-dir=$(CURDIR)
executable = plot

ifeq (release,$(MAKECMDGOALS))
  target := release
  MAKECMDGOALS := all
endif
ifeq (debug,$(MAKECMDGOALS))
  target := debug
  MAKECMDGOALS := all
endif

target ?= release
TARGET ?= $(target)
target_dir := target/$(TARGET)
objects := $(patsubst src/%.c,$(target_dir)/%.o,$(wildcard src/*.c))

debug_cflags := -g -D DEBUG
release_cflags := -O2
CFLAGS += $($(TARGET)_cflags)
CC = gcc

.PHONY: all debug release
all debug release: $(target_dir)/$(executable)

$(target_dir):
	mkdir -p $(target_dir)

.PRECIOUS: %.o
.SECONDEXPANSION:
%.o: src/$$(basename $$(notdir $$@)).c | $(target_dir)
	$(CC) $(CFLAGS) -o $*.o -c $(subst $(target_dir),src,$*).c

%/$(executable): $(objects)
	$(CC) $(CFLAGS) -lm -o $*/$(executable) $(objects)

.PHONY: clean
clean:
	rm -rf target
