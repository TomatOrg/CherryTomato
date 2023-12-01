
PLATFORM := um

SRCS 		+= $(TARGET_DIR)/target.c

ifeq ($(WASM), 1)
OUT_FILE 	   := $(BUILD_DIR)/firmware.html
else
OUT_FILE 	   := $(BUILD_DIR)/firmware
endif

all: $(OUT_FILE)

run: $(OUT_FILE)
	$(OUT_FILE)