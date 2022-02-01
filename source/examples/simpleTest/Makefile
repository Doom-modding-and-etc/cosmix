EE_BIN = test.elf
EE_OBJS = ../../cosmitoFileIO.o ../../mixer_thread.o ../../mixer.o test6.o ../../sjpcm_rpc.o #isjpcm.s
#EE_CFLAGS = -g
EE_INCS = -I$(PS2DEV)/isjpcm/include/ 
EE_LDFLAGS =  -L$(PS2SDK)/ee/lib/ -L$(PS2DEV)/isjpcm/lib

EE_LIBS = -lsjpcm -lmixer

all: $(EE_BIN)

clean:
	rm -f *.elf *.o

#isjpcm.s : isjpcm.irx
#	bin2s isjpcm.irx isjpcm.s isjpcm

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
