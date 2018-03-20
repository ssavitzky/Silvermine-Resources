/* spset - sets up krec and spectral data, swaps,etc. */
/* receives pointers to a 512byte KREC area 
		and pointer to the input file
	returns pointer to spectral data properly flipped
********************************************************************/

#include <stdlib.h>
#include <string.h>
#include <std.h>
#include "spec.h"
#include <io.h>
#include <stdio.h>

void krec_sw(KREC *);
void prstatus(KREC *);

LONG far *spset(char far* infile, KREC far *pkrec)
	{
	KREC *pkinrec;
	char far *sin;
	char far *sout;
	int i,j,k;
	void  *spdata;
	/* copy first 256 chars of input file to krec */
	sin = infile;
	if ((sout = (char *)pkrec)== NULL)
		errstrg("krec pointer = null");
	for (i=0;i < 256; i++)
		*sout++ = *sin++;
	/* initial swap */
	krec_sw(pkrec);

	if (pkrec->ebytes == 256)
		for (i = 0; i < 256; i++)
			*sout++ = *sin++;    /*transfer second half, 1800*/

	spdata = sin;     /* data */

 /*** === needs converter for this  section === ***/

	/****************************** output some stuff *****/

	return((LONG *)spdata);
	}
