/************************************************************************************************************************
** CosMix                                                                                                              **
** Copyright (c) 2009 - 2010 Pedro Duarte(cosmito)                                                                     **
** Copyright (c) 2022 - André Guillaume(Wolf3s)			                                                               **
**                                                                                                                     **
** CosMix Source Code is distributed in the hope that it will be useful,                                               ** 
** but WITHOUT ANY WARRANTY; without even the implied warranty of                                                      **
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the                                                        **
** GNU General Public License for more details.									                                       **
** Description: External functions														              				   **
*************************************************************************************************************************/
#include <stdio.h>

#ifndef _COSMITOFILEIO
#define _COSMITOFILEIO

// Note: This calls fseek, so watch out...
int GetFilesize(FILE * fd);

int GetWAVsize(char *filename);

#endif // _COSMITOFILEIO
