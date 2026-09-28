# Builds fm_${{ values.module }}.so for the BRM 15 Connection Manager.
#
#   make        build/lib/fm_${{ values.module }}.so and build/pin.conf.d/
#   make test   unit tests of src/logic/ (no CM needed)
#   make lint   clang-format and cppcheck
#   make clean
#
# Runs in the BRM SDK image (see the Dockerfile), where PIN_HOME holds the SDK
# headers and libraries.

MODULE   := fm_${{ values.module }}
PIN_HOME ?= /oms
BUILD    := build

CC     ?= gcc
CFLAGS ?= -O2 -g
# -isystem: warnings in the SDK headers are not ours to fix.
# -Wno-unused-parameter: every opcode handler has the same signature, used or not.
CFLAGS += -std=gnu11 -fPIC -Wall -Wextra -Wno-unused-parameter -Werror \
	-Iinclude -Isrc -isystem $(PIN_HOME)/include

SRCS       := $(wildcard src/*.c src/*/*.c)
HDRS       := $(wildcard include/*.h src/*/*.h)
LOGIC_SRCS := $(wildcard src/logic/*.c)
TEST_SRCS  := $(wildcard test/unit/*.c)
CONFS      := $(wildcard conf/pin.conf.d/*.conf)

OBJS  := $(SRCS:%.c=$(BUILD)/obj/%.o)
TESTS := $(TEST_SRCS:test/unit/%.c=$(BUILD)/test/%)

.PHONY: all test lint clean

all: $(BUILD)/lib/$(MODULE).so $(CONFS:conf/%=$(BUILD)/%)

# CM and libportal symbols stay undefined: the CM provides them when it loads the FM.
$(BUILD)/lib/$(MODULE).so: $(OBJS)
	@mkdir -p $(@D)
	$(CC) -shared -o $@ $^

$(BUILD)/obj/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -MMD -MP -c -o $@ $<

$(BUILD)/pin.conf.d/%: conf/pin.conf.d/%
	@mkdir -p $(@D)
	cp $< $@

$(BUILD)/test/%: test/unit/%.c $(LOGIC_SRCS) $(HDRS)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -o $@ $< $(LOGIC_SRCS) -L$(PIN_HOME)/lib -lportal -lcmocka

test: $(TESTS)
	@for t in $(TESTS); do \
		echo "==> $$t"; \
		LD_LIBRARY_PATH=$(PIN_HOME)/lib $$t || exit 1; \
	done

lint:
	clang-format --dry-run --Werror $(SRCS) $(HDRS) $(TEST_SRCS)
	cppcheck --quiet --error-exitcode=1 --enable=warning,performance,portability \
		--suppress=missingIncludeSystem -Iinclude -Isrc src

clean:
	rm -rf $(BUILD)

-include $(OBJS:.o=.d)
