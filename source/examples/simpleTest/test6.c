/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
#
# Based on ps2sdk audsrv sample
*/

// Mixer test

#include <stdio.h>
#include <string.h>
#include <kernel.h>
#include <sifrpc.h>
#include <loadfile.h>
#include <tamtypes.h>
#include <sjpcm.h>

#include <graph.h>

#include <mixer/wav.h>

#include <mixer/mixer.h>
#include <mixer/mixer_thread.h>

extern unsigned char isjpcm[];
extern unsigned int size_isjpcm;

int ofs1;		// offset sample 1
int ofs2;
int sample1Size;
int sample2Size;
char* sample1;
char* sample2;

int m_mixer_playing;		// status : If == 0, mixer is not playing : currently not working
s32 main_thread_id;

//static int debug1;

///
int main(int argc, char **argv)
{
	int ret;
	FILE *wav; //, *dumpL, *dumpR;
    //debug1 = 0;
	ofs1 = 0;
	ofs2 = 10;
	sample1Size = 0;
	sample2Size = 0;
	sample1 = NULL;
	sample2 = NULL;

	//m_mixer_playing = 0;
    //count = 0;
    main_thread_id = GetThreadId ();

	SifInitRpc(0); 

	printf("sample: kicking IRXs\n");
    //ret = SifLoadModule("host:LIBSD.irx", 0, NULL);
	ret = SifLoadModule("rom0:LIBSD", 0, NULL);
	printf("libsd loadmodule %d\n", ret);

	//printf("sample: loading isjpcm embedded module\n");	
	//SifExecModuleBuffer(isjpcm, size_isjpcm, 0, NULL, &ret);
	//printf("isjpcm loadmodule %d\n", ret);

	ret = SifLoadModule("host:SJPCM.IRX", 0, NULL);
	if (ret < 0) 
	{
		printf("Failed to load module: SJPCM.IRX\n");
		SleepThread();
	}

	SjPCM_Init(1);		// sync mode

	printf("SjPCM_Setvol ...\n");
	SjPCM_Setvol(0x3fff);

	ChangeThreadPriority ( GetThreadId (), 42 );

	// Init mixer thread
    Mixer_Init();

	//---------------------------------------------------------------------------------
	// get sample1
	wav = fopen("host:sample1.wav", "rb");
	if (wav == NULL)
	{
		printf("failed to open host:sample1.wav file\n");
		SjPCM_Quit();
		return -1;
	}

	//Get filesize //
	sample1Size = GetFilesize(wav);
	sample1Size = sample1Size - 0x30;		// discard header lenght
	fseek(wav, 0x30, SEEK_SET);			    // discard header itself

	printf("samplesize = %d\n", sample1Size);

	if (!(sample1 = malloc(sample1Size)))			// TBD : padding
	{
		fclose(wav);
		printf("Error: Couldnt allocate buffer\n");
		SjPCM_Quit();
		return -1;
	}

	if (!(fread(sample1, 1, sample1Size, wav)))
	{
		fclose(wav);
		printf("Error: Cannot read file\n");
		free(sample1);
		SjPCM_Quit();
		return -1;
	}
	fclose(wav);

    //---------------------------------------------------------------------------------
	// get sample2
	wav = fopen("host:sample2.wav", "rb");
	if (wav == NULL)
	{
		printf("failed to open host:sample2.wav wav file\n");
		SjPCM_Quit();
		return -1;
	}

	//Get filesize //
	sample2Size = GetFilesize(wav);
	sample2Size = sample2Size - 0x30;		// discard header lenght
	fseek(wav, 0x30, SEEK_SET);			// discard header itself

	printf("sample2size = %d\n", sample2Size);

	if (!(sample2 = malloc(sample2Size)))			// TBD : padding
	{
		fclose(wav);
		printf("Error: Couldnt allocate buffer\n");
		SjPCM_Quit();
		return -1;
	}

	if (!(fread(sample2, 1, sample2Size, wav)))
	{
		fclose(wav);
		printf("Error: Cannot read file\n");
		free(sample1);
		free(sample2);
		SjPCM_Quit();
		return -1;
	}
	fclose(wav);
	//---------------------------------------------------------------------------------

	printf("starting play loop\n");


	// starts mixing
	m_mixer_playing = 1;

    // Start playing both samples
    PlaySample((sint16*)sample1, sample1Size, 128, 1);
    PlaySample((sint16*)sample2, sample2Size, 128, 1);

	while(IsPlaying()>0)
	{
		RotateThreadReadyQueue(42);
	}

	// end

    Mixer_Terminate();

	SjPCM_Setvol(0);
	SjPCM_Quit();
	free(sample1);
	free(sample2);
	printf("end\n");
	return 0;
}
