# 
# PixelKey Unit Tests Makefile
# Snippets taken from https://makefiletutorial.com/
#
# !IMPORTANT! This must be run from the top directory using `make -f test/makefile`.
#

TARGET_BIN := pixelkey_test

DEFINES := DEBUG=1
DEFINES += _RENESAS_RA_

WARNINGS := unused uninitialized all extra missing-declarations conversion pointer-arith shadow logical-op aggregate-return missing-prototypes 

BUILD_DIR := ./test/build

TESTS_SRC := ./test/tests
PIXELKEY_SRC := ./src/src
UNITY_SRC := ./test/unity

INCLUDE_PATHS := ./test $(UNITY_SRC)/src $(UNITY_SRC)/extras/fixture/src $(UNITY_SRC)/extras/memory/src
INCLUDE_PATHS += $(PIXELKEY_SRC) $(PIXELKEY_SRC)/pixelkey $(PIXELKEY_SRC)/pixelkey/keyframes

INCLUDE_FLAGS := $(addprefix -I,$(INCLUDE_PATHS))

SRCS := ./test/pixelkey_test.c 
SRCS += $(shell find $(TESTS_SRC) -iname '*.c')
SRCS += $(shell find $(PIXELKEY_SRC)/pixelkey -iname '*.c')
SRCS += $(UNITY_SRC)/src/unity.c $(UNITY_SRC)/extras/fixture/src/unity_fixture.c $(UNITY_SRC)/extras/memory/src/unity_memory.c

# Prepends BUILD_DIR and appends .o to every src file
OBJS := $(SRCS:%.c=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)


CC := gcc
CPPFLAGS := $(INCLUDE_FLAGS) -MMD -MP
CFLAGS := -O1 -fsigned-char -fsingle-precision-constant -g -std=gnu17
CFLAGS += $(addprefix -W,$(WARNINGS))
CFLAGS += $(addprefix -D,$(DEFINES))
LDFLAGS := -lm

$(BUILD_DIR)/$(TARGET_BIN): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LDFLAGS) -o $@

$(BUILD_DIR)/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

.PHONY: check
check:
	@echo SRCS: $(SRCS)
	@echo OBJS: $(OBJS)

.PHONY: clean
clean:
	rm -fr $(BUILD_DIR)

.PHONY: rebuild
rebuild: clean $(BUILD_DIR)/$(TARGET_BIN)

.PHONY: run
run: $(BUILD_DIR)/$(TARGET_BIN)
	$(BUILD_DIR)/$(TARGET_BIN) -v


# Include the .d makefiles. The - at the front suppresses the errors of missing
# Makefiles. Initially, all the .d files will be missing, and we don't want those
# errors to show up.
-include $(DEPS)
