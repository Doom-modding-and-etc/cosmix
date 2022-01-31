#define NULL 0

#include "mixer.h"

#include "mixer_thread.h"
//#include <kernel.h>
//#include "sjpcm.h"

static sint16 *mixbuffer_L;
static sint16 *mixbuffer_R;
sint16 *mixbuffer_L_offset;
sint16 *mixbuffer_R_offset;

static int MIXER_BUFSIZE;

//extern int m_mixer_playing;
extern int count;

//static int debug1;

extern sint16 *s1, *s2;
static channel channels[_MIXER_MAXCHANNELS];

int mixer_period = 1;       // TBD

int tickcount = 0;

/// Section Init

// from gsKit
int from_gsKit_detect_signal()
{
	//char romname[14];
	//GetRomName((char *)romname);
	//if (romname[4] == 'E') {
		return 1;
	//}
	//else {
	//	return 0;
	//}
}

void Mixer_SetMode()
{
    // TBD : sets WAV mono, stereo, or separate buffers mode
}

/// Section A
///
void PlaySample(sint16 * sampleAddress, int sampleLenght, int vol, int stereo)      // TBD : return the channel selected
{
    int selected_channel = -1, nchan = 0;

    // Find the first available (inactive) channel. If none found, use any.
    for(nchan=0; nchan<_MIXER_MAXCHANNELS; nchan++)
    {
        if (channels[nchan].active == 1)
            continue;

        selected_channel = nchan;
        break;
    }

    if (selected_channel == -1)
        // Use any. Pick 1st for now. TBD : improve pick method
        selected_channel = 0;

    // Set channel
    channels[selected_channel].currentSample = sampleAddress;
    channels[selected_channel].active = 1;
    channels[selected_channel].counter = sampleLenght;
    channels[selected_channel].sampleLenght = sampleLenght;
    channels[selected_channel].volume = vol;
    channels[selected_channel].stereo = stereo;
}

///
int PlaySampleAtChannel(int selected_channel, sint16 * sampleAddress, int sampleLenght, int vol, int stereo)
{
    // return if invalid chan number
    if (selected_channel < 0 || selected_channel >= _MIXER_MAXCHANNELS)
        return -1;

    // Set channel
    channels[selected_channel].currentSample = sampleAddress;
    channels[selected_channel].active = 1;
    channels[selected_channel].counter = sampleLenght;
    channels[selected_channel].sampleLenght = sampleLenght;
    channels[selected_channel].volume = vol;
    channels[selected_channel].stereo = stereo;

    return selected_channel;
}

///
int  StopSampleAtChannel(int chan)
{
    // return if invalid chan number
    if (chan < 0 || chan >= _MIXER_MAXCHANNELS)
        return -1;

        channels[chan].active = 0;
        channels[chan].counter = 0;
        channels[chan].currentSample = NULL;
        channels[chan].sampleLenght = 0;
		channels[chan].volume = 0;
        channels[chan].stereo = 0;

	return chan;
}

///
int IsPlayingAtChannel(int chan)
{
    // return if invalid chan number
    if (chan < 0 || chan >= _MIXER_MAXCHANNELS)
        return -1;

    return channels[chan].active;
}

///
int IsPlaying()
{
    int i;
    int nPlaying = 0;

    for(i=0; i<_MIXER_MAXCHANNELS; i++)
    {   
        nPlaying = nPlaying + channels[i].active;
    }
    return nPlaying;
}

/// Section B
///

///
void Mixer_Init()
{
    int i, PAL;

    // init buffers
printf("Mixer_Init(): init buffers: malloc mixbuffer_L - %d bytes\n", sizeof(sint16)*_MIXER_BUFSIZE);
    mixbuffer_L = malloc(sizeof(sint16)*_MIXER_BUFSIZE);
printf("Mixer_Init(): init buffers: malloc mixbuffer_R - %d bytes\n", sizeof(sint16)*_MIXER_BUFSIZE);
    mixbuffer_R = malloc(sizeof(sint16)*_MIXER_BUFSIZE);
    // Init channels
    for(i=0; i<_MIXER_MAXCHANNELS; i++)
    {   
        channels[i].active = 0;
        channels[i].counter = 0;
        channels[i].currentSample = NULL;
        channels[i].sampleLenght = 0;
		channels[i].volume = 0;
		channels[i].stereo = 0;
    }

    // Check for PAL or NTSC
    PAL = from_gsKit_detect_signal();

    if (PAL == 1)
        MIXER_BUFSIZE = 960*3;
    else
        MIXER_BUFSIZE = 800;

    // Start mixer thread
    Mixer_StartThread();
	Mixer_AddVBlankHandler();
}

