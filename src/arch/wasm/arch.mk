# no cross compiler, that's what native is
CROSS_COMPILER 		:=

OUT_FILE_EXTENSION 	:= html
COMPILER_NAME		:= emcc

CFLAGS		   		+= -sUSE_SDL=2
LDFLAGS  	   		+= -lSDL2
LDFLAGS		   		+= -sSINGLE_FILE

