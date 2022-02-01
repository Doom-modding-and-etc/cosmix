EE_OBJS = source/cosmitoFileIO.o source/mixer_thread.o source/mixer.o source/sjpcm_rpc.o

EE_LIB = libmixer.a

BIN2S = $(PS2SDK)/bin/bin2s

EE_INCS = -I$(PS2DEV)/isjpcm/include/ 

EE_CFLAGS += -Wall -Os -I../include -I../include/

EE_LDFLAGS = -L$(PS2DEV)/isjpcm/lib/ -L$(PS2SDK)/ee/lib/

EE_LIBS = -lsjpcm 

EE_CFLAGS = -Wall 

all: $(EE_LIB) $(EE_OBJS)
	@echo Copying...
	@[ -d $(PS2SDK)/ee/include/mixer ] || mkdir -p $(PS2SDK)/ee/include/mixer
	@cp -frv source/include/*.h $(PS2SDK)/ee/include/mixer
	@cp -f $(EE_LIB) $(PS2SDK)/ee/lib
	@rm -f $(EE_LIB) $(EE_OBJS)
	@echo Done!

freesd.s: $(PS2SDK)/iop/irx/freesd.irx
	$(BIN2S) $< $@ IRX/freesd_irx
	 
isjpcm.s: $(PS2DEV)/isjpcm/bin/isjpcm.irx
	$(BIN2S) $< $@ IRX/isjpcm_irx

jammer:	
	$(MAKE) -C source/examples/jammer all

simpleTest:
	$(MAKE) -C source/examples/simpleTest all

simpleTest_with_thread_switching:
	$(MAKE) -C source/examples/simpleTest_with_thread_switching all

samples: jammer simpleTest simpleTest_with_thread_switching

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