///
void Mixer_Terminate()
{
    Mixer_RemoveVBlankHandler();
	Mixer_StopThread();

    // TBD : incorporate SjPCM control here?
}

///
void Mixer_Tick()
{
    int nchan, mi, stereo, vol;
    float volfact = 1.0;
    sint16 sleft = 0, sright = 0;		            // TBD : should be static for better performance ?
    sint32 smixLeft = 0, smixRight = 0;
    int _doMix = 0, i = 0;

    if (tickcount > 2)
    {
        tickcount = 0;
    }
    if (tickcount == 0)
    {
        _doMix = 1;
    }

    if (_doMix == 1)
    {
        /// TBD : get ogg decoded MIXER_BUFSIZE

        // mix the samples
        for(mi=0; mi<MIXER_BUFSIZE; mi=mi+mixer_period)
        {
            smixLeft = 0;
            smixRight = 0;

            mixbuffer_L[mi] = 0;
            mixbuffer_R[mi] = 0;

            for(nchan=0; nchan<_MIXER_MAXCHANNELS; nchan++)
            {
                if (channels[nchan].active == 0)
                    continue;

                if (channels[nchan].counter > 0)
                {
                    stereo = channels[nchan].stereo;
                    sleft  = *channels[nchan].currentSample++;                  // get left

                    vol = channels[nchan].volume;
                    if (vol < 128)
                    {
                        volfact = (float)vol/128.0;
                        sleft = (sint16) (volfact * (float)sleft);
                    }
                    channels[nchan].counter -= 2;

                    if (stereo == 1)                
                    {
                        sright = *channels[nchan].currentSample++;              // get right
                        vol = channels[nchan].volume;
                        if (vol < 128)
                        {
                            volfact = (float)vol/128.0;
                            sright = (sint16) (volfact * (float)sright);
                        }
                        channels[nchan].counter -= 2;
                    }
                    else
                        sright = sleft;

                    // If end of sample was reached, put sample into inactive and reset the remaining params
                    if (channels[nchan].counter <= 0)
                    {
                        channels[nchan].active = 0;
                        channels[nchan].counter = 0;
                        channels[nchan].currentSample = NULL;
                        channels[nchan].sampleLenght = 0;
                        channels[nchan].volume = 0;
                        channels[nchan].stereo = 0;
                    }
                }
                smixLeft += sleft;
                smixRight += sright;
            }

            /// TBD : ogg stream mixed here
            //  smixLeft += oggsleft[mi]   : mi - atenção ao mixer_period !
            //  smixRight += oggsright[mi]


            if(smixLeft > 0x7FFF) smixLeft = 0x7FFF;		// simple volume limiting (borrowed from pgen source)
            else if(smixLeft < -0x8000) smixLeft = -0x8000;
            mixbuffer_L[mi] = (sint16)smixLeft;

            if(smixRight > 0x7FFF) smixRight = 0x7FFF;
            else if(smixRight < -0x8000) smixRight = -0x8000;
            mixbuffer_R[mi] = (sint16)smixRight;

            // If halving frequency, repeat left and right sample
            if (mixer_period == 2)
            {
                mixbuffer_L[mi+1] = (sint16)smixLeft;
                mixbuffer_R[mi+1] = (sint16)smixRight;
            }
        }
    }

    //if (allChannelsInactive == 1)
    mixbuffer_L_offset = mixbuffer_L;
    mixbuffer_R_offset = mixbuffer_R;

    for (i=0; i<tickcount; i++)
    {
        mixbuffer_L_offset += 960;
        mixbuffer_R_offset += 960;
    }

    //SjPCM_Enqueue((sint16*)mixbuffer_L_offset, (sint16*)mixbuffer_R_offset, 960, /*1*/ 0);
    tickcount = tickcount + 1;
}

/*

from : http://replaygain.hydrogenaudio.org/wav_format.txt

                                        Sample 1
							   LEFT                    RIGHT
                             16 bits                  16 bits
                       ----SIGNED INT----       ----SIGNED INT----  
                     /                    \   /                    \
                     LEFT LOW     LEFT HIGH   RIGHT LOW    RIGHT HIGH

                     Channel 0    Channel 0   Channel 1    Channel 1
                      (left)       (left)      (right)      (right)
                     low-order   high-order   low-order   high-order
                       byte         byte         byte        byte


                            Data Packing for 16-Bit Stereo PCM

*/
