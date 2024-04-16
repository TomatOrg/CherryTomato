########################################################################################################################
# Cherry Tomato
########################################################################################################################

#
# The target to compile for:
#	- ttgo-twatch-2020-v2
#	- um
#
TARGET 			?= um

#
# Do we have debug code enabled
# 	y/n
#
DEBUG			?= y

#-----------------------------------------------------------------------------------------------------------------------
# Build flags
#-----------------------------------------------------------------------------------------------------------------------

OUT_DIR			:= out

# The compiler flags
CFLAGS 			:= -Os -g3 -fno-tree-vectorize -flto
CFLAGS 			+= -Wall -Werror
CFLAGS 			+= -Wno-unused-label -Wno-unused-function
CFLAGS 			+= -fstrict-volatile-bitfields
CFLAGS 			+= -Isrc
CFLAGS 			+= -DPRINTF_SUPPORT_DECIMAL_SPECIFIERS=0
CFLAGS 			+= -DPRINTF_SUPPORT_EXPONENTIAL_SPECIFIERS=0
CFLAGS 			+= -DSUPPORT_MSVC_STYLE_INTEGER_SPECIFIERS=0
CFLAGS 			+= -DPRINTF_SUPPORT_WRITEBACK_SPECIFIER=0

# The linker script flags
LDFLAGS			:=

# The common source code
SRCS 			:=
SRCS 			+= src/apps/entry.c
SRCS 			+= src/task/lock.c
SRCS 			+= src/task/event.c
SRCS 			+= src/task/timer.c
SRCS 			+= src/task/tpl.c

SRCS 			+= src/util/printf.c

SRCS			+= src/util/libm/libm.c
SRCS			+= src/util/libm/ef_sqrt.c

#-----------------------------------------------------------------------------------------------------------------------
# Target configurations
#-----------------------------------------------------------------------------------------------------------------------

default: all

COMPILER_NAME		:= gcc
OUT_FILE_EXTENSION	:= elf

# Include the target
TARGET_DIR 		:= src/target/$(TARGET)
BIN_DIR			:= $(OUT_DIR)/bin/$(TARGET)
BUILD_DIR		:= $(OUT_DIR)/build/$(TARGET)
include $(TARGET_DIR)/target.mk

# Include platform stuff
PLATFORM_DIR	:= src/platform/$(PLATFORM)
CFLAGS 			+= -I$(PLATFORM_DIR)
include $(PLATFORM_DIR)/platform.mk

# Include arch stuff
ARCH_DIR		:= src/arch/$(ARCH)
CFLAGS  		+= -I$(ARCH_DIR)
include $(ARCH_DIR)/arch.mk

# If the target/platform provide a console then set it
ifneq ($(HAS_CONSOLE),0)
CFLAGS += -D__HAS_CONSOLE__
endif

ifeq ($(DEBUG),y)
CFLAGS += -D__DEBUG__
else
CFLAGS += -DNDEBUG
endif

#-----------------------------------------------------------------------------------------------------------------------
# Binaries
#-----------------------------------------------------------------------------------------------------------------------

CC				:= $(CROSS_COMPILER)$(COMPILER_NAME)
OBJCOPY			:= $(CROSS_COMPILER)objcopy

#-----------------------------------------------------------------------------------------------------------------------
# Generic Targets
#-----------------------------------------------------------------------------------------------------------------------

OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:%.o=%.d)
BINS ?=

-include $(DEPS)

# The elf output, this is not always the last step, so put it in the build dir instead
$(BUILD_DIR)/firmware.$(OUT_FILE_EXTENSION): $(OBJS)
	@echo LD $@
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

# Compile c/asm files
$(BUILD_DIR)/%.c.o: %.c
	@echo CC $@
	@mkdir -p $(@D)
	$(CC) -Wall $(CFLAGS) -MMD -c $< -o $@

$(BUILD_DIR)/%.S.o: %.S
	@echo CC $@
	@mkdir -p $(@D)
	$(CC) -Wall $(CFLAGS) -MMD -c $< -o $@


clean:
	rm -rf $(BIN_DIR) $(BUILD_DIR)
