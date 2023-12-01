CC 			   ?= gcc
LDFLAGS  	   += -lSDL2
ifeq ($(WASM), 1)
CFLAGS		   += -sUSE_SDL=2
CC 			   := emcc
LDFLAGS		   += -sSINGLE_FILE  -sASYNCIFY=1
endif