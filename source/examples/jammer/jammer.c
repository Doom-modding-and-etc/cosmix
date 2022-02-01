// cosMix sound jammer example

#include <stdio.h>
#include <string.h>
#include <kernel.h>
#include <sifrpc.h>
#include <libpad.h>
#include <loadfile.h>
#include <tamtypes.h>
#include <graph.h>

// cosMix headers
#include <sjpcm.h>
#include <mixer/wav.h>
#include <mixer/mixer.h>
#include <mixer/mixer_thread.h>

extern unsigned char usbd[];
extern unsigned int size_usbd;

extern unsigned char usbhdfsd[];
extern unsigned int size_usbhdfsd;

extern unsigned char freesd[];
extern unsigned int size_freesd;

extern unsigned char SJPCM[];
extern unsigned int size_SJPCM;
static char s_pUDNL   [] __attribute__(   (  section( ".data" ), aligned( 1 )  )   ) = "rom0:UDNL rom0:EELOADCNF";

char* sampleTRIANGLE = NULL;
char* sampleCIRCLE = NULL;
char* sampleCROSS = NULL;
char* sampleSQUARE = NULL;
char* samplePAD_LEFT = NULL;
char* samplePAD_DOWN = NULL;
char* samplePAD_RIGHT = NULL;
char* samplePAD_UP = NULL;
char* samplePAD_L1 = NULL;
char* samplePAD_L2 = NULL;
char* samplePAD_R1 = NULL;
char* samplePAD_R2 = NULL;
char* samplePAD_SELECT = NULL;
char* samplePAD_START = NULL;

int sampleTRIANGLE_Size;
int sampleCIRCLE_Size;
int sampleCROSS_Size;
int sampleSQUARE_Size;
int samplePAD_LEFT_Size;
int samplePAD_DOWN_Size;
int samplePAD_RIGHT_Size;
int samplePAD_UP_Size;
int samplePAD_L1_Size;
int samplePAD_L2_Size;
int samplePAD_R1_Size;
int samplePAD_R2_Size;
int samplePAD_SELECT_Size;
int samplePAD_START_Size;

int channelTypeTRIANGLE = 0;
int channelTypeCIRCLE = 0; 
int channelTypeCROSS = 0;
int channelTypeSQUARE = 0; 
int channelTypePAD_LEFT = 0;
int channelTypePAD_DOWN = 0;
int channelTypePAD_RIGHT = 0;
int channelTypePAD_UP = 0; 
int channelTypePAD_L1 = 0; 
int channelTypePAD_L2 = 0; 
int channelTypePAD_R1 = 0; 
int channelTypePAD_R2 = 0; 
int channelTypePAD_START = 0;

int continueloop;

// pad_dma_buf is provided by the user, one buf for each pad
// contains the pad's current state
static char padBuf[256] __attribute__((aligned(64)));
static char actAlign[6];
static int actuators;

int loadWAV(char *filename, char* sample, int sample_Size);

void loadPadModules();
int waitPadReady(int, int);
int initializePad(int, int);
int padUtils_ReadButton(int port, int slot, u32 old_pad, u32 new_pad);
int padUtils_WaitButton(int port, int slot);

void GetElfFilename(const char *argv0_probe, char* deviceName, char* fullPath, char* elfFilename);

