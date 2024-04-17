CROSS_COMPILER 	:=
COMPILER_NAME 	:= clang

# Some arch specific flags
CFLAGS		+= -static
CFLAGS 		+= -target arm-none-eabi
CFLAGS 		+= -fno-pie -fno-pic -ffreestanding
CFLAGS 		+= -nostdlib -nodefaultlibs
CFLAGS 		+= -fno-stack-check -fno-stack-protector -fomit-frame-pointer
CFLAGS 		+= -flto
CFLAGS 		+= -fno-exceptions -fno-unwind-tables -fno-asynchronous-unwind-tables

OUT_FILE 	:= $(BUILD_DIR)/firmware.elf

SRCS 		+=
