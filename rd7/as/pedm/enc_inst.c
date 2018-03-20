/************************************************************************/
/*									*/
/*			Instrument Header Encoding Module		*/
/*			Version 1.1	11/19/87			*/
/*									*/
/* Description:	The following module is used encode the Instrument	*/
/*		specific part of a krec into the standard file format.	*/
/*									*/
/* Contents:	encode_4()	       -Create instrument specific	*/
/*					part of header for Lambda 4.	*/	
/*									*/
/* Author:	T. J. Moberly						*/
/*		Perkin-Elmer Corporation				*/
/*		Oak Brook Software group				*/
/*		2OOO York Road Box 4776					*/
/*		Oak Brook, Illinois 60522				*/
/*									*/
/* Version 1.0:	T. J. Moberly 7/2/87					*/
/*		Initial implementaion of Lambda 4 file header module.	*/
/*									*/
/* Version 1.1:	T. J. Moberly 11/19/87					*/
/*		Adjustments made to the scaling and format strings	*/
/*		applied to the wavelength programming factors and waves.*/
/*									*/
/************************************************************************/

#include "portab.h"
#include "dmstd.h"		/* AS - to handle UTINY, etc in spec.h*/
#include "spec.h"
#include "dmspec.h"

/************************************************************************/
/*									*/
/* Function:	encode_4	-Encodes the Lambda 4 instrument    	*/
/*				 specific part of the krec.		*/
/*									*/
/* Description:	The following function encodes the Lambda 4 instrument	*/
/*		specific part of the krec into the standard file format.*/
/*									*/
/* Globals:	None.							*/
/*									*/
/* Parameters:	CHAR 	*ptr;		Pointer to header info.		*/
/*		KREC	*kptr;		Pointer to Krec being encoded.	*/
/*									*/
/* Input:	None.							*/
/*									*/
/* Output:	None.							*/
/*									*/
/* Calls:	None.  							*/
/*									*/
/* Returns:	None.							*/
/*									*/
/************************************************************************/

