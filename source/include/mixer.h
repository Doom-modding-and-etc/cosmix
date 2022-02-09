/************************************************************************************************************************
** CosMix                                                                                                              **
** Copyright (c) 2009 - 2010 Pedro Duarte(cosmito)                                                                     **
** Copyright (c) 2022 - André Guillaume(Wolf3s)			                                                               **
**                                                                                                                     **
** CosMix Source Code is distributed in the hope that it will be useful,                                               ** 
** but WITHOUT ANY WARRANTY; without even the implied warranty of                                                      **
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the                                                        **
** GNU General Public License for more details.									                                       **
** Description: Mixer functions 														                               **
*************************************************************************************************************************/

#ifndef _MIXER_H
#define _MIXER_H

#define _MIXER_MAXCHANNELS 16
#define _MIXER_BUFSIZE  960*3     // TBD

typedef signed short sint16;
typedef signed int sint32;

#include <stdio.h>

/// Section Init
///
void Mixer_SetMode();
void Mixer_Init();
void Mixer_Terminate();

/// Section A
///
void PlaySample(sint16 * sampleAddress, int sampleLenght, int vol, int stereo);
int  PlaySampleAtChannel(int selected_channel, sint16 * sampleAddress, int sampleLenght, int vol, int stereo);
int  StopSampleAtChannel(int selected_channel);
int  IsPlayingAtChannel(int chan);
int  IsPlaying();                   // Returns the number of channels currently playing

/*
**se fizer um resume, testar se o channel.counter > 0 ... assim distingue-se entre um stop (.counter == 0) e um pause.
**If makes an resume, test if the channel.counter > 0 ... still can be distiguinted between stop (.counter = 0) and one pause
*/

///
/// Section B
///
void Mixer_Tick();

typedef struct
{
    int active;             // 1 if playing, 0 if not playing
    sint16 *currentSample;
    int counter;
    int sampleLenght;       // redundant
	int volume;			    // 0 to 128
    int stereo;			    // 1 if playing stereo sample, 0 if mono

} channel;

// EN-US: use: array channels : channel channels[]
// PT-BR uso : array channels : channel channels[]

#endif
