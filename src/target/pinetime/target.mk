
# The platform we are on
PLATFORM := nrf52832

# TODO: configure
SRCS 		+= $(TARGET_DIR)/target.c

# Add drivers
# TODO

all: $(BIN_DIR)/firmware.bin
