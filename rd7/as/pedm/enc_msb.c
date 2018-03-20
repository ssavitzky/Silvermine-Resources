/*----- modified by AS for rd7 use -------------------------------------*/

/************************************************************************/
/*									*/
/*			PEMSB Encoding Module				*/
/*			Version 1.1	10/9/87				*/
/*									*/
/* Description:	The following module is used encode the PEMSB  		*/
/*		specifc part of a spectrum krec	into the		*/
/*		standard file header format.				*/
/*									*/
/* Contents:	encode_pemsb()		Encode PEMSB specific   	*/
/*					Part of krec.			*/
/*									*/
/* Author:	T. J. Moberly						*/
/*		Perkin-Elmer Corporation				*/
/*		Oak Brook Software group				*/
/*		2OOO York Road Box 4776					*/
/*		Oak Brook, Illinois 60522				*/
/*									*/
/* Version 1.0:	T. J. Moberly 7/2/87					*/
/*		Initial implementaion of PEMSB encoding module.		*/
/*									*/
/* Version 1.1:	D. Kusswurm	10/9/87					*/
/*		Last x position value now formatted as a DOUBLE rather	*/
/*		than a FLOAT to eliminate rounding errors.		*/
/*									*/
/************************************************************************/

#include "portab.h"
#include "dmstd.h"		/* AS - to handle UTINY, etc in spec.h*/
#include "spec.h"
#include "dmspec.h"
#include "defs.h"
#include "error.h"
#include "slm.h"

/************************************************************************/
/*									*/
/* Function:	encode_pemsb	-Create the PEMSB specific part		*/
/*				 of the file header from a krec.	*/
/*									*/
/* Description:	The following function creates the PEMSB specific	*/
/*		part of the standard file header.			*/
/*									*/
/* Globals:	None.							*/
/*									*/
/* Parameters:	CHAR 	*ptr;		Pointer to header info.		*/
/*		KREC	*kptr; 		Pointer to krec to be saved.	*/
/*									*/
/* Input:	None.							*/
/*									*/
/* Output:	None.							*/
/*									*/
/* Calls:	ovl_load()		Load SLM overlay.		*/
/*		ovl_exec()		Execute SLM overlay		*/
/*		ovl_free()		Free overlay area.		*/
/*									*/
/* Returns:	RET_SUCCESS		Successful conversion.		*/
/*		RET_ERROR		Conversion error.		*/
/*									*/
/************************************************************************/

int douv(char*,KREC*);
int doir(char*,KREC*);
int dofl(char*,KREC*);

