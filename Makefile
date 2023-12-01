########################################################################################################################
# Cherry Tomato
########################################################################################################################

#
# The target to compile for:
#	- ttgo-twatch-2020-v2
#	- um
#
TARGET 			?= ttgo-twatch-2020-v2

#-----------------------------------------------------------------------------------------------------------------------
# Build flags
#-----------------------------------------------------------------------------------------------------------------------

OUT_DIR			:= out

# The compiler flags
CFLAGS 			:= -Os
CFLAGS 			+= -Wall -Werror
CFLAGS 			+= -Wno-unused-label -Wno-unused-function
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
ifneq ($(TARGET), um)
SRCS 			+= src/task/time.c
endif
SRCS 			+= src/util/printf.c

SRCS			+= src/util/libm/libm.c
SRCS			+= src/util/libm/ef_sqrt.c

SRCS			+= src/apps/watch/timer.c
SRCS			+= src/apps/watch/roundedrect.c
SRCS			+= src/apps/watch/alarmdone.c
SRCS			+= src/apps/watch/applist.c
SRCS			+= src/apps/watch/main.c
SRCS			+= src/apps/watch/gesturerecognizer.c
SRCS			+= src/apps/watch/thumbnail.c
SRCS			+= src/apps/watch/plat.c
SRCS			+= src/apps/watch/messagelist.c
SRCS			+= src/apps/watch/ui.c
SRCS			+= src/apps/watch/messagestore.c
SRCS			+= src/apps/watch/watchface.c
SRCS			+= src/apps/watch/physics.c
SRCS			+= src/apps/watch/text.c
SRCS			+= src/apps/watch/fullmessage.c

#-----------------------------------------------------------------------------------------------------------------------
# Target configurations
#-----------------------------------------------------------------------------------------------------------------------

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

#-----------------------------------------------------------------------------------------------------------------------
# Binaries
#-----------------------------------------------------------------------------------------------------------------------

CC				?= $(CROSS_COMPILER)gcc

#-----------------------------------------------------------------------------------------------------------------------
# Generic Targets
#-----------------------------------------------------------------------------------------------------------------------

OBJS := $(SRCS:%=$(BUILD_DIR)/%.o) $(BUILD_DIR)/res/fonts.o
DEPS := $(OBJS:%.o=%.d)
BINS ?=

-include $(DEPS)

# The elf output, this is not always the last step, so put it in the build dir instead
$(OUT_FILE): $(OBJS)
	@echo LD $@
	@mkdir -p $(@D)
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^

# Compile c/asm files
$(BUILD_DIR)/%.c.o: %.c
	@echo CC $@
	@mkdir -p $(@D)
	$(CC) -Wall $(CFLAGS) -MMD -c $< -o $@

$(BUILD_DIR)/%.S.o: %.S
	@echo CC $@
	@mkdir -p $(@D)
	$(CC) -Wall $(CFLAGS) -MMD -c $< -o $@

$(BUILD_DIR)/res/fonts.o: $(BUILD_DIR)/res/BebasNeue.ttf $(BUILD_DIR)/res/Roboto.ttf
	@mkdir -p $(@D)
	@python src/scripts/fontconv.py $(BUILD_DIR)/res
	@cd $(BUILD_DIR)/res/; xxd -i _roboto > fonts.c
	@cd $(BUILD_DIR)/res/; xxd -i _bebas1 >> fonts.c
	@cd $(BUILD_DIR)/res/; xxd -i _bebas2 >> fonts.c
	@cd $(BUILD_DIR)/res/; xxd -i _bebas3 >> fonts.c
	@cd $(BUILD_DIR)/res/; xxd -i _bebas4 >> fonts.c
	$(CC) -Wall $(CFLAGS) -MMD -c $(BUILD_DIR)/res/fonts.c -o $(BUILD_DIR)/res/fonts.o

$(BUILD_DIR)/res/BebasNeue.ttf:
	@mkdir -p $(@D)
	curl https://cdn.jsdelivr.net/fontsource/fonts/bebas-neue@latest/latin-400-normal.ttf -o $@

$(BUILD_DIR)/res/Roboto.ttf:
	@mkdir -p $(@D)
	curl https://cdn.jsdelivr.net/fontsource/fonts/roboto@latest/latin-400-normal.ttf -o $@


clean:
	rm -rf $(BIN_DIR) $(BUILD_DIR)