///
int main(int argc, char **argv)
{
	int ret, butres, prev_butres = 0, mixer_bufsize=0, size;
	FILE *wav;

    //Pad
    int port, slot;
    u32 old_pad = 0, new_pad = 0;

    //Reset PS2 (from uLaunchELF)  : needed for fopen and fread from mass: . Probabilly fioOpen and fioRead work without this (from usb_mass readme.txt)
    SifIopReset("rom0:UDNL rom0:EELOADCNF",0);
    while(!SifIopSync());
    fioExit();
    SifExitIopHeap();
    SifLoadFileExit();
    SifExitRpc();
    SifExitCmd();
    SifInitRpc(0);
    FlushCache(0);
    FlushCache(2);

    // TBD : try with this : http://forums.ps2dev.org/viewtopic.php?p=4861&highlight=#4861
    //
    //    void resetIOP() {
    //
    //   SifInitRpc(0);
    //   SifExitIopHeap();
    //   SifLoadFileExit();
    //   SifExitRpc();
    //
    //
    //   SifIopReset("rom0:UDNL rom0:EELOADCNF",0);
    //
    //   while (!SifIopSync()) ;
    //   SifInitRpc(0);
    //} 

    // Check for PAL or NTSC
    int PAL = from_gsKit_detect_signal();

    if (PAL == 1)
        mixer_bufsize = 960;
    else
        mixer_bufsize = 800;

    init_scr();

    char elfFilename[100];
    char deviceName[10];
    char fullPath[100];

    GetElfFilename(argv[0], deviceName, fullPath, elfFilename);

    scr_printf("Welcome to JammerPS2 v1.0, by cosmito 2009\n\n");
    scr_printf("------------------------------------------\n\n");
    scr_printf("Put some mono or stereo 48Khz (not 44Khz!) WAV along with %s before  starting Jammer.\n", elfFilename);
    scr_printf("USB pens/drives are supported. Then use the Dualshock pad 1 buttons to fire up  the sounds.\n\n");
    scr_printf("It will look any of the samples :\n");
    scr_printf("    TRIANGLE.wav   CIRCLE.wav     CROSS.wav\n");
    scr_printf("    SQUARE.wav     PAD_LEFT.wav   PAD_DOWN.wav\n");
    scr_printf("    PAD_RIGHT.wav  PAD_UP.wav     PAD_L1.wav\n");
    scr_printf("    PAD_L2.wav     PAD_R1.wav     PAD_R2.wav\n");
    scr_printf("    PAD_START.wav\n\n");
    scr_printf("Remember the PS2 is limited to 32MB of RAM, so don't push it...\n\n"); 

	//ret = SifLoadModule("host:LIBSD.irx", 0, NULL);
	//ret = SifLoadModule("rom0:LIBSD", 0, NULL);
	//if (ret<0)
    //{
    //  scr_printf("Failed to load module: rom0:LIBSD\n");
	//	SleepThread();
	//}
    
    // freesd
    ret = SifExecModuleBuffer(freesd, size_freesd, 0, NULL, &ret);
    printf("freesd loadmodule %d\n", ret);

    //Load embedded modules

    // USB mass support
    ret = SifExecModuleBuffer(usbd, size_usbd, 0, NULL, &ret);
    ret = SifExecModuleBuffer(usbhdfsd, size_usbhdfsd, 0, NULL, &ret);

    // SJPCM (requires freesd or libsd)
    SifExecModuleBuffer(SJPCM, size_SJPCM, 0, NULL, &ret);
    
	//ret = SifLoadModule("host:SJPCM.IRX", 0, NULL);
	//if (ret < 0) 
	//{
	//	scr_printf("Failed to load module: SJPCM.IRX\n");
	//	SleepThread();
	//}

    SjPCM_Init(1);		// sync mode

	printf("SjPCM_Setvol ...\n");
	SjPCM_Setvol(0x3fff);

	ChangeThreadPriority ( GetThreadId (), 42 );
    Mixer_Init();

    scr_printf("Loading samples from %s ...\n", fullPath);

    //---------------------------------------------------------------------------------
    // get sampleTRIANGLE
    char auxstr[255];
    strcpy(auxstr, fullPath);   strcat(auxstr, "TRIANGLE.wav");
  
    size = GetWAVsize(auxstr);
    if( size != 0)
    {
        scr_printf("    TRIANGLE.wav");
        int padextrasize = ((size / mixer_bufsize)+1)*mixer_bufsize;  // padding
        if (!(sampleTRIANGLE = malloc(padextrasize)))
        {
            printf("Error: Couldnt allocate buffer\n");
            return -1;
        }
        channelTypeTRIANGLE = loadWAV(auxstr, sampleTRIANGLE, size);
        sampleTRIANGLE_Size = padextrasize;
    }

    //---------------------------------------------------------------------------------
    // get sampleCIRCLE
        strcpy(auxstr, fullPath);   strcat(auxstr, "CIRCLE.wav");
    size = GetWAVsize(auxstr);
    if( size != 0)
    {
        scr_printf("    CIRCLE.wav");
        int padextrasize = ((size / mixer_bufsize)+1)*mixer_bufsize;
        if (!(sampleCIRCLE = malloc(padextrasize)))
        {
            printf("Error: Couldnt allocate buffer\n");
            return -1;
        }
        channelTypeCIRCLE = loadWAV(auxstr, sampleCIRCLE, size);
        sampleCIRCLE_Size = padextrasize;
    }
    
    //---------------------------------------------------------------------------------
    // get sampleCROSS
        strcpy(auxstr, fullPath);   strcat(auxstr, "CROSS.wav");
    size = GetWAVsize(auxstr);
    if( size != 0)
    {
        scr_printf("    CROSS.wav");
        int padextrasize = ((size / mixer_bufsize)+1)*mixer_bufsize;
        if (!(sampleCROSS = malloc(padextrasize)))
        {
            printf("Error: Couldnt allocate buffer\n");
            return -1;
        }
        channelTypeCROSS = loadWAV(auxstr, sampleCROSS, size);
        sampleCROSS_Size = padextrasize;
    }

    //---------------------------------------------------------------------------------
    // get sampleSQUARE
        strcpy(auxstr, fullPath);   strcat(auxstr, "SQUARE.wav");
    size = GetWAVsize(auxstr);
    if( size != 0)
    {
        scr_printf("    SQUARE.wav");
        int padextrasize = ((size / mixer_bufsize)+1)*mixer_bufsize;  // padding
        if (!(sampleSQUARE = malloc(padextrasize)))
        {
            printf("Error: Couldnt allocate buffer\n");
            return -1;
        }
        channelTypeSQUARE = loadWAV(auxstr, sampleSQUARE, size);
        sampleSQUARE_Size = padextrasize;
    }

    //---------------------------------------------------------------------------------
    // get samplePAD_LEFT
    strcpy(auxstr, fullPath);   strcat(auxstr, "PAD_LEFT.wav");

    size = GetWAVsize(auxstr);
    if( size != 0)
    {
        scr_printf("    PAD_LEFT.wav");
        int padextrasize = ((size / mixer_bufsize)+1)*mixer_bufsize;
        if (!(samplePAD_LEFT = malloc(padextrasize)))
        {
            printf("Error: Couldnt allocate buffer\n");
            return -1;
        }
        channelTypePAD_LEFT = loadWAV(auxstr, samplePAD_LEFT, size);
        samplePAD_LEFT_Size = padextrasize;
    }
    
        //---------------------------------------------------------------------------------
    // get samplePAD_DOWN
    strcpy(auxstr, fullPath);   strcat(auxstr, "PAD_DOWN.wav");

    size = GetWAVsize(auxstr);
    if( size != 0)
    {
        scr_printf("    PAD_DOWN.wav");
        int padextrasize = ((size / mixer_bufsize)+1)*mixer_bufsize;
        if (!(samplePAD_DOWN = malloc(padextrasize)))
        {
            printf("Error: Couldnt allocate buffer\n");
            return -1;
        }
        channelTypePAD_DOWN = loadWAV(auxstr, samplePAD_DOWN, size);
        samplePAD_DOWN_Size = padextrasize;
    }
    
    //---------------------------------------------------------------------------------
    // get samplePAD_RIGHT
    strcpy(auxstr, fullPath);   strcat(auxstr, "PAD_RIGHT.wav");

    size = GetWAVsize(auxstr);
    if( size != 0)
    {
        scr_printf("    PAD_RIGHT.wav");
        int padextrasize = ((size / mixer_bufsize)+1)*mixer_bufsize;  // padding
        if (!(samplePAD_RIGHT = malloc(padextrasize)))
        {
            printf("Error: Couldnt allocate buffer\n");
            return -1;
        }
        channelTypePAD_RIGHT = loadWAV(auxstr, samplePAD_RIGHT, size);
        samplePAD_RIGHT_Size = padextrasize;
    }
    //---------------------------------------------------------------------------------
    // get samplePAD_UP
    strcpy(auxstr, fullPath);   strcat(auxstr, "PAD_UP.wav");

    size = GetWAVsize(auxstr);
    if( size != 0)
    {
        scr_printf("    PAD_UP.wav");
        int padextrasize = ((size / mixer_bufsize)+1)*mixer_bufsize;
        if (!(samplePAD_UP = malloc(padextrasize)))
        {
            printf("Error: Couldnt allocate buffer\n");
            return -1;
        }
        channelTypePAD_UP = loadWAV(auxstr, samplePAD_UP, size);
        samplePAD_UP_Size = padextrasize;
    }
    
    //---------------------------------------------------------------------------------
    // get samplePAD_L1
    strcpy(auxstr, fullPath);   strcat(auxstr, "PAD_L1.wav");

    size = GetWAVsize(auxstr);
    if( size != 0)
    {
        scr_printf("    PAD_L1.wav");
        int padextrasize = ((size / mixer_bufsize)+1)*mixer_bufsize;
        if (!(samplePAD_L1 = malloc(padextrasize)))
        {
            printf("Error: Couldnt allocate buffer\n");
            return -1;
        }
        channelTypePAD_L1 = loadWAV(auxstr, samplePAD_L1, size);
        samplePAD_L1_Size = padextrasize;
    }
    
    //---------------------------------------------------------------------------------
    // get samplePAD_L2
    strcpy(auxstr, fullPath);   strcat(auxstr, "PAD_L2.wav");

        size = GetWAVsize(auxstr);
    if( size != 0)
    {
        scr_printf("    PAD_L2.wav");
        int padextrasize = ((size / mixer_bufsize)+1)*mixer_bufsize;
        if (!(samplePAD_L2 = malloc(padextrasize)))
        {
            printf("Error: Couldnt allocate buffer\n");
            return -1;
        }
        channelTypePAD_L2 = loadWAV(auxstr, samplePAD_L2, size);
        samplePAD_L2_Size = padextrasize;
    }
    
    //---------------------------------------------------------------------------------
    // get samplePAD_R1
    strcpy(auxstr, fullPath);   strcat(auxstr, "PAD_R1.wav");

    size = GetWAVsize(auxstr);
    if( size != 0)
    {
        scr_printf("    PAD_R1.wav");
        int padextrasize = ((size / mixer_bufsize)+1)*mixer_bufsize;
        if (!(samplePAD_R1 = malloc(padextrasize)))
        {
            printf("Error: Couldnt allocate buffer\n");
            return -1;
        }
        channelTypePAD_R1 = loadWAV(auxstr, samplePAD_R1, size);
        samplePAD_R1_Size = padextrasize;
    }
    
    //---------------------------------------------------------------------------------
    // get samplePAD_R2
    strcpy(auxstr, fullPath);   strcat(auxstr, "PAD_R2.wav");

    size = GetWAVsize(auxstr);
    if( size != 0)
    {
        scr_printf("    PAD_R2.wav");
        int padextrasize = ((size / mixer_bufsize)+1)*mixer_bufsize;
        if (!(samplePAD_R2 = malloc(padextrasize)))
        {
            printf("Error: Couldnt allocate buffer\n");
            return -1;
        }
        channelTypePAD_R2 = loadWAV(auxstr, samplePAD_R2, size);
        samplePAD_R2_Size = padextrasize;
    }
    
    //---------------------------------------------------------------------------------
    // get samplePAD_START
    strcpy(auxstr, fullPath);   strcat(auxstr, "PAD_START.wav");

    size = GetWAVsize(auxstr);
    if( size != 0)
    {
        scr_printf("    PAD_START.wav");
        int padextrasize = ((size / mixer_bufsize)+1)*mixer_bufsize;
        if (!(samplePAD_START = malloc(padextrasize)))
        {
            printf("Error: Couldnt allocate buffer\n");
            return -1;
        }
        channelTypePAD_START = loadWAV(auxstr, samplePAD_START, size);
        samplePAD_START_Size = padextrasize;
    }
    
    //---------------------------------------------------------------------------------

    //------------------------------------------------------------------------//
    // Prepare Pad (From pad_example) //
    //------------------------------------------------------------------------//
    
    scr_printf("\n");
    scr_printf("done\n");
    scr_printf("setup pad support ...\n");

    loadPadModules();
    padInit(0);
    
    port = 0; // 0 -> Connector 1, 1 -> Connector 2
    slot = 0; // Always zero if not using multitap
    
    if ((ret = padPortOpen(port, slot, padBuf)) == 0)
    {
        scr_printf("padOpenPort failed: %d\n", ret);
        SleepThread();
    }
    
    if (!initializePad(port, slot))
    {
        scr_printf("pad initalization failed!\n");
        SleepThread();
    }

    int stereo = 1;
    int mono = 0;
    int maxvolume = 128;
    
    continueloop = 1;

    scr_printf("done\n\n");
    scr_printf("Ready! let the fun start!\n");

    // main loop
	while(continueloop == 1)
	{
		RotateThreadReadyQueue(_MIXER_THREAD_PRIORITY);

        //butres = padUtils_WaitButton(port, slot);
        butres = padUtils_ReadButton(port, slot, old_pad, new_pad);
        if (butres == prev_butres)
            continue;

        prev_butres = butres;

        switch(butres)
        {
        case PAD_SQUARE:
            if (sampleSQUARE != NULL)
            {    PlaySample((sint16*)sampleSQUARE, sampleSQUARE_Size, maxvolume, channelTypeSQUARE);
            scr_printf("square\n");}
            break;
        case PAD_CIRCLE:
            if (sampleCIRCLE != NULL)
            {    PlaySample((sint16*)sampleCIRCLE, sampleCIRCLE_Size, maxvolume, channelTypeCIRCLE);
            scr_printf("circle\n");}
            break;
        case PAD_CROSS:
            if (sampleCROSS != NULL)
            {    PlaySample((sint16*)sampleCROSS, sampleCROSS_Size, maxvolume, channelTypeCROSS);
            scr_printf("cross\n");}
            break;
        case PAD_TRIANGLE:
            if (sampleTRIANGLE != NULL)
            {    PlaySample((sint16*)sampleTRIANGLE, sampleTRIANGLE_Size, maxvolume, channelTypeTRIANGLE);
            scr_printf("triangle\n");}
            break;

        case PAD_LEFT:
            if (samplePAD_LEFT != NULL)
            {   PlaySample((sint16*)samplePAD_LEFT, samplePAD_LEFT_Size, maxvolume, channelTypePAD_LEFT);
            scr_printf("pad left\n");}
            break;
        case PAD_DOWN:
            if (samplePAD_DOWN != NULL)
            {  PlaySample((sint16*)samplePAD_DOWN, samplePAD_DOWN_Size, maxvolume, channelTypePAD_DOWN);
            scr_printf("pad down\n");}
            break;
        case PAD_RIGHT:
            if (samplePAD_RIGHT != NULL)
            { PlaySample((sint16*)samplePAD_RIGHT, samplePAD_RIGHT_Size, maxvolume, channelTypePAD_RIGHT);
            scr_printf("pad right\n");}
            break;
        case PAD_UP:
            if (samplePAD_UP != NULL)
            {    PlaySample((sint16*)samplePAD_UP, samplePAD_UP_Size, maxvolume, channelTypePAD_UP);
            scr_printf("pad up\n");}
            break;
        case PAD_L1:
            if (samplePAD_L1 != NULL)
            {   PlaySample((sint16*)samplePAD_L1, samplePAD_L1_Size, maxvolume, channelTypePAD_L1);
            scr_printf("pad L1\n");}
            break;
        case PAD_L2:
            if (samplePAD_L2 != NULL)
            {  PlaySample((sint16*)samplePAD_L2, samplePAD_L2_Size, maxvolume, channelTypePAD_L2);
            scr_printf("pad L2\n");}
            break;
        case PAD_R1:
            if (samplePAD_R1 != NULL)
            { PlaySample((sint16*)samplePAD_R1, samplePAD_R1_Size, maxvolume, channelTypePAD_R1);
            scr_printf("pad R1\n");}
            break;
        case PAD_R2:
            if (samplePAD_R2 != NULL)
            {PlaySample((sint16*)samplePAD_R2, samplePAD_R2_Size, maxvolume, channelTypePAD_R2);
            scr_printf("pad R2\n");}
            break;
        case PAD_START:
            if (samplePAD_START != NULL)
            { PlaySample((sint16*)samplePAD_START, samplePAD_START_Size, maxvolume, channelTypePAD_START);
            scr_printf("START\n");
            }
            break;

        case PAD_SELECT:
            scr_printf("SELECT button not used yet\n");
            //continueloop = 0;
            break;
        }
	}

    //// end
    //Mixer_Terminate();
    //SjPCM_Setvol(0);
    //SjPCM_Quit();
    ////should free samples ...   TBD

    //scr_printf("\nend\n");
    return 0;
}