VOID
overlay(ptr, kptr)
CHAR	*ptr;				/* Pointer to header data	*/
KREC	*kptr;				/* Pointer to krec		*/
{
	CHAR	temp[256];		/* Scratchpad buffer area	*/

	/****************************************************************/
	/* Encode the instrument specific part of the krec.		*/
	/****************************************************************/

	switch (kptr->corr_typ)
	{
		case 'p' : strcat(ptr, "PARTIAL");
			   break;

		case 'c' : strcat(ptr, "CORRECTED");
			   break;

		default  : strcat(ptr, "UNCORRECTED");
	}
	strcat(ptr, "\r\n");

	sprintf(temp, "%d\r\n", kptr->drv_ord);
	strcat(ptr, temp);

	sprintf(temp, "%u\r\n", kptr->drv_pts);
	strcat(ptr, temp);

	sprintf(temp, "%u\r\n", kptr->idno);
	strcat(ptr, temp);

	sprintf(temp, "%-1.2f\r\n", (kptr->minutes / 100.0));
	strcat(ptr, temp);

	sprintf(temp, "%-1.2f\r\n", (kptr->delminute / 100.0));
	strcat(ptr, temp);

	sprintf(temp, "%-.1f\r\n", (kptr->baselo / 100.0));
	strcat(ptr, temp);

	sprintf(temp, "%-.1f\r\n", (kptr->basehi / 100.0));
	strcat(ptr, temp);

	if (kptr->ser_over == 0)
		strcat(ptr, "SERIAL\r\n");
	else strcat(ptr, "OVERLAY\r\n");

	if (kptr->output & PLOT)
		strcat(ptr, "PLOT");
	strcat(ptr, "\r\n");

	if (kptr->output & TABLE)
		strcat(ptr, "TABLE");
	strcat(ptr, "\r\n");

	if (kptr->output & INCREMENT)
		strcat(ptr, "INCREMENT");
	strcat(ptr, "\r\n");

	if (kptr->output & GRID)
		strcat(ptr, "GRID");
	strcat(ptr, "\r\n");

	if (kptr->output & DISKCPY)
		strcat(ptr, "COPY");
	strcat(ptr, "\r\n");

	if (kptr->output & SAFE_ALT)
		strcat(ptr, "SAFE_ALT");
	strcat(ptr, "\r\n");

	switch (kptr->grid)
	{
		case GRID_X	: strcat(ptr, "X\r\n");
			  	  break;

		case GRID_Y	: strcat(ptr, "Y\r\n");
			  	  break;

		default	: strcat(ptr, "XY\r\n");
	}

	switch (kptr->timeunits)
	{
		case 0	: strcat(ptr, "SEC\r\n");
			  break;

		case 1	: strcat(ptr, "MIN\r\n");
			  break;

		case 2	: strcat(ptr, "AUTO\r\n");
	}

	sprintf(temp, "%-1.2f\r\n", (kptr->instdel / 100.0));
	strcat(ptr, temp);

	sprintf(temp, "%-1.2f\r\n", (kptr->dispdel / 100.0));
	strcat(ptr, temp);

	sprintf(temp, "%-1.2f\r\n", (kptr->ordmax / 100.0));
	strcat(ptr, temp);

	sprintf(temp, "%-1.2f\r\n", (kptr->ordmin / 100.0));
	strcat(ptr, temp);

	sprintf(temp, "%-.1f,%-.1f,%-.1f,%-.1f,%-.1f,%-.1f,%-.1f,%-.1f\r\n", (kptr->wave1 / 100.0),
		(kptr->wave2 / 100.0),(kptr->wave3 / 100.0),
		(kptr->wave4 / 100.0),(kptr->wave5 / 100.0),
		(kptr->wave6 / 100.0),(kptr->wave7 / 100.0),
		(kptr->wave8 / 100.0));
	strcat(ptr, temp);

	sprintf(temp, "%-.3f,%-.3f,%-.3f,%-.3f,%-.3f,%-.3f,%-.3f,%-.3f\r\n", (kptr->fact1 / 1000.0),
		(kptr->fact2 / 1000.0),(kptr->fact3 / 1000.0),
		(kptr->fact4 / 1000.0),(kptr->fact5 / 1000.0),
		(kptr->fact6 / 1000.0),(kptr->fact7 / 1000.0),
		(kptr->fact8 / 1000.0));
	strcat(ptr, temp);

	sprintf(temp, "%s\r\n", kptr->ext_name);
	strcat(ptr, temp);

	if (kptr->accessory & CELLPROG)
	{
		sprintf(temp, "%-1.2f,%-1.2f,%-1.2f,%-1.2f,%-1.2f,%-1.2f\r\n", (kptr->acc.cells.cellmin1 / 100.0) + .005,
			(kptr->acc.cells.cellmin2 / 100.0) + .005,(kptr->acc.cells.cellmin3 / 100.0) + .005,
			(kptr->acc.cells.cellmin4 / 100.0) + .005,(kptr->acc.cells.cellmin5 / 100.0) + .005,
			(kptr->acc.cells.cellmin6 / 100.0) + .005);
		strcat(ptr, temp);

		sprintf(temp, "%-1.2f,%-1.2f,%-1.2f,%-1.2f,%-1.2f,%-1.2f\r\n", (kptr->acc.cells.cellmax1 / 100.0) + .005,
			(kptr->acc.cells.cellmax2 / 100.0) + .005,(kptr->acc.cells.cellmax3 / 100.0) + .005,
			(kptr->acc.cells.cellmax4 / 100.0) + .005,(kptr->acc.cells.cellmax5 / 100.0) + .005,
			(kptr->acc.cells.cellmax6 / 100.0) + .005);
		strcat(ptr, temp);

		sprintf(temp, "%d\r\n", kptr->acc.cells.cellmap);
		strcat(ptr, temp);
		strcat(ptr, "\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n");
	}
	else
	{
		strcat(ptr, "\r\n\r\n\r\n");
		sprintf(temp, "%d\r\n", kptr->acc.accsry.samples);
		strcat(ptr, temp);

		sprintf(temp, "%d\r\n", kptr->acc.accsry.stndards);
		strcat(ptr, temp);

		sprintf(temp, "%-1.1f\r\n", (kptr->acc.accsry.volume / 10.0));
		strcat(ptr, temp);

		sprintf(temp, "%-1.1f\r\n", (kptr->acc.accsry.forward / 10.0));
		strcat(ptr, temp);

		sprintf(temp, "%-1.1f\r\n", (kptr->acc.accsry.position / 10.0));
		strcat(ptr, temp);

		sprintf(temp, "%-1.1f\r\n", (kptr->acc.accsry.pdelay / 10.0));
		strcat(ptr, temp);

		if (kptr->acc.accsry.temp1 == 9999)
			strcat(ptr, "TEMPOFF");
		else
		{
			sprintf(temp, "%-1.1f\r\n", (kptr->acc.accsry.temp1 / 10.0));
			strcat(ptr, temp);
		}

		strcat(ptr, "\r\n\r\n");
	}
}
