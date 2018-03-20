/* graphsp - graph spectrum !!!!! */
/* assumes that getNext is pointing to first data point */

#include <graph.h>
#include <stdlib.h>
#include <string.h>
#include <std.h>
#include "..\..\as\spec.h"
#include <io.h>
#include <stdio.h>

/* === external routines that return non-ints MUST be declared! === */
int getNext3s(void);		/*get next 2 bytes & return short */

/* Structures for configuration	and other data */
GLOBAL struct videoconfig vc;


static char presskey[] ={"  Press any key to continue"};

int graph3sp(char *path7,
	KREC far *pkrec)
	{
	double fscale;
	double fy;
	double fx;
	double fpoints;	/* npoints/xpixels */
	int i,j,k;
	long idata;
	long npoints;
	short xpixels;
	short ypixels;
	short x,y,yp2;
	double dmin;
	int isgraph;

	/* setup graphics */
	isgraph = graphset();
	xpixels = vc.numxpixels;
	ypixels = vc.numypixels;
	yp2 = ypixels/2;

/* initialize spectral settings */
	idata = pkrec->maxy - pkrec->miny;
	if (idata < 100 )
		fscale = (double)yp2/(double)pkrec->scale;
	else
		fscale = (double)yp2/(double)idata;
	dmin = (double)pkrec->miny * fscale;

	npoints = pkrec->npts;

/* do the plot only if isgraph */
	if (isgraph)
		{
		/* move to first point */
		idata = getNext3s();
		y = yp2 - (int)((double)idata*fscale - dmin);
		x = 0;
		_moveto(x,y);

		if (npoints > xpixels)
			{
			/* set number of points per horizontal pixel */
			fpoints = (double)npoints/(double)xpixels;
			j = 1;		/*point count*/
			/* this is crude display, so no interpolation */
			for ( ;x < xpixels; x++)
				{
				i = (int)((double)x*fpoints);	  /* next plottable point */
				while (j++ < i)
					idata = getNext3s();
				y = yp2 - (int)((double)idata*fscale - dmin);
				_lineto(x,y);
				}
			}
		else
			{
			/* set no. xpixels per datapoint, no interp */
			fpoints =(double)xpixels/(double)npoints;
			for (j = 1; j < npoints; j++)
				{
				x = (int)((double)j * fpoints);
				idata = getNext3s();
				y = yp2 - (int)((double)idata*fscale - dmin);
				_lineto(x,y);
				}
			}
		}

	/* do status in either graphic or mono mode */	
	pstat(path7,pkrec,isgraph);
	
	if (isgraph)
		{
		i = vc.numtextrows;		/* may be 25 or 40 or?? */
		_settextwindow (i-1,1,i,80);
		_settextcolor(0x07);
		_settextposition(i,1);
		_outtext(presskey);
		while (!kbhit());
		_setvideomode(_DEFAULTMODE);
		}
	else
		{
		printf(presskey);
		while (!kbhit());
		}
	/* get rid of that key, might cause trouble */
	i = getch();
	if (i == 0) {
		getch();
		return(0);
	}
	if (i == 'q' || i == 'n')
		return(1);       /* for now, later return(getch()) */
	else
		return(0);
	}



