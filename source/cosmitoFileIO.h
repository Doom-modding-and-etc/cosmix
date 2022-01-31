#include <stdio.h>

#ifndef _COSMITOFILEIO
#define _COSMITOFILEIO

// Note: This calls fseek, so watch out...
int GetFilesize(FILE * fd);

int GetWAVsize(char *filename);

#endif // _COSMITOFILEIO
