/* thstatus.c - displays selected info below graph */

#include <stdlib.h>
#include <graph.h>
#include <string.h>
#include <ctype.h>
#include <std.h>
#include <io.h>
#include <stdio.h>
#include "pctherm.h"
#include "headers\antypes.h"

#define NORAMP 2147483647L
#define STATUSLINES 7

extern VCTRL far *vc;
extern SHEAD far *sh;
extern struct disk_dcmeth  far *dcm;
extern DMETH far *dm;		/* display method */
extern DHEAD far *dh;
extern HCMETH far *hcm;
extern float far *dataptr;

extern long npoints, nords;
extern char *antypes[];
extern struct videoconfig vcon;

void thstat(int isgraph)
	{
	char buffer[80];		/* temp loc for sprint */
	char date[24];
	int i;	
	int firstline;
	int antype;
	double dtemp;
	struct rccoord rcoord;

	firstline = (i = vcon.numtextrows) - STATUSLINES + 2;
	_settextwindow(firstline,1,i,80);
	_settextcolor(0x07);

	_settextposition(1,1);
	sprintf(buffer,"%s", &sh->sid[0]);
	_outtext(buffer);

	antype = hcm->antype - 48;
	if ((antype < 0)||(antype > 7))
		sprintf(buffer,"Analyzer type= %c",hcm->antype);
	else
		sprintf(buffer,"Data Type = %s",antypes[antype]);
	_settextposition(1,40);
	_outtext(buffer);

	udate(sh->datetime,date);
	sprintf(buffer,"Date= %s",date);
	_settextposition(2,1);
	_outtext(buffer);

	if (nords > 1)
		sprintf(buffer,"Points=%ld, Ordinates=%ld",npoints,nords);
	else
		sprintf(buffer,"Points=%ld",npoints);
	_settextposition(2, 40);
	_outtext(buffer);


	dtemp = (double)sh->sweight;
	sprintf(buffer,"Sample weight=%10.3f",dtemp);
	_settextposition(3, 1);
	_outtext(buffer);

	if (isTMA(hcm->antype) || isDMA(hcm->antype))
		{
		sprintf(buffer,"Sample Zero= %f",(double)sh->szero);
		_settextposition(4, 1);
		_outtext(buffer);
		sprintf(buffer,"Sample Height= %f",(double)sh->sheight);
		_settextposition(4, 40);
		_outtext(buffer);
		sprintf(buffer,"Sample Width= %f",(double)sh->swidth);
		_settextposition(5, 1);
		_outtext(buffer);
		sprintf(buffer,"Sample length= %f",(double)sh->slength);
		_settextposition(5, 40);
		_outtext(buffer);
		}
	}
