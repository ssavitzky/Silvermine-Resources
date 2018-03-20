/* uv3set - sets up krec and spectral data, swaps,etc.  for UV to PECSS */
/* receives pointer to a 512byte KREC area holding 3000 krec converted
	to 7000 style.
	converts to css style and writes new krec back
********************************************************************/

#include <stdlib.h>
#include <string.h>
#include <std.h>
#include "uvspec.h"
#include "cssspec.h"
#include <io.h>
#include <stdio.h>

void uv3set(KREC7 far *pkinrec)
	{
	KREC temprec;
	KREC far *pkrec;
	int i,j,k;
	long l,m;
	
	pkrec = &temprec;
	memset((void*)pkrec,0,sizeof(temprec));
	/* copy first 256 chars of input file to krec */
	memcpy((void*)pkrec,(void*)pkinrec,256);

	/* standard sp done, now do uv/ls? specific */
	l = pkinrec->flags;
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
	pkrec->scnspd = pkinrec->scnspd;
	pkrec->rspnse = pkinrec->rspnse;
	pkrec->nirsens = pkinrec->nirsens;
	pkrec->uvslit = pkinrec->uvslit;
	pkrec->uvgain = pkinrec->uvgain;
	pkrec->detswitch = pkinrec->detswitch;
	pkrec->tdwave = pkinrec->mono1;
	pkrec->cycltm = pkinrec->cyctm;
	pkrec->cyclnum = pkrec->ncycles = pkinrec->ncycl;
	pkrec->drvdel = pkinrec->drvdel;
	pkrec->drvord = pkinrec->drvord;
	pkrec->scaldef = (float)pkrec->scale/pkrec->maxy * 10000.0;

	/* now copy back all 512 bytes */
	memcpy((void*)pkinrec,(void*)pkrec,512);
	return;
	}