///
int loadWAV(char *filename, char* sample, int sample_Size)
{
    char nchannelsInfo[1];
    int stereo = 1;
    int mono = 0;

    FILE *wav;
    wav = fopen(filename, "rb");
    if (wav != NULL)
    {
        fseek(wav, 22, SEEK_SET);			    // bah, should be done in a better way
        fread(nchannelsInfo, 1, 1, wav);

        fseek(wav, 0x30, SEEK_SET);			    // discard header itself

        //if (!(sample = (char*)malloc(*sample_Size)))			// TBD : padding
        //{
        //    //scr_printf("Error: Out of memory for %s\n", filename);
        //    //SleepThread();
        //}

        fread(sample, 1, sample_Size, wav);
        fclose(wav);

        if (nchannelsInfo[0] == 1)
            return mono;
        else
            return stereo;
    }
}

////////
// Reads a pad button without blocking and returns its id
//
int padUtils_ReadButton(int port, int slot, u32 old_pad, u32 new_pad)
{
    struct padButtonStatus buttons;
    int ret;
    u32 paddata;

    ret = padRead(port, slot, &buttons);
    if (ret != 0)
    {
        paddata = 0xffff ^ buttons.btns;
        
        new_pad = paddata & ~old_pad;
        old_pad = paddata;
        

        if (new_pad & PAD_LEFT)
        {
            //scr_printf("LEFT\n");
            return(PAD_LEFT);
        }
        if (new_pad & PAD_DOWN)
        {
            //scr_printf("DOWN\n");
            return(PAD_DOWN);
        }
        if (new_pad & PAD_RIGHT)
        {
            //scr_printf("RIGHT\n");
            return(PAD_RIGHT);
        }
        if (new_pad & PAD_UP)
        {
            //scr_printf("UP\n");
            return(PAD_UP);
        }
        if (new_pad & PAD_START)
        {
            //scr_printf("START\n");
            return(PAD_START);
        }
        if (new_pad & PAD_R3)
        {
            //scr_printf("R3\n");
            return(PAD_R3);
        }
        if (new_pad & PAD_L3)
        {
            //scr_printf("L3\n");
            return(PAD_L3);
        }
        if (new_pad & PAD_SELECT)
        {
            //scr_printf("SELECT\n");
            return(PAD_SELECT);
        }
        if (new_pad & PAD_SQUARE)
        {
            //scr_printf("SQUARE\n");
            return(PAD_SQUARE);
        }
        if (new_pad & PAD_CROSS)
        {
            //scr_printf("CROSS\n");
            return(PAD_CROSS);
        }
        if (new_pad & PAD_CIRCLE)
        {
            //scr_printf("CIRCLE\n");
            return(PAD_CIRCLE);
        }
        if (new_pad & PAD_TRIANGLE)
        {
            //scr_printf("TRIANGLE\n");
            return(PAD_TRIANGLE);
        }
        if (new_pad & PAD_R1)
        {
            //scr_printf("R1\n");
            return(PAD_R1);
        }
        if (new_pad & PAD_L1)
        {
            //scr_printf("L1\n");
            return(PAD_L1);
        }
        if (new_pad & PAD_R2)
        {
            //scr_printf("R2\n");
            return(PAD_R2);
        }
        if (new_pad & PAD_L2)
        {
            //scr_printf("L2\n");
            return(PAD_L2);
        }
    }
    else
        return -1;
    
    return 0;   // 0 means no button was pressed
}


