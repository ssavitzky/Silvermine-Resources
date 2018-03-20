/* graphth - graph thermal data !!!!! */

#include <graph.h>
#include <stdlib.h>
#include <string.h>
#include <std.h>
#include "pctherm.h"
#include <io.h>
#include <stdio.h>

extern VCTRL far *vc;
extern SHEAD far *sh;
extern struct disk_dcmeth  far *dcm;
extern DMETH far *dm;		/* display method */
extern DHEAD far *dh;
extern HCMETH far *hcm;

extern long npoints, nords;
#define STATUSLINES 7

void initData(FILE *);
float gNextFloat(void);

GLOBAL struct videoconfig vcon = {0};

static char presskey[] ={"  Press any key to continue"};

int graphth(FILE *infile, FILE *outf)
	{
	double fscale;		
	double fy;
	double fx;
	double fpoints;	/* npoints/xpixels */
	int i,j,k;
	int kmax;
	long idata;			/* data range, max-min */
	short xpixels;
	short ypixels;
	short yorigin;		/* graph position for ymin*/
	short x,y,yp2;		/* yp2 confines graph to upper half of screen */
	int vmode;			/* video mode for this graph */
	double dmin;
	int isgraph;
	double yfactor;
	double xfactor;
	double fypixels;
	double ymin,ymax, yrange;
	double xmin,xmax, xrange;
	float *fpointer;                 /* pointer to current value for min, max */
	float far *ydata[MAX_ORD + 1];   /*Point to start of each ord*/
									/* actually ydata[0] contains x values*/
	long np = npoints;
	int nordsp1 = nords + 1;
	long datasize;
	float f;                /*for gNextFloat */
	int arrow;
	char tbuffer[40];

	/* make sure the data is in the right format */
/*NOT  NEEDED FOR THERMAL ?????????????????????????*****************
	if (inPCformat) {
		pkrec = (KREC *) buf;
		pdata = (LONG *) (buf + sizeof(KREC));
	} else {
		pkrec = &skrec;
		pdata = spset(buf, pkrec);
		swapl (pdata, pkrec->npts);
	}
************************************************************/

	/******************* SET UP GRAPHICS ********************/

	/* find out what we have */
	_setvideomode (_DEFAULTMODE);
	_getvideoconfig(&vcon);

	/* Select best text and	graphics modes */
	/* modeled after grdemo.c */

	isgraph = YES;		/* only MDA is NO */
	switch (vcon.adapter) {
		case _MDPA :
			isgraph = NO;
			vmode = _TEXTMONO;
			break;
		case _CGA :
			vmode =	_MRES4COLOR;
			break;
		case _HGC :
			vmode = _HERCMONO;
			break;
		case _EGA :
			if (vcon.memory > 64) vmode = _ERESCOLOR;
			else vmode = _HRES16COLOR;
			break;
		case _VGA :
		case _MCGA :
			vmode =	_VRES2COLOR;
			break;
	}
	/* I'm uncomfortable with _ERESNOCOLOR but... */
	if (vcon.mode == _ERESNOCOLOR && vmode != _HERCMONO)
		vmode = _ERESNOCOLOR;

	_setvideomode(vmode);  /*with luck, this is it */
	_getvideoconfig(&vcon);
	xpixels = vcon.numxpixels;
	ypixels = vcon.numypixels;
	yp2 = ypixels/2;

	if (!xpixels || !ypixels || !isgraph)
		{
		printf("Graphics mode not available\n");
		isgraph = NO;		/* probably redundant, but makes sure */
		}

	/***************FINISHED GRAPHICS SET UP *********************/

/* initialize data settings */
	/* amount of space needed to store x values and all ordinates*/
	datasize = xpixels * 4;		/* no. bytes per ord */
	for (i = 0; i < nordsp1; i++)
		{
		if (!(ydata[i] = (float far *)malloc(datasize)))
			{
			_setvideomode(_DEFAULTMODE);
			printf("not enough space for graphic data\n");
			exit(10);
			}
		}

	/* do status in either graphic or mono mode **********/
	thstat(isgraph);                  

	/****************  haul in the points and put into malloc area */
	fseek(infile,vc->doff,SEEK_SET);

	initData(infile);


	/* set no. of x values per horizontal pixel */
	fpoints = (double)xpixels/(double)npoints;

	k = -1;
	for (i = 0; i < np ; i++)
		{
		if (np <= xpixels)
			k++;
		else
			k = (int)(fpoints * (double)i);
		if (k >= xpixels)		/* don't overrun */
			break;
		for ( j = 0; j < nordsp1; j++)
		       *(ydata[j] +k) = gNextFloat();
		}
	kmax = min(k, xpixels);

/* DEBUG ***************************************
	isgraph = NO;
	for (j = 0; j < nordsp1; j++)
		{
		for (k = 0; k < kmax; k++)
			fprintf(outf," %10.6f\n",*(ydata[j] + k));
		}
*/

	/* determine range  and scaling for xdata */
	fpointer = ydata[0];
	xmin = xmax = *fpointer++;
	for (k = 1; k < kmax; k++)
		{
		fx = *fpointer++;
		if (fx < xmin)
			xmin = fx;
		if (fx > xmax)
			xmax = fx;
		}
	xfactor = (double)xpixels/(xmax - xmin);

	/* set yorigin above status area, i.e. STATUSLINES*/
	/* note integer arithmetic !*/
	yorigin = ypixels - ((ypixels/vcon.numtextrows) * STATUSLINES);
	_setviewport(0,0,xpixels,yorigin);

/* do the plot only if isgraph */
	j = 1; 
 nextj:		/* use to loop j on up/down arrow */
	if (isgraph)
		{
		/* determine range  and scaling for ydata */
		fpointer = ydata[j];
		ymin = ymax = *fpointer++;
		for (k = 1; k < kmax; k++)
			{
			fy = *fpointer++;
			if (fy < ymin)
				ymin = fy;
			if (fy > ymax)
				ymax = fy;
			}
		if (ymax == ymin)
			{
			if (ymin)
				{
				ymin = ymin/2.;
				yfactor = (double)yorigin/ymax;
				}
			else
				{
				ymin = -.5;
				yfactor = (double)yorigin/2.;
				}
			}
		else
			yfactor = (double)yorigin/(ymax - ymin);

		/* move to first point */
		x = (*(ydata[0]) - xmin) * xfactor;
		y = yorigin - ((*(ydata[j]) - ymin) * yfactor);
		_moveto(x,y);
		k = 1;

		for (; k < kmax; k++)
			{
			x = (*(ydata[0] + k) - xmin) * xfactor;
			y = yorigin - ((*(ydata[j] + k) - ymin) * yfactor);
			_lineto(x,y);
			}

		i = vcon.numtextrows;		/* may be 25 or 40 or?? */
		_settextwindow (i-1,1,i,80);
		_settextcolor(0x07);
		_settextposition(i,1);
		_outtext(presskey);
		if (nords > 1)
			{
			_settextposition(i,40);
			sprintf(tbuffer,"Ordinate= %d, \030\031 to change",j);
			_outtext(tbuffer);
			}
		while (!kbhit());
		if (!(arrow = getch()))	/* so arrow contains quit char */
			{
			arrow = getch();
			_clearscreen(_GVIEWPORT);
			if (arrow == 72)
				{
				j++;
				if (j > nords) j = 1;
				goto nextj;
				}
			if (arrow == 80)
				{
				j--;
				if (j < 1) j = nords;
				goto nextj;
				}
			}
		_setvideomode(_DEFAULTMODE);
		}

	else
		{
		printf(presskey);
		while (!kbhit());
		arrow = getch();
		}
	if ((arrow == 'q') || (arrow == 'n'))
		return(1);
	else
		return(0);	
	}

