# Builds a customized BRM 15 policy library for the Connection Manager.
#
#   make        build/lib/<library>.so: Oracle's stock source, routed by overrides.conf
#   make test   unit tests of src/logic/ (no CM needed)
#   make lint   clang-format and cppcheck, on our code only
#   make clean
#
# Runs in the BRM SDK image (see the Dockerfile), where PIN_HOME holds the SDK
# headers, libraries and stock policy source. The stock source is copied into
# build/stage/ at build time and never committed.

PIN_HOME ?= /oms
BUILD    := build
STAGE    := $(BUILD)/stage

CC     ?= gcc
CFLAGS ?= -O2 -g
CFLAGS += -std=gnu11 -fPIC -isystem $(PIN_HOME)/include
# Our code is held to a stricter standard than the stock source.
# -Wno-unused-parameter: every opcode handler has the same signature, used or not.
STRICT_CFLAGS := -Wall -Wextra -Wno-unused-parameter -Werror -Isrc
# Extra flags the stock source may need. If it does not compile, copy them from
# the stock Makefile in $(PIN_HOME)/source/sys/<library>/.
STOCK_CFLAGS ?=

SRCS       := $(wildcard src/*/*.c)
HDRS       := $(wildcard src/*/*.h)
LOGIC_SRCS := $(wildcard src/logic/*.c)
TEST_SRCS  := $(wildcard test/unit/*.c)
TESTS      := $(TEST_SRCS:test/unit/%.c=$(BUILD)/test/%)

.PHONY: all stage library test lint clean

all: stage
	@$(MAKE) --no-print-directory library LIBRARY=$$(cat $(STAGE)/LIBRARY)

stage:
	bash scripts/route-overrides.sh overrides.conf $(STAGE)

ifdef LIBRARY
STOCK_SRCS := $(wildcard $(STAGE)/*.c)
OBJS := $(STOCK_SRCS:$(STAGE)/%.c=$(BUILD)/obj/stock/%.o) $(SRCS:%.c=$(BUILD)/obj/%.o)

library: $(BUILD)/lib/$(LIBRARY).so

# The CM looks up functions by name only when it loads the library, so check
# here that every routed wrapper and the stock function it wraps are defined.
$(BUILD)/lib/$(LIBRARY).so: $(OBJS)
	@mkdir -p $(@D)
	$(CC) -shared -o $@ $^
	@defined=$$(nm -D --defined-only $@ | awk '{ print $$3 }'); \
	while read -r opcode stock wrapper; do \
		for fn in $$stock $$wrapper; do \
			if ! printf '%s\n' "$$defined" | grep -qx "$$fn"; then \
				echo "$@ does not define $$fn ($$opcode)" >&2; rm -f $@; exit 1; \
			fi; \
		done; \
	done < $(STAGE)/ROUTED

$(BUILD)/obj/stock/%.o: $(STAGE)/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(STOCK_CFLAGS) -I$(STAGE) -c -o $@ $<
endif

# -I$(STAGE): wrappers include the generated policy_ops.h.
$(BUILD)/obj/%.o: %.c $(HDRS)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(STRICT_CFLAGS) -I$(STAGE) -c -o $@ $<

$(BUILD)/test/%: test/unit/%.c $(LOGIC_SRCS) $(HDRS)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(STRICT_CFLAGS) -o $@ $< $(LOGIC_SRCS) -L$(PIN_HOME)/lib -lportal -lcmocka

test: $(TESTS)
	@for t in $(TESTS); do \
		echo "==> $$t"; \
		LD_LIBRARY_PATH=$(PIN_HOME)/lib $$t || exit 1; \
	done

lint:
	clang-format --dry-run --Werror $(SRCS) $(HDRS) $(TEST_SRCS)
	cppcheck --quiet --error-exitcode=1 --enable=warning,performance,portability \
		--suppress=missingIncludeSystem -Isrc src

clean:
	rm -rf $(BUILD)
