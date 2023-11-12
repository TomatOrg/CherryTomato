
# The platform we are on
PLATFORM := esp32

# TODO: configure
SRCS 		+= $(TARGET_DIR)/target.c

# Add drivers
SRCS 		+= src/drivers/axp202/axp202.c
SRCS 		+= src/drivers/st7789/st7789.c

all: $(BIN_DIR)/firmware.bin

# Quick way to run the firmware without flashing it
run: $(BIN_DIR)/firmware.bin
	sudo esptool --chip esp32 --before default_reset --no-stub write_flash 4096 out/bin/$(TARGET)/firmware.bin
	sudo esptool --chip esp32 --before default_reset --no-stub run
	sudo picocom -b 115200 /dev/ttyUSB0

#QEMU := /home/tomato/checkouts/qemu/build/qemu-system-xtensa
QEMU := out/toolchain/qemu/bin/qemu-system-xtensa

qemu: $(QEMU) $(BIN_DIR)/firmware.bin
	rm -rf $(OUT_DIR)/image.bin.full
	dd if=$(BIN_DIR)/firmware.bin of=$(OUT_DIR)/image.bin.full bs=1 seek=4096
	truncate -s 16M $(OUT_DIR)/image.bin.full
	$(QEMU) \
		 --trace "*i2c*" -machine esp32 \
		-serial stdio \
		-drive file=$(OUT_DIR)/image.bin.full,if=mtd,format=raw

fetch-qemu:
	mkdir -p $(OUT_DIR)/toolchain
	cd $(OUT_DIR)/toolchain && wget -N https://github.com/espressif/qemu/releases/download/esp-develop-8.1.2-20231017/esp-qemu-xtensa-softmmu-develop_8.1.2_20231017-x86_64-linux-gnu.tar.bz2
	cd $(OUT_DIR)/toolchain && tar -xvf esp-qemu-xtensa-softmmu-develop_8.1.2_20231017-x86_64-linux-gnu.tar.bz2
