/* rd7 version - file header encoding module for PEDM */
/* Based on:                                          */

/************************************************************************/
/*									*/
/*			File Header Encoding Module			*/
/*			Version 1.1	10/8/87				*/
/*									*/
/* Description:	The following module is used encode a spectrum krec	*/
/*		into a standard file header.				*/
/*									*/
/* Contents:	encode_header()		Create a header from a krec.	*/
/*									*/
/* Author:	T. J. Moberly						*/
/*		Perkin-Elmer Corporation				*/
/*		Oak Brook Software group				*/
/*		2OOO York Road Box 4776					*/
/*		Oak Brook, Illinois 60522				*/
/*									*/
/* Version 1.0:	T. J. Moberly 7/2/87					*/
/*		Initial implementaion of file header module.		*/
/*									*/
/* Version 1.1:	D. Kusswurm	10/8/87					*/
/*		Field width specifier removed in format string for	*/
/*		ordinate minimum and maximum.  Abscissa start value	*/
/*		istart now formatted as a DOUBLE rather	than a FLOAT	*/
/*		to eliminate rounding errors.				*/
/*									*/
/************************************************************************/

#include "portab.h"
#include "dmstd.h"		/* AS - to handle UTINY, etc in spec.h*/
#include "spec.h"
#include "dmspec.h"
#include <time.h>

/************************************************************************/
/*									*/
/* Function:	encode_header	-Create a file header from a krec.  	*/
/*									*/
/* Description:	The following function creates a standard file header	*/
/*		from the specified krec.				*/
/*									*/
/* Globals:	None.							*/
/*									*/
/* Parameters:	CHAR 	*ptr;		Pointer to header info.		*/
/*		KREC	*kptr;		Pointer to spec to be saved.	*/
/*		WORD	status;		Status return code.		*/
/*									*/
/* Input:	None.							*/
/*									*/
/* Output:	None.							*/
/*									*/
/* Calls:	encode_pemsb()		Encode spectroscopy specific.	*/
/*									*/
/* Returns:	None.							*/
/*									*/
/************************************************************************/
/*----------------------------------------------------***
**	A.S:	these are compression codes, depending on dtype,
**		see dmdo pkrec->dtype switch
** 	default for 0 is 4-byte flipped
**----------------------------------------------------***/
int compcode[5] = {8,1,6,3,8};
char sp[] = ".SP";
char nodate[] = "NO_DATE_";
char notime[] = " NO_SYS_TIME_";

