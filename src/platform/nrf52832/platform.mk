#
# The arch we are using is xtensa
#
ARCH 				:= armv7-m

# We have a UART console
HAS_CONSOLE			:= 0

# Add platform code
SRCS				+= $(PLATFORM_DIR)/entry.c

# Choose the exact cpu we have
CFLAGS 				+= -march=armv7e-m -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb

# Add the linker script, will build the image for us
LDFLAGS 			+= -T$(PLATFORM_DIR)/linker.lds

# Build the firmware from the elf file
$(BIN_DIR)/firmware.bin: $(BUILD_DIR)/firmware.elf
	mkdir -p $(@D)
	llvm-objcopy  -j .text -j .rodata -j .data -j .bss -O binary $^ $@