CROSS_COMPILER := $(abspath $(OUT_DIR))/toolchain/xtensa-esp-elf/bin/xtensa-esp32-elf-

# Some arch specific flags
CFLAGS 	+= -static
CFLAGS 	+= -mauto-litpools
CFLAGS 	+= -mforce-no-pic
CFLAGS 	+= -mtarget-align
CFLAGS 	+= -mlongcalls
CFLAGS 	+= -mabi=windowed
CFLAGS 	+= -fno-pie -fno-pic -ffreestanding
CFLAGS 	+= -nostartfiles -nostdlib -nodefaultlibs
CFLAGS 	+= -fno-stack-check -fno-stack-protector -fomit-frame-pointer
CFLAGS 	+= -flto -fuse-linker-plugin -fno-fat-lto-objects
CFLAGS 	+= -fno-exceptions -fno-unwind-tables -fno-asynchronous-unwind-tables


SRCS 	+= $(ARCH_DIR)/vectors.c
SRCS 	+= $(ARCH_DIR)/vectors.S
SRCS 	+= $(ARCH_DIR)/builtins.c
SRCS 	+= $(ARCH_DIR)/builtins.S

fetch-toolchain:
	mkdir -p $(OUT_DIR)/toolchain
	cd $(OUT_DIR)/toolchain && wget -N https://github.com/espressif/crosstool-NG/releases/download/esp-13.2.0_20230928/xtensa-esp-elf-13.2.0_20230928-x86_64-linux-gnu.tar.xz
	cd $(OUT_DIR)/toolchain && tar -xvf xtensa-esp-elf-13.2.0_20230928-x86_64-linux-gnu.tar.xz
