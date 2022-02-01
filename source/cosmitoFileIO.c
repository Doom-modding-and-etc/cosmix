/************************************************************************************************************************
** CosMix                                                                                                              **
** Copyright (c) 2009 - 2010 Pedro Duare(cosmito)                                                                      **
** Copyright (c) 2022 - André Guillaume(Wolf3s)			                                                               **
**                                                                                                                     **
** CosMix Source Code is distributed in the hope that it will be useful,                                               ** 
** but WITHOUT ANY WARRANTY; without even the implied warranty of                                                      **
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the                                                        **
** GNU General Public License for more details.									                                       **
** Description: External functions														              				   **
*************************************************************************************************************************/

#include "include/wav.h"
#include <stdio.h>
// Note: This calls fseek, so watch out...
int GetFilesize(FILE * fd)
{
	int size;
	fseek(fd, 0, SEEK_END);
	size = (int)ftell(fd);
	fseek(fd, 0, SEEK_SET);
	return (size);
}

///
int GetWAVsize(char *filename)
{
    int size;
    FILE *wav;
    wav = fopen(filename, "rb");

    if (wav != NULL)
    {
        fseek(wav, 0, SEEK_END);
        size = (int)ftell(wav);
        size -= 0x30;
        fclose(wav);
        return size;
    }
    else
        return 0;
}

/// TBD
//// Dynamically mallocs a signed char buffer and loads from file into it. res < 0 means error.
//FILE AllocLoad_scharbuffer(char *filename, int optionalBufSize, int *res)
//{
//	FILE *fd;
//	int size;
//
//
//	if (optionalBufSize == 0)
//
//}
