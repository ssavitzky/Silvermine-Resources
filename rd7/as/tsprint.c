#include <stdlib.h>
#include <string.h>
#include <std.h>
#include "spec.h"
#include <io.h>
#include <stdio.h>


main()
	{
		int ndecpl;					/* Number of dec. pl. in output	*/
	TEXT koutln[82];			 /*	output buffer */
	DOUBLE ymin;				 /*	minimum	value */
	DOUBLE ymax;				 /*	maximum	value */
	DOUBLE del;					 /*	spectrum interval */
	DOUBLE temp;				 /*	temporary */
	DOUBLE temp1;				 /*	temporary */
	UTINY stype;				/* spectrum	type */
	int i;   	  				/* counter */
	int j;					   /* counter */
	LONG npts;


	temp = pkrec->istart/f100;
	temp1 =	pkrec->ifin/f100;
/* The following line doesn't work!!!!!!!!!!!!!!!!!!!!!!!!\
**	sprintf(koutln + 3,"%6i %8.1f %8.1f%6.2f",pkrec->npts,temp,temp1,del);
**** so try the following!!!!!!*/
	sprintf(koutln+3,"%6i",npts);
	sprintf(koutln+10,"%8.1f %8.1f",temp,temp1);
	sprintf(koutln+26,"%6.2f",del);

