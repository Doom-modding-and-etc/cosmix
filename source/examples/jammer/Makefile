EE_BIN = jammerPS2.elf
EE_OBJS = ../../cosmitoFileIO.o ../../mixer_thread.o ../../mixer.o jammer.o ../../sjpcm_rpc.o usbd.s usbhdfsd.s freesd.s SJPCM.s
EE_INCS = -I$(PS2DEV)/isjpcm/include/ 
EE_LDFLAGS = -L$(PS2SDK)/ee/lib/ -L$(PS2DEV)/isjpcm/lib
EE_LIBS = -lpad -ldebug -lmixer -lsjpcm

all: $(EE_BIN)

clean:
	rm -f *.elf *.o *.s
	rm -f ../../*.o

usbd.s : ./IRX/usbd.irx
	bin2s ./IRX/usbd.irx usbd.s usbd

usbhdfsd.s : ./IRX/usbhdfsd.irx
	bin2s ./IRX/usbhdfsd.irx usbhdfsd.s usbhdfsd

freesd.s : ../../IRX/freesd-1.01.irx
	bin2s ../../IRX/freesd-1.01.irx freesd.s freesd
	
SJPCM.s : ../../IRX/SJPCM.IRX
	bin2s ../../IRX/SJPCM.IRX SJPCM.s SJPCM
	
include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