VOID
encode(ptr, kptr, status)
CHAR	*ptr;				/* Pointer to header info	*/
KREC	*kptr;				/* Krec Pointer			*/
WORD	*status;			/* Status return code		*/
{
	RWORD	encode_status;		/* Encode status code		*/
	CHAR	temp[80];		/* AS: increase Scratchpad buffer area	*/
	char *s1,*s2;		/* AS: for string transfers*/
	int i;			/* AS: counter */
	char c;        /* AS: temp char */
	DOUBLE scale = 100./(DOUBLE)kptr->scale;	/* AS: I think!! */
	/****************************************************************/
	/* Create the first 2 records of the Primary Header.		*/
	/****************************************************************/

	strcpy(ptr, "PE ");
	switch(kptr->stype)
	{
		case	IR: strncpy(ptr + 3, "IR       ",9);
			   break;

		case	UV: strncpy(ptr + 3, "UV       ",9);
			   break;

		case	FL: strncpy(ptr + 3, "FL       ",9);
			   break;

		case	IR + 10	: strncpy(ptr + 3, "IRTD     ",9);
			  	 break;

		case	UV + 10	: strncpy(ptr + 3, "UVTD     ",9);
				   break;

		case	FL + 10	: strncpy(ptr + 3, "FLTD     ",9);
				   break;
	}


	strncpy(ptr + 12, "Subtech     ",12);
	strncpy(ptr + 24, "SPECTRUM    ",12);
	strncpy(ptr + 36, "BINARY      ",12);
	strncpy(ptr + 48, "rd7         ",12);
	strncpy(ptr + 60, "0.00        \r\n",14);
	strcat(ptr, "  -1\r\n");

	/****************************************************************/
	/* Now create the remaining part of the Primary header.		*/
	/****************************************************************/
	/* AS: in CDS, spname usually blank filled */
	s1 = kptr->spname;
	s2 = temp;
	for (i = 0; i<sizeof(kptr->spname);i++)
		{
		c = *s1++;
		if(c==' ' || c=='\0')
			break;
		*s2++ = toupper(c);
		}
	*s2 = '\0';
	strcat(temp,sp);	/* upper case name, then tack on .SP*/
	strcat(ptr, temp);		/* Sample id ??	*/
	strcat(ptr, "\r\n");

	/* CDS file may not have a date, time */
	if (!kptr->date[0]) /* no date, so move in nodate, notime */
		{
		strncpy(kptr->date,nodate,8);
		strncpy(kptr->time,notime,12);
		}
		
	strncpy(temp, kptr->date,8);		/* Data creation date	*/
	temp[8] = '\0';
	strcat(ptr, temp);
	strcat(ptr, "\r\n");

	strncpy(temp, kptr->time + 1,11);		/* Data creation time	*/
	temp[11] = ' ';
	temp[12] = '\0';
	strcat(ptr, temp);
	strcat(ptr, "\r\n");

	strncpy(temp, kptr->date,8);		/* Last modified date	*/
	temp[8] = '\0';
	strcat(ptr, temp);
	strcat(ptr, "\r\n");

	strncpy(temp, kptr->time + 1,11);		/* Last modified time	*/
	temp[11] = ' ';
	temp[12] = '\0';
	strcat(ptr, temp);
	strcat(ptr, "\r\n");

/*	strncat(ptr, kptr->analyst, 21);	/* Analyst name		**
*/
	strcat(ptr,"From 7000 disc");
	strcat(ptr, "\r\n");

	/****************************************************************/
	/* Encode the PEMSB and discipline specific info into the 	*/
	/* primary header.						*/
	/****************************************************************/

	encode_status = encode_pemsb(ptr, kptr);

	/****************************************************************/
	/* Add the Secondary Header information.               		*/
	/****************************************************************/

	strcat(ptr, "#HDR\r\n");
	strcat(ptr, "-1\r\n");
	strcat(ptr, "-1\r\n");

	/****************************************************************/
	/* Add the information for the Graphics header.        		*/
	/****************************************************************/

	strcat(ptr, "#GR\r\n");			/* X units		*/
	if (kptr->abscid[0])
		strcat(ptr, kptr->abscid);
	else 
		{
		if (kptr->stype >= 10)
			strcat(ptr,"SEC");
		else
			if (kptr->stype == 0)
				strcat(ptr,"CM-1");
			else 
				strcat(ptr,"NM");
		}
	strcat(ptr, "\r\n");

	if (kptr->ordid[0])
		strcat(ptr, kptr->ordid);		/* Y units		*/
	else
		{
		switch(kptr->stype)
			{
			case	IR: 
			case  IR + 10:
			case	UV:
			case	UV + 10	:
				if (kptr->flags & FLG_TA)
					strcat(ptr,"ABS");
				else
					strcat(ptr,"%T");
				break;

			case	FL:
			case	FL + 10	: strcat(ptr,"ENER");
				   break;
			}
		}
	strcat(ptr, "\r\n");

						/* User Y scaling	*/
	sprintf(temp, "%-1.21lf\r\n", scale);
	strcat(ptr, temp);

	strcat(ptr, "0\r\n");			/* User Y offset	*/

						/* First X		*/
	sprintf(temp, "%-lf\r\n", (DOUBLE)kptr->istart / 100.0);
	strcat(ptr, temp);

						/* Delta X		*/
	sprintf(temp, "%-1.2f\r\n", (FLOAT)kptr->ndel / 100.0);
	strcat(ptr, temp);

	sprintf(temp, "%ld\r\n", kptr->npts);	/* Number of points	*/
	strcat(ptr, temp);

			/* Compresion technique	*/
	sprintf(temp,"%d\r\n",compcode[kptr->dtype]);
	strcat(ptr, temp);

						/* Maximum ordinate val	*/
	sprintf(temp, "%-lf\r\n", (DOUBLE)kptr->maxy * scale);
	strcat(ptr, temp);

						/* Minimum ordinate val */
	sprintf(temp, "%-lf\r\n", (DOUBLE)kptr->miny * scale);
	strcat(ptr, temp);

	strcat(ptr, "#DATA\r\n");

	/****************************************************************/
	/* Set the return status code.					*/
	/****************************************************************/

	*status = encode_status;
}
