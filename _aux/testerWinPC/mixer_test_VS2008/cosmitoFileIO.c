#include "cosmitoFileIO.h"

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
