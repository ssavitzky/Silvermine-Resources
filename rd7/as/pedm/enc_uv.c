/************************************************************************/
/*									*/
/*			UV Header Encoding Module			*/
/*			Version 1.0	7/2/87				*/
/*									*/
/* Description:	The following module is used encode the UV specific 	*/
/*		part of a krec into the standard file format.		*/
/*									*/
/* Contents:	encode_uv()	       -Create UV part of the header  	*/
/*					from a krec.			*/
/*									*/
/* Author:	T. J. Moberly						*/
/*		Perkin-Elmer Corporation				*/
/*		Oak Brook Software group				*/
/*		2OOO York Road Box 4776					*/
/*		Oak Brook, Illinois 60522				*/
/*									*/
/* Version 1.0:	T. J. Moberly 7/2/87					*/
/*		Initial implementaion of UV file header module.		*/
/*									*/
/************************************************************************/

#include "portab.h"
#include "dmstd.h"		/* AS - to handle UTINY, etc in spec.h*/
#include "spec.h"
#include "dmspec.h"

/************************************************************************/
/*									*/
/* Function:	encode_uv	-Create the UV specific part of the    	*/
/*				 standard file format from a krec.	*/
/*									*/
/* Description:	The following function creates the UV specific part of 	*/
/*		a standard file header from the specified krec. 	*/
/*									*/
/* Globals:	None.							*/
/*									*/
/* Parameters:	CHAR 	*ptr;		Pointer to header info.		*/
/*		KREC	*kptr;		Pointer to krec to be saved.	*/
/*									*/
/* Input:	None.							*/
/*									*/
/* Output:	None.							*/
/*									*/
/* Calls:	None.							*/
/*									*/
/* Returns:	None.							*/
/*									*/
/************************************************************************/

VOID
overlay(ptr, kptr)
CHAR	*ptr;				/* Pointer to header info	*/
KREC	*kptr;				/* Krec Pointer			*/
{
	CHAR	temp[64];		/* Temporary string holding	*/

	/****************************************************************/
	/* Put the UV specific part of the krec intop the header.	*/
	/****************************************************************/

	sprintf(temp, "%u\r\n", kptr->instno);	/* Instrument number	*/
	strcat(ptr, temp);

						/* Fixed wave		*/
	sprintf(temp, "%-1.2f\r\n", kptr->fixwave / 100.0);
	strcat(ptr, temp);

	switch (kptr->method)			/* Method		*/
	{
		case SCAN	: strcat(ptr, "SCAN");
				  break;

		case TIMEDR	: strcat(ptr, "TIMEDR");
				  break;

		case WLPG	: strcat(ptr, "WLPG");
				  break;

		case QUANT	: strcat(ptr, "QUANT");
				  break;

		case CONC	: strcat(ptr, "CONC");
				  break;

		case KINS	: strcat(ptr, "KINETICS");
				  break;

		case WAVE_2	: strcat(ptr, "WAVE2");
	}
	strcat(ptr, "\r\n");

	sprintf(temp, "%u\r\n", kptr->cycles);	/* Current cycle number	*/
	strcat(ptr, temp);

	sprintf(temp, "%u\r\n", kptr->totcycs);	/* Total no. of cycles	*/
	strcat(ptr, temp);

						/* Response time	*/
	sprintf(temp, "%-1.1f\r\n", kptr->response / 10.0);
	strcat(ptr, temp);

	sprintf(temp, "%d\r\n", kptr->integ);	/* Integration time	*/
	strcat(ptr, temp);

						/* Cycle time		*/
	sprintf(temp, "%-1.1f\r\n", kptr->cyctime / 10.0);
	strcat(ptr, temp);

						/* Scanning speed	*/
	sprintf(temp, "%-1.1f\r\n", kptr->scnspd / 10.0);
	strcat(ptr, temp);

						/* Slit size		*/
	sprintf(temp, "%-1.2f\r\n", kptr->slit / 100.0);
	strcat(ptr, temp);

	if (kptr->accessory & CELLPROG)		/* Cell programmer	*/
		strcat(ptr, "CELLPROG");
	strcat(ptr, "\r\n");

	if (kptr->accessory & TCA)		/* Temperature control	*/
		strcat(ptr, "TEMP");
	strcat(ptr, "\r\n");

	if (kptr->accessory & SIPPER)		/* Sipper		*/
		strcat(ptr, "SIPPER");
	strcat(ptr, "\r\n");

	if (kptr->accessory & MULTISAMP)	/* Multi sampler	*/
		strcat(ptr, "MULTISAMP");
	strcat(ptr, "\r\n");

	switch (kptr->lamps)			/* Lamps		*/
	{
		case 1	: strcat(ptr, "UV");
			  break;

		case 2	: strcat(ptr, "VIS");
			  break;

		case 3	: strcat(ptr, "UV/VIS");
	}
	strcat(ptr, "\r\n");
}