////////
// Blocks until a button is pressed and returns its id
//
int padUtils_WaitButton(int port, int slot)
{
    u32 old_pad = 0, new_pad = 0;
    int ret = 0;

    while(ret==0)
    {
        ret = padUtils_ReadButton(port, slot, old_pad, new_pad);
        RotateThreadReadyQueue(_MIXER_THREAD_PRIORITY);
    }

    if (ret < 0)
        return ret;   // error

    while (padUtils_ReadButton(port, slot, old_pad, new_pad) != 0)
    {
        RotateThreadReadyQueue(_MIXER_THREAD_PRIORITY);
    }

    return ret;
}

//----------------------------------------------------------------------------//
// Pad Functions from pad_example //
//----------------------------------------------------------------------------//

///
int waitPadReady(int port, int slot)
{
    int state;
    int lastState;
    char stateString[16];
    
    state = padGetState(port, slot);
    lastState = -1;
    while ((state != PAD_STATE_STABLE) && (state != PAD_STATE_FINDCTP1))
    {
        if (state != lastState)
        {
            padStateInt2String(state, stateString);
            printf("Please wait, pad(%d,%d) is in state %s\n",
                port, slot, stateString);
        }
        lastState = state;
        state=padGetState(port, slot);
    }
    // Were the pad ever 'out of sync'?
    if (lastState != -1)
    {
        printf("Pad OK!\n");
    }
    return 0;
}

