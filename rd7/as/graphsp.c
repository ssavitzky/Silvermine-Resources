/* graphsp - graph spectrum !!!!! */

#include <graph.h>
#include <stdlib.h>
#include <string.h>
#include <std.h>
#include "spec.h"
#include <io.h>
#include <stdio.h>

/* === external routines that return non-ints MUST be declared! === */

extern LONG far *spset();


/* Structures for configuration	and other data */
GLOBAL struct videoconfig vc = {0};


static char presskey[] ={"  Press any key to continue"};

void graphsp(char * path7,
	char far *buf,
	int inPCformat)
	{
	KREC far *pkrec;
	KREC skrec;
	LONG far *pdata;
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
	int vmode;
	double dmin;
	int isgraph;
	
	/* make sure the data is in the right format */

	if (inPCformat) {
		pkrec = (KREC *) buf;
		pdata = (LONG *) (buf + sizeof(KREC));
	} else {
		pkrec = &skrec;
		pdata = spset(buf, pkrec);
		swapl (pdata, pkrec->npts);
	}
	/* find out what we have */
	_setvideomode (_DEFAULTMODE);
	_getvideoconfig(&vc);

	/* Select best text and	graphics modes */
	/* modeled after grdemo.c */

	isgraph = YES;		/* only MDA is NO */
	switch (vc.adapter) {
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
			if (vc.memory > 64) vmode = _ERESCOLOR;
			else vmode = _HRES16COLOR;
			break;
		case _VGA :
		case _MCGA :
			vmode =	_VRES2COLOR;
			break;
	}
	/* I'm uncomfortable with _ERESNOCOLOR but... */
	if (vc.mode == _ERESNOCOLOR && vmode != _HERCMONO)
		vmode = _ERESNOCOLOR;

	_setvideomode(vmode);  /*with luck, this is it */
	_getvideoconfig(&vc);
	xpixels = vc.numxpixels;
	ypixels = vc.numypixels;
	yp2 = ypixels/2;

	if (!xpixels || !ypixels || !isgraph)
		{
		printf("Graphics mode not available\n");
		isgraph = NO;		/* probably redundant, but makes sure */
		}

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
		y = yp2 - (int)((double)pdata[0]*fscale - dmin);
		x = 0;
		_moveto(x,y);

		/* set number of points per horizontal pixel */
		fpoints = (double)npoints/(double)xpixels;
		/* this is crude display, so no interpolation */
		for ( ;x < xpixels; x++)
			{
			i = (int)((double)x*fpoints);
			idata=pdata[i];
			y = yp2 - (int)((double)idata*fscale - dmin);
			_lineto(x,y);
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
	getch();
	return;	
	}



