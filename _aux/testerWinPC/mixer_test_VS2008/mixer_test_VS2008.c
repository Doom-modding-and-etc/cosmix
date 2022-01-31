// mixer_test_VS2008.cpp : Defines the entry point for the console application.
//

//#include "stdafx.h"

#include <stdio.h>
#include <string.h>
#include "cosmitoFileIO.h"
#include <malloc.h>

#define NULL 0
#define scr_printf printf
typedef signed short sint16;

char* sampleTRIANGLE = NULL;

int loadWAV(char *filename, char* sample, int sample_Size);

int main(int argc, char* argv[])
{
    int mixer_bufsize = 0, size = 0, padextrasize = 0;
    int channelTypeTRIANGLE = 0;
    int sampleTRIANGLE_Size;
    char fullPath[100];
    char auxstr[255];

    // Check for PAL or NTSC
    int PAL = from_gsKit_detect_signal();

    if (PAL == 1)
        //mixer_bufsize = 960;
        mixer_bufsize = 1024;
    else
        mixer_bufsize = 800;

    Mixer_Init();
    scr_printf("Loading samples from %s ...\n", "");

    // get sampleTRIANGLE
    strcpy(auxstr, "");   strcat(auxstr, "TRIANGLE.wav");

    size = GetWAVsize(auxstr);
    if( size != 0)
    {
        scr_printf("    TRIANGLE.wav");
        padextrasize = ((size / mixer_bufsize)+1)*mixer_bufsize;  // padding
        if (!(sampleTRIANGLE = (char*)malloc(padextrasize)))
        {
            printf("Error: Couldnt allocate buffer\n");
            return -1;
        }
        channelTypeTRIANGLE = loadWAV(auxstr, sampleTRIANGLE, size);
        sampleTRIANGLE_Size = padextrasize;
    }

    if (sampleTRIANGLE != NULL)
    {
        PlaySample((sint16*)sampleTRIANGLE, sampleTRIANGLE_Size, 128, channelTypeTRIANGLE);
        scr_printf("triangle\n");
    }

    do 
    {
        Mixer_Tick();

    } while (1);

    Mixer_Tick();
    Mixer_Tick();
    Mixer_Tick();
    Mixer_Tick();
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
