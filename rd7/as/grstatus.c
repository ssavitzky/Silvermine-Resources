/* grstatus - rd7sp -displays status under spectrum plot */

/***********************************************************\
**	Displays full path on 7000 disk
** Spectrum Type:
**	hdr line
** status line
** Date:                Time:
** Ident:
***********************************************************/

#include <graph.h>
#include <stdlib.h>
#include <string.h>
#include <std.h>
#include "spec.h"
#include <io.h>
#include <stdio.h>


/* SPECTRUM TYPES */
char *spectype[] = {
	"Infrared",
	"Ultraviolet",
	"Luminescence",
	"IG",
	"??"};
char *timed[2] = {
	" ",
	": Time Drive"};

LOCAL TEXT hdr[] = {"   Points   From      To    Int      Min     Max    Indicators          Name\n"};
LOCAL TEXT flist[] = {"ALDF                "};    /* flag list */
LOCAL TEXT hdr40a[] = {"   Points   From      To    Int"};
LOCAL TEXT hdr40b[] = {"   Min     Max    Indicators         "};
/* Name\n"};*/


GLOBAL struct videoconfig vc;
LOCAL void stat40(int,char*,char*,int,int);
LOCAL void textout(int,int,int,char *);
VOID pstat(char *path7,				/* full path name on 7000 disk */
	KREC *pkrec,			 /*	pointer	to krec	of above region	*/
	int isgraph)					/* to handle IBM monochrome adapter */
	{
	INTERN DOUBLE f100 = 100.0;
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
	ULONG nflags;				 /*	flags */
	BYTES stob();				/* COUNT to	buffer */
	int td;                 /* timedrive flag */
	struct rccoord rcoord;	/* for _outtext, etc. */
	char *s;
	char dt[9];				/* for handling date,time as strings */
	char tm[9];
	char id[74];
	int ftext;				/* first line of text window */
	int is80;            /* distinguish CGA */


	/* Initialise */
	ndecpl = 2;		   /* Default display */
	nflags = pkrec->flags;

	/* set text window */
	is80 = (vc.numtextcols >= 80);
	ftext = vc.numtextrows - (is80?8:11);	 
	_settextwindow(ftext,1,vc.numtextrows-1,vc.numtextcols);
	_wrapon(_GWRAPON);		/* activate if need wrap for CGA */
	_settextcolor(07);

	/*	output filename */
	sprintf(koutln,"File: %s",path7);
	textout(isgraph,1,1,koutln);
	
	/* handle stype and sign of delta for IR */
	del	= (pkrec->ndel)/f100;
	if ((stype = pkrec->stype) < 2)
		del = -del;		/* display a positive data interval	*/

	if (stype >	9)
		{		
		stype -= 10;
		td = 1;
		}
	else
		td = 0;
	/* output spectrum type */
	i = (stype >= 0 && stype < 5) ? stype : 4;
	sprintf(koutln,"Spectrum Type: %s %s",spectype[i],timed[td]);
	textout(isgraph,2,1,koutln);

	sprintf(koutln,"Instrument: %i",(unsigned long)pkrec->instno);
	if (is80)
		textout(isgraph,2,45,koutln);
	else 
		textout(isgraph,3,1,koutln);

	/* zeros into koutln for later check and blank */
	for (i = 0; i < sizeof(koutln); i++)
		koutln[i] = ' ';

	temp = pkrec->istart/f100;
	temp1 =	pkrec->ifin/f100;
/* The following line doesn't work!!!!!!!!!!!!!!!!!!!!!!!!\
**	sprintf(koutln + 3,"%6i %8.1f %8.1f%6.2f",pkrec->npts,temp,temp1,del);
**** and the following does !!!!!!*/
	sprintf(koutln+3,"%6i",pkrec->npts);
	sprintf(koutln+10,"%8.1f %8.1f",temp,temp1);
	sprintf(koutln+26,"%6.2f",del);

/* replace trailing zeros after decimal point with spaces
   in start	and fin if any
*/
	if ((koutln[24]) == '0' && (koutln[16] == '0'))
		koutln[15] = koutln[16] = koutln[23] = koutln[24] = ' ';

	/* scale min,max */
	temp = (DOUBLE)pkrec->scale;  /*guarantee next calcs in	float */
	ymin = (pkrec->miny/temp);
	ymax = (pkrec->maxy/temp);

	if (((nflags & 1) && (stype	< 2)) || nflags	& 2)
		/* In absorbance or	LOG	*/
		ndecpl = 4;	   /* 4	dec	pl for non-lum absorbance values */
	else
		/* in transmittance	*/
		{
		/* convert into	% */
		ymin *=	f100;
		ymax *=	f100;
		}
	sprintf(koutln + 34,"%8.*f%8.*f",ndecpl, ymin, ndecpl, ymax);

/* write 'A	DF....'	depending on flag setting in flags
*/
	*(koutln + 51) = 'T';
	for	(i = 0,	j =	1; i < 12; i++)
		{
		if ((stype > 1)	&& (i == 0))
			*(koutln + 51) = ' ';
		else
			if (nflags & j)
				*(koutln + i + 51) = flist[i];
		j *= 2;
		}
	if(pkrec->enhwdth != 0 )
		*(koutln + 55) = 'E';
	if(pkrec->drvdel !=	0)
		{
		*(koutln + 56) = 'D';
		*(koutln + 57) = pkrec->drvord + '0';
		*(koutln + 58) = 'W';
		*(koutln + 59) = (pkrec->drvdel	/ 100.0) + '0';
		}
	/* store 'S' and smooth	no.	*/
	if (pkrec->nsmth)
		{
		*(koutln + 61) = 'S';
		*(koutln + 62) = pkrec->nsmth +	'0';
		}

	/* store 'A' and no. average or	accum. */
	if (pkrec->naccs)
		{
		*(koutln + 64) = 'A';
		*(koutln + 65) = (nflags & 0x4000)?	'V': 'C';
		sprintf(koutln	+ 66, "%-3i",pkrec->naccs);
		}

	strncpy(dt,pkrec->spname,8);	/* these two lines guarantee string */
	dt[8]='\0';
	sprintf(koutln + 70,"%8s",dt);
	/* suppress nulls, except final */
	for (i=0;i<79;i++)
		if (koutln[i] == '\0')
			koutln[i] = ' ';
		koutln[79] = '\0';
	if (is80)
		{
		textout(isgraph,3,1,hdr);
		textout(isgraph,4,1,koutln);
		}
	else
		{
		stat40(isgraph,hdr40a,koutln,34,4);
		stat40(isgraph,hdr40b,koutln+34,36,6);
		textout(isgraph,8,1,"Name: ");	/* probably ok, because isgraph */
		textout(isgraph,8,7,koutln + 70);
		}

	/* date, time */
	strncpy(dt,pkrec->date,8);
	dt[8]='\0';
	strncpy(tm,pkrec->time,8);
	tm[8]='\0';
	sprintf(koutln,"Date: %8s  Time:%8s",dt,tm);
	textout(isgraph,is80?5:9,1,koutln);
	/* IDENT */
	strncpy(id,pkrec->ident,72);
	s = id;
	for (i = 0; i < 72; i++, s++)
		if ((*s < ' ')||(*s > 0xe7))
			*s = ' ';
	*s = '\0';
	sprintf(koutln,"Ident: %72s",id);
	koutln[80]='\0';	/*guarantee not overwritten*/
	textout(isgraph,is80?6:10,1,koutln);

	return;
	}

/** output lines for CGA, 40 char text */
void stat40(int isgraph,
	char *hdr,
	char *strg,
	int nchars,
	int line)
	{
	char ctemp;

	textout(isgraph,line,1,hdr);
	ctemp = strg[nchars];
	strg[nchars] = '\0';
	textout(isgraph,line+1,1,strg);
	strg[nchars] = ctemp;
	}

/****************  text output for graphic or mono ******/
LOCAL void textout(
	int isgraph,
	int row,
	int col,
	char *strg)
	{
	if (isgraph)
		{
		_settextposition(row,col);
		_outtext(strg);
		}
	else
		puts(strg);			/* let newlines handle placement for now */
	}
