
PLATFORM := um

SRCS 		+= $(TARGET_DIR)/target.c


all: $(BUILD_DIR)/firmware.elf


run: $(BUILD_DIR)/firmware.elf
	$(BUILD_DIR)/firmware.elf
