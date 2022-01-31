/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
#
*/

// Mixer test
// Same as test6 but using thread switching thus not required to do a RotateThreadReadyQueue() at critical loop regions, using EEUG advices.

#include <stdio.h>
#include <string.h>
#include <kernel.h>
#include <sifrpc.h>
#include <loadfile.h>
#include <tamtypes.h>
#include "..\..\sjpcm.h"

#include <graph_registers.h>

#include "..\..\cosmitoFileIO.h"

#include "..\..\mixer.h"
#include "..\..\mixer_thread.h"

extern unsigned char isjpcm[];
extern unsigned int size_isjpcm;

int sample1Size;
int sample2Size;
char* sample1;
char* sample2;

int m_mixer_playing;		// status : If == 0, mixer is not playing : currently not working


#define THREAD_STACK_SIZE   (8 * 1024)
#define PAL_HSYNC_PERIOD    625             // not valid for NTSC
#define N_THREADS           2               // Number of threads running : 1 for mixer + 1 for dispatcher

static ee_thread_t thread_thread;

static u8          disp_stack[THREAD_STACK_SIZE] ALIGNED(16);
static int         disp_threadid;


void dispatcher ( void* apParam )
{
    while ( 1 )
    {
        SleepThread (); 
    }
}


void alarmfunction(s32 id, u16 time, void *arg)
{
    iWakeupThread ( disp_threadid );
    iRotateThreadReadyQueue ( 30 );             // see end of file
    iSetAlarm ( PAL_HSYNC_PERIOD/N_THREADS, alarmfunction, NULL );
} 


///
int main(int argc, char **argv)
{
    extern void *_gp;
	int ret;
	FILE *wav;
	sample1Size = 0;
	sample2Size = 0;
	sample1 = NULL;
	sample2 = NULL;

	SifInitRpc(0); 

    // EEUG : pay attention at the priority of your 'main' thread. 
    // Make sure that it is higher than threads you create 
    //(i.e. priority value is lower than 0x1E). I think ps2link sets it to 64 
    // - set the priority of your main thread to '0x1D': 
    // - create/start your "vegetables counters", set alarm handlers etc.; 
    // - lower priority of your 'main' thread 
    ChangeThreadPriority (  GetThreadId (), 29  );   // 29 is lower than 30 ;)

    printf("\ncreating dispatcher thread\n");
    thread_thread.func = dispatcher;
    thread_thread.stack = disp_stack;
    thread_thread.stack_size = THREAD_STACK_SIZE;
    thread_thread.gp_reg = &_gp;
    thread_thread.initial_priority = 0;
    StartThread (   disp_threadid = CreateThread ( &thread_thread ), NULL  );
    SetAlarm( PAL_HSYNC_PERIOD/N_THREADS, alarmfunction, NULL);

    ChangeThreadPriority ( GetThreadId (), 30 );

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

	//ChangeThreadPriority ( GetThreadId (), 42 );

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
		//RotateThreadReadyQueue(30);       // no longer needed
	}

	// end

    Mixer_Terminate();

    TerminateThread(disp_threadid);
    DeleteThread(disp_threadid);

	SjPCM_Setvol(0);
	SjPCM_Quit();
	free(sample1);
	free(sample2);
	printf("end\n");
	return 0;
}

// EEUG : Alternatively iRotateThreadReadyQueue can be moved into the dispatcher and replaced by RotateThreadReadyQueue.