WORD
encode_pemsb(ptr, kptr)
CHAR	*ptr;				/* Pointer to header info	*/
KREC	*kptr;				/* Krec Pointer			*/
{
	CHAR	temp[80];		/* Temporary string holding	*/
	CHAR	slm_name[13];		/* Filename of discipline SLM	*/
	RWORD	ovl_handle;		/* Overlay handle number	*/
	RWORD	status;			/* Status return code		*/
	char *s;	            /* AS for ident manip ?? */
	int i;					/* counter */

	/****************************************************************/
	/* Put the PEMSB specific information to the header.		*/
	/****************************************************************/

	status = RET_SUCCESS;
	/* IDENT */
	strncpy(temp,kptr->ident,72);
	s = temp;
	for (i = 0; i < 72; i++, s++)
		if ((*s < ' ')||(*s > 0xe7))
			*s = ' ';			/* known to be garbage now & then */
	*s = '\0';
	while (s-- > temp)
		{
		if (*s == ' ')
			*s = '\0';
		else
			if (*s > ' ')
				break;
		}
	strcat(ptr, temp);		/* Ident line		*/
	strcat(ptr, "\r\n");

						/* Last X data point	*/
	sprintf(temp, "%-lf\r\n", (DOUBLE)kptr->ifin / 100.0);
	strcat(ptr, temp);

	if(kptr->naccs)
		{
		sprintf(temp, "%d\r\n", kptr->naccs);	/* No. of accumulations	*/
		strcat(ptr, temp);
		}
	else
		strcat(ptr,"1\r\n");

	/****************************************************************/
	/* The rest of this header consists of the evaluation of the	*/
	/* command manipulation flags.					*/
	/****************************************************************/

	if (kptr->flags & FLG_MOD)		/* Math			*/
		strcat(ptr, "MATH");
/************************************ AS - math is all
	if (kptr->manip & FLG_ADD)
		strcat(ptr, "ADD");
	if (kptr->manip & FLG_SUB)
		strcat(ptr, "SUB");
	if (kptr->manip & FLG_MULT)
		strcat(ptr, "MULT");
	if (kptr->manip & FLG_DIV)
		strcat(ptr, "DIV");
**************************************************/
	strcat(ptr, "\r\n");

/* AS: unable to indicate diff performed, since CDS only has flag */
/*	if (kptr->flags & FLG_DIFF)		/* Diff			** 
	{
		sprintf(temp, "%lf,%lf", kptr->diffactor, kptr->dffct_offset);
		strcat(ptr, temp);
	}
*/
	strcat(ptr, "\r\n");

/* AS: CDS only has flag, default numeric to 1 */
	if (kptr->flags & FLG_FLAT)		/* Flat			*/
	{
		sprintf(temp, "%s,%d", "FLAT",1);
		strcat(ptr, temp);
	}
	strcat(ptr, "\r\n");

	if (kptr->nsmth)		/* Smooth  1=always sav-gol in cds **/
	{
		sprintf(temp, "%d,%lf", 1, (DOUBLE)kptr->nsmth);
		strcat(ptr, temp);
	}
	strcat(ptr, "\r\n");

	if (kptr->absex)		/* Abex			*/
	{
		sprintf(temp, "%lf,%lf",(DOUBLE)kptr->absex,(DOUBLE)0);
		strcat(ptr, temp);
	}
	strcat(ptr, "\r\n");

/* may not be present in all cds ????*/
	if (kptr->drvord && kptr->drvdel)		/* Deriv		*/
	{
		sprintf(temp, "%d,%lf", kptr->drvord,(DOUBLE)kptr->drvdel/100.);
		strcat(ptr, temp);
	}
	strcat(ptr, "\r\n");

	if (kptr->enhfact && kptr->enhwdth)		/* Enhance		*/
	{
		sprintf(temp, "%lf,%lf",(DOUBLE)kptr->enhfact,(DOUBLE)kptr->enhwdth);
		strcat(ptr, temp);
	}
	strcat(ptr, "\r\n");

/*	if (kptr->manip & FLG_NORM)		/* Normalize		** NOT in CDS--
	{
		sprintf(temp, "%f,%ld,%lf", kptr->norm_wave / 100.0, kptr->norm_offset, kptr->norm_factor);
		strcat(ptr, temp);
 	}
*/
	strcat(ptr, "\r\n");

/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!! AS: I'm off one at this point and can't resolve, so:::: 
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
	strcat(ptr, "\r\n");

	/****************************************************************/
	/* Calls to encode the discipline specific info.	*/
	/****************************************************************/

	switch (kptr->stype)
	{
		case UV + 10:
		case UV	:	status = douv(ptr,kptr);
				break;

		case IR + 10:
		case IR	:	status = doir(ptr,kptr);
				break;

		case FL + 10:
		case FL	:	status = dofl(ptr,kptr);
	}

	return(status);
}

/* dummy line filler for inst stuff */
void dodummy(char *ptr,KREC *kptr)
	{
	CHAR	temp[80];		/* Temporary string holding	*/
	int i;

	sprintf(temp,"%u\r\n",kptr->instno);	/*instrument number*/
	strcat(ptr,temp);
	/* tack on lines till 42 */
	for (i = 22; i < 42; i++)
		strcat(ptr,"\r\n");
	}

/* dummy inst for now */
int douv(char *ptr,KREC *kptr)
	{
	dodummy(ptr,kptr);
	return(RET_SUCCESS);
	}
int doir(char *ptr,KREC *kptr)
	{
	dodummy(ptr,kptr);
	return(RET_SUCCESS);
	}
int dofl(char *ptr,KREC *kptr)
	{
	dodummy(ptr,kptr);
	return(RET_SUCCESS);
	}


