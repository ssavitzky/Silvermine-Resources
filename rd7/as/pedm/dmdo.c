/* dmdo - this will become entry point for rd7 ?? */
/* receives dm file pointer, file opened with wb attribute,
**	pointer to data buffer and length of input buffer
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

#define HDRBUF 1024

char far *spset(char far*,KREC far*);
LOCAL KREC skrec;
void encode(char far*,KREC far*, int *);


void dmdo(char*filid,
	int dmhandle,
	char *inalloc,			  /* file read in */
	LONG splength)			  /* length of file */
	{
	char *spdata;		/* only used in calls */
	KREC far *pkrec;
	int i;
	char *hdralloc;
	int status;

	pkrec = &skrec;
	spdata = spset(inalloc,pkrec);

	/*********** now flip data, pointed to by spdata */
	/******   === account for compression === */
	if (!pkrec->dtype)  /* just in case !! */
		pkrec->dtype = 4;
	switch(pkrec->dtype)
		{
		case 1: /* single byte data, need no swap */
			/* compression code = 1 */
			break;
		case 2: /* 2 byte data, just swap bytes */
			/* compression code = 6 */
			swab(spdata,spdata,2 * pkrec->npts);
			break;
		case 3: /* 3byte data, don't swap, let compression code = 3 */
			break;
		case 4:	/* normal mode, compression code = 8 */
		default:
			swapl (spdata, pkrec->npts);
		}

	/********* ready for encode ??????? *******************/
	hdralloc = (char *)malloc(HDRBUF);

	if (hdralloc == NULL)
		{
		errorPrintf("insufficient memory for header, malloc \n");
		exit(1);
		}

	memset(hdralloc,'\0',HDRBUF);	/* zero fill */
	encode(hdralloc,pkrec,&status);			
	i = strlen(hdralloc);
	write(dmhandle,hdralloc,i);

	/* now write data */
	i = pkrec->npts * pkrec->dtype;	/* takes care of all compressions */
	write(dmhandle,spdata,i);

	return;
	}
