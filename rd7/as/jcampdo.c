/* jcampdo - this will become entry point for rd7 ?? */
/* receives dxfile pointer, header file pointer
**	pointer to data buffer and length of input buffer
****************************************************/
#include <stdlib.h>
#include <string.h>
#include <std.h>
#include "spec.h"
#include <io.h>
#include <stdio.h>

IMPORT LONG difdup;	/* set from .hdr file in jchdr1 */

VOID jchdr1(TEXT *,KREC *);
LONG far *spset(char far*,KREC far*);
void rdinit(FILE far *);
void wrtInit(FILE far *);
LOCAL KREC skrec;
#define TITLESIZE 72
LOCAL TEXT title[TITLESIZE];


void jcampdo(char*filid,
	FILE far *dxfile,
	FILE far *hdrfile,
	char *inalloc,			  /* file read in */
	LONG splength)			  /* length of file */
	{
	long *spdata;
	KREC far *pkrec;
	int i;

	pkrec = &skrec;
	spdata = spset(inalloc,pkrec);

	/*********** now flip data, pointed to by spdata */
	/******   === account for compression === */
	swapl (spdata, pkrec->npts);

	rdinit(hdrfile);		/* can handle NULL in rdline, I think */
	wrtInit(dxfile);		/* file is open, so sets up for wrtline */

	/* headerfile is either true or false, filid is sent in */
	gttitle(title,(LONG)hdrfile,pkrec,filid);
	jchdr1(title,pkrec);
	jchdr2(spdata,pkrec);
	jchdr3(spdata,pkrec);
	
	if(difdup)
		difdupo(spdata,pkrec);
	else
		dataout(spdata,pkrec);

	/*** Finish Up ***/
	linewrt("##END=");
	return;
	}
