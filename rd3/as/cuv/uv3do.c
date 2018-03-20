/* uv3do - this will become entry point for rd3 ?? */
/* receives sp file pointer, file opened with wb attribute,
**	pointer to data buffer and length of input buffer
****************************************************/

/****************************************************************
*****************************************************************/

#include <stdlib.h>
#include <string.h>
#include <std.h>
#include "..\spec.h"
#include <io.h>
#include <stdio.h>

void zbufr(char far *,long);

void uv3do(FILE *f,	/* output file */
	KREC far *skrec,
	LONG splength)			  /* length of file */
	{
	LONG *spdata;		/* only used in calls */
	KREC far *pkrec;
	int i;
	char *hdralloc;
	int status;

	pkrec = skrec;
	/* at this point, we have krec in 7000 form, now convert to cuv*/
	/* uv3set takes this krec, converts it, then returns new krec 
			in its place*/
	uv3set(pkrec);

	/* If done, then we can write !!! */
	fwrite((char *)pkrec,1,512,f);    /* UV always 512 bytes */

	/* now write data */
	i = pkrec->npts;		/* takes care of all compressions */
	spdata = (long *)malloc(i * 4);
	if (spdata == NULL)
		{
		errorPrintf("Insufficient memory for data points, malloc \n");
		return;
		}
	for (i = 0; i < pkrec->npts; i++)
		spdata[i] = getNext3sl();
	i = fwrite(spdata,4,(size_t)pkrec->npts,f);
	fflush(f);
	free(spdata);

	return;
	}

	
