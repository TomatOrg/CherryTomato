
#
# The cross compiler to use
#
CROSS_COMPILER 		:= xtensa-lx106-elf-

#
# The arch we are using is xtensa
#
ARCH 				:= xtensa

# We have a UART console
HAS_CONSOLE			:= 1

# Add platform code
SRCS				+= $(PLATFORM_DIR)/bluetooth/bt.c
SRCS				+= $(PLATFORM_DIR)/hardware/gpio.c
SRCS				+= $(PLATFORM_DIR)/hardware/i2c.c
SRCS				+= $(PLATFORM_DIR)/hardware/spi.c
SRCS				+= $(PLATFORM_DIR)/entry.c
SRCS				+= $(PLATFORM_DIR)/entry.S

CFLAGS 				+= -T$(PLATFORM_DIR)/linker.lds

LIBS				+= $(PLATFORM_DIR)/bluetooth/libbtdm_app.a

# The main target is the firmware binary
all: $(BIN_DIR)/firmware.bin


# Build the firmware from the elf file
$(BIN_DIR)/firmware.bin: $(BUILD_DIR)/firmware.elf
	@echo ESPTOOL ELF2IMAGE $@
	@mkdir -p $(@D)
	@esptool --chip esp32 --trace elf2image --output $@ --version 3 --min-rev 3 --flash_freq 40m --flash_mode dio --flash_size 16MB $^
