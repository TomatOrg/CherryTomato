
# The platform we are on
PLATFORM := esp32

# TODO: configure
SRCS 		+= $(TARGET_DIR)/target.c

# Add drivers
SRCS 		+= src/drivers/st7789/st7789.c
SRCS 		+= src/drivers/ft6x06/ft6x06.c

all: $(BIN_DIR)/firmware.bin
