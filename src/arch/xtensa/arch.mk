CROSS_COMPILER				:= $(OUT_DIR)/toolchain/xtensa-esp-elf/bin/xtensa-esp32-elf-

# Some arch specific flags
CFLAGS 	+= -mauto-litpools
CFLAGS 	+= -mconst16
CFLAGS 	+= -mforce-no-pic
CFLAGS 	+= -mtarget-align
CFLAGS 	+= -mlongcalls

SRCS 	+= $(ARCH_DIR)/vectors.c
SRCS 	+= $(ARCH_DIR)/vectors.S

fetch-toolchain:
	mkdir -p $(OUT_DIR)/toolchain
	cd $(OUT_DIR)/toolchain && wget -N https://github.com/espressif/crosstool-NG/releases/download/esp-13.2.0_20230928/xtensa-esp-elf-13.2.0_20230928-x86_64-linux-gnu.tar.xz
	cd $(OUT_DIR)/toolchain && tar -xvf xtensa-esp-elf-13.2.0_20230928-x86_64-linux-gnu.tar.xz
