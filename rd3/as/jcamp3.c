/* jcamp - this will become entry point for rd3  jcamp output  */
/* receives dxfile pointer, header file pointer	and
**	pointer to data buffer 
****************************************************/
#include <stdlib.h>
#include <string.h>
#include <std.h>
#include "..\..\as\spec.h"
#include <io.h>
#include <stdio.h>
#include "okrec.h"

IMPORT LONG difdup;	/* set from .hdr file in jchdr1 */

VOID jchdr1(TEXT *,KREC *);
VOID jc3hdr2(KREC *);
VOID jc3hdr3(LONG, KREC *);	/* pass firstpoint */
void rdinit(FILE far *);
void wrtInit(FILE far *);
LONG getNext3sl(void);
void dSeekChar(LONG);

#define TITLESIZE 72
LOCAL TEXT title[TITLESIZE];

void jcamp3(char*filid,
	FILE far *dxfile,
	FILE far *hdrfile,
	KREC *pkrec)
{
	int i;
	long firstpoint;

	rdinit(hdrfile);		/* can handle NULL in rdline, I think */
	wrtInit(dxfile);		/* file is open, so sets up for wrtline */

	/* headerfile is either true or false, filid is sent in */
	gttitle(title,(LONG)hdrfile,pkrec,filid);
	jchdr1(title,pkrec);
	jc3hdr2(pkrec);
	dSeekChar((long)sizeof(OKREC));     /*rewinds spfile & reinits data */
	firstpoint = getNext3sl();
	jc3hdr3(firstpoint,pkrec);

	dSeekChar((long)sizeof(OKREC));     /*rewinds spfile & reinits data */
	if(difdup)
		difdupo(pkrec);
	else
		dataout(pkrec);

	/*** Finish Up ***/
	return;
}

