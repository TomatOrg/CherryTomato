# no cross compiler, that's what native is
CROSS_COMPILER 	:=

# SDL for the emulation
CC 			   ?= gcc
LDFLAGS  	   += -lSDL2 -lm
ifeq ($(WASM), 1)
CFLAGS		   += -sUSE_SDL=2
CC 			   := emcc
LDFLAGS		   += -sSINGLE_FILE
endif
