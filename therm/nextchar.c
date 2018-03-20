/* nextchar.c - substitutes for rd7 dNextChar	*/
/* performs buffering for large data files */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <std.h>
#include <io.h>
#include <stdio.h>

#define INBUFSIZE 16384

static FILE *infil;
static char *inbufr = NULL;
static char *nextchar = NULL;
static int nbytes = 0;		/* max = INBUFSIZE */

void initData(FILE *infile)
	{
	infil = infile;

	if (!(inbufr = (char *)malloc(INBUFSIZE)))
		{
		printf("can't allocate data buffer \n");
		exit(1);
		}
	nextchar = inbufr;
	return;
	}

int dNextChar(void)
	{
	if (nbytes-- > 0)
		return(*nextchar++ & 255);

	/* no chars in buffer, reload */
	nbytes = fread(inbufr,1,INBUFSIZE,infil);
	if (nbytes <= 0)
		return(-1);
	/*????? check for error or eof???*/
	nextchar = inbufr;
	nbytes--;
	return(*nextchar++ & 255);
	}