///
int initializePad(int port, int slot)
{
    int ret;
    int modes;
    int i;
    
    waitPadReady(port, slot);
    
    // How many different modes can this device operate in?
    // i.e. get # entrys in the modetable
    modes = padInfoMode(port, slot, PAD_MODETABLE, -1);
    printf("The device has %d modes\n", modes);
    
    if (modes > 0)
    {
        printf("( ");
        for (i = 0; i < modes; i++)
        {
            printf("%d ", padInfoMode(port, slot, PAD_MODETABLE, i));
        }
        printf(")");
    }
    
    printf("It is currently using mode %d\n",
        padInfoMode(port, slot, PAD_MODECURID, 0));
    
    // If modes == 0, this is not a Dual shock controller
    // (it has no actuator engines)
    if (modes == 0)
    {
        printf("This is a digital controller?\n");
        return 1;
    }
    
    // Verify that the controller has a DUAL SHOCK mode
    i = 0;
    do
    {
        if (padInfoMode(port, slot, PAD_MODETABLE, i) == PAD_TYPE_DUALSHOCK)
            break;
        i++;
    }
    while (i < modes);
    if (i >= modes)
    {
        printf("This is no Dual Shock controller\n");
        return 1;
    }
    
    // If ExId != 0x0 => This controller has actuator engines
    // This check should always pass if the Dual Shock test above passed
    ret = padInfoMode(port, slot, PAD_MODECUREXID, 0);
    if (ret == 0)
    {
        printf("This is no Dual Shock controller??\n");
        return 1;
    }
    
    printf("Enabling dual shock functions\n");
    
    // When using MMODE_LOCK, user cant change mode with Select button
    padSetMainMode(port, slot, PAD_MMODE_DUALSHOCK, PAD_MMODE_LOCK);
    
    waitPadReady(port, slot);
    printf("infoPressMode: %d\n", padInfoPressMode(port, slot));
    
    waitPadReady(port, slot);
    printf("enterPressMode: %d\n", padEnterPressMode(port, slot));
    
    waitPadReady(port, slot);
    actuators = padInfoAct(port, slot, -1, 0);
    printf("# of actuators: %d\n",actuators);
    
    if (actuators != 0)
    {
        actAlign[0] = 0;   // Enable small engine
        actAlign[1] = 1;   // Enable big engine
        actAlign[2] = 0xff;
        actAlign[3] = 0xff;
        actAlign[4] = 0xff;
        actAlign[5] = 0xff;
        
        waitPadReady(port, slot);
        printf("padSetActAlign: %d\n",
            padSetActAlign(port, slot, actAlign));
    }
    else
    {
        printf("Did not find any actuators.\n");
    }
    
    waitPadReady(port, slot);
    
    return 1;
}

