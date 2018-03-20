/* uvset - sets up krec and spectral data, swaps,etc.  for UV to PECSS */
/* receives pointers to a 512byte KREC area 
		and pointer to the input file
	returns pointer to spectral data properly flipped
********************************************************************/

#include <stdlib.h>
#include <string.h>
#include <std.h>
#include "uvspec.h"
#include "cssspec.h"
#include <io.h>
#include <stdio.h>


void krec_sw(KREC *);
void swapl(LONG *,int);
int swaps(int);

LONG far *uvset(char far* infile, KREC far *pkrec)
	{
	KREC7 far *pkinrec;
	char far *sin;
	char far *sout;
	int i,j,k;
	long l,m;
	void  *spdata;

	pkinrec = (KREC7 *)infile;
	/* copy first 256 chars of input file to krec */
	sin = infile;
	if ((sout = (char *)pkrec)== NULL)
		errstrg("krec pointer = null");
	for (i=0;i < 256; i++)
		*sout++ = *sin++;
	/* initial swap */
	krec_sw(pkrec);
	pkrec->ndel = abs(pkrec->ndel);	/* seems to be always pos */

	/* standard sp done, now do uv/ls? specific */
	l = pkinrec->flags;
	swapl(&l,1);
	pkrec->filler = 0;
	if (pkrec->stype == 2)
		pkrec->ordtype = 3;
	else
		{
		if(l & 1 )
			pkrec->ordtype = 1;
		else
			pkrec->ordtype = 2;
		}
	pkrec->manip = 0;
	if (l & 2) pkrec->manip |= FLG_LOG;
	if (l & 4) pkrec->manip |= FLG_DIFF;
	if (l & 8) pkrec->manip |= FLG_FLAT;
	if (l & 16) pkrec->manip |= FLG_MERGE;
	if (l & 32) pkrec->manip |= FLG_ARITH;
	if (l & 128) pkrec->manip |= FLG_MOD;
	if (pkrec->naccs > 1) pkrec->manip |= FLG_AVE;
	pkrec->filler = 0;
	pkrec->method = pkinrec->dspare;
	pkrec->ordmode = pkinrec->ordmode;
	pkrec->scnspd = swaps(pkinrec->scnspd);
	pkrec->rspnse = pkinrec->rspnse;
	pkrec->nirsens = pkinrec->nirsens;
	pkrec->uvslit = swaps(pkinrec->uvslit);
	pkrec->uvgain = swaps(pkinrec->uvgain);
	pkrec->detswitch = swaps(pkinrec->detswitch);
	pkrec->tdwave = pkinrec->mono1;
	swapl(&pkrec->tdwave,1);
	pkrec->cycltm = swaps(pkinrec->cyctm);
	pkrec->cyclnum = pkrec->ncycles = swaps(pkinrec->ncycl);
	pkrec->drvdel = swaps(pkinrec->drvdel);
	pkrec->drvord = pkinrec->drvord;
	pkrec->scaldef = (float)pkrec->scale/pkrec->maxy * 10000.0;


	if (pkrec->ebytes == 256)
	   spdata = infile + 512;     /* data */
	else
		{
		spdata = infile + 256;
		pkrec->ebytes = 256;
		}
	return((LONG *)spdata);
	}

