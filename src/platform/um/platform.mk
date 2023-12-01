
CROSS_COMPILER 		:=

# The arch we are using is fake
ARCH 				:= um

# We have a (fake) UART console
HAS_CONSOLE			:= 1

# Add platform code
SRCS				+= $(PLATFORM_DIR)/entry.c

# The main target is the firmware binary
all: $(OUT_FILE)
