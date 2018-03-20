/* dm3do - this will become entry point for rd3 to Data Manager */
/* receives dm file pointer, file opened with wb attribute,
**	pointer to converted KREC and length of input file
****************************************************/

/****************************************************************
**	header segment is based upon DATA MANAGER's encode modules.
**		apparently(!) the header modules stuff their characters
**		into an array, and then output the works.  This module, therefore
**		allocates a 1K buffer (tho 512 probably enuf), then
**		calls encode. On return finds where "##DATA\r\n" is located,
**		outputs complete header in one swoop, then outputs data
**		in one swoop !!

**	In encode, the game I am playing with KREC is to use the CDS KREC
**		in place of the DM header, since it APPEARS they kept the same 
**    names 
*****************************************************************/

#include <stdlib.h>
#include <string.h>
#include <std.h>
#include "spec.h"
#include <io.h>
#include <stdio.h>
#include "..\okrec.h"

#define HDRBUF 1024
#define DATABUF (8 * 1024)

void encode(char far*,KREC far*, int *);
long getNext3sl(void);	/*get next 2 bytes & convert short to long */


void dm3do(FILE *f,	/* output file */
	KREC far *skrec,
	LONG splength)			  /* length of file */
	{
	KREC far *pkrec;
	int i,j;
	char *hdralloc;
	LONG *spdata;
	char *spchars;
	int status;

	pkrec = skrec;

	/********* ready for encode ??????? *******************/
	hdralloc = (char *)malloc(HDRBUF);

	if (hdralloc == NULL)
		{
		errorPrintf("insufficient memory for header, malloc \n");
		return;
		}

	memset(hdralloc,'\0',HDRBUF);	/* zero fill */
	encode(hdralloc,pkrec,&status);			
	i = strlen(hdralloc);
	fwrite(hdralloc,1,i,f);
	/* finished with hdralloc area, free it */
	free(hdralloc);

	/* now write data */
	dSeekChar((long)sizeof(OKREC));     /*rewinds spfile & reinits data */
	i = pkrec->npts;
	spchars = malloc(DATABUF);
	if (spchars == NULL)
		{
		errorPrintf("Insufficient memory for data points, malloc \n");
		return;
		}
	spdata = (long *)spchars;
	for (i = 0; i < pkrec->npts; i++)
		{
		 *spdata++ = getNext3sl();
		 if((j = ((char *)spdata - spchars)) >=DATABUF)
		 	 { 
			 fwrite(spchars,1,j,f);
			 spdata = (long *)spchars;
			 }
		}
	if(j = (char *)spdata - spchars)
		fwrite(spchars,1,j,f);
	fflush(f);
	free(spchars);
	
	return;
	}