void loadPadModules()
{
    int ret;
    
    ret = SifLoadModule("rom0:SIO2MAN", 0, NULL);
    if (ret < 0)
    {
        printf("sifLoadModule sio failed: %d\n", ret);
        SleepThread();
    }
    
    ret = SifLoadModule("rom0:PADMAN", 0, NULL);
    if (ret < 0)
    {
        printf("sifLoadModule pad failed: %d\n", ret);
        SleepThread();
    }   
}

//----------------------------------------------------------------------------//
// End Pad Functions from pad_example //
//----------------------------------------------------------------------------//

///
void GetElfFilename(const char *argv0_probe, char* deviceName, char* fullPath, char* elfFilename)
{
        int i;

        int lenght = strlen(argv0_probe);
        int doispontosIndex = 0;
        int slashIndex = 0;
        int isFileOnRoot = 0;

        
        // locate '/' from the end of path
        for(i=lenght-1; i>=0; i--)
        {
            if (argv0_probe[i] == '/')
            {
                slashIndex = i;
                break;
            }
        }
        if (slashIndex == 0)
            isFileOnRoot = 1;       // elf is located on root of device

        // locate ':' from the start of path
        for(i=0; i<lenght; i++)
        {
            if (argv0_probe[i] == ':')
            {
                doispontosIndex = i;
                break;
            }
        }

        // set deviceName to device name (ex: 'mass:', 'host:', 'mc0', etc)
        strncpy(deviceName, argv0_probe, doispontosIndex+1);
        deviceName[doispontosIndex+1] = 0;

        // set fullPath to full path (ex: 'mass:directory/', 'mass:directory1/directory2', etc (no limit over depth))
        if (isFileOnRoot)
            strcpy(fullPath, deviceName);   // fullPath = deviceName actually
        else
        {
            strncpy(fullPath, argv0_probe, slashIndex+1);   
            fullPath[slashIndex+1] = 0;
        }

        // set elfFilename
        if (isFileOnRoot)
            memcpy(elfFilename, argv0_probe + doispontosIndex + 1, lenght - doispontosIndex);
        else
            memcpy(elfFilename, argv0_probe + slashIndex + 1, lenght - slashIndex);
}
