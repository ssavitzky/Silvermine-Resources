/* cuvdo - this will become entry point for rd7 ?? */
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

char far *uvset(char far*,KREC far*);
void zbufr(char far *,long);
LOCAL KREC skrec;

void cuvdo(char*filid,
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
	zbufr((char *)pkrec,(LONG)sizeof(skrec));

	/*uvset is updated version of spset for cuv to css conversion */
	spdata = uvset(inalloc,pkrec);

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

	/* If done, then we can write !!! */
	write(dmhandle,(char *)pkrec,512);    /* UV always 512 bytes */

	/* now write data */
	i = pkrec->npts * pkrec->dtype;	/* takes care of all compressions */
	write(dmhandle,spdata,i);

	return;
	}

	
