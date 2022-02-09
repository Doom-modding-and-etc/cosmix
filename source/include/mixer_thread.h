/************************************************************************************************************************
** CosMix                                                                                                              **
** Copyright (c) 2009 - 2010 Pedro Duarte(cosmito)                                                                     **
** Copyright (c) 2022 - André Guillaume(Wolf3s)			                                                               **
**                                                                                                                     **
** CosMix Source Code is distributed in the hope that it will be useful,                                               ** 
** but WITHOUT ANY WARRANTY; without even the implied warranty of                                                      **
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the                                                        **
** GNU General Public License for more details.									                                       **
** Description: Mixer Thread functions		                                                                           **
*************************************************************************************************************************/

#ifndef _MIXER_THREAD_H
#define _MIXER_THREAD_H

#define _MIXER_THREAD_PRIORITY 30

void Mixer_AddVBlankHandler();
void Mixer_RemoveVBlankHandler();
void Mixer_StartThread();
void Mixer_StopThread();
int Mixer_Tick_IntHandler(int cause);
void MixerThread_Play(void *arg);

#endif
