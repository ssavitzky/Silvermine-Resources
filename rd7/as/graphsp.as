/* graphsp - graph spectrum !!!!! */

#include <graph.h>
#include <stdlib.h>
#include <string.h>
#include <std.h>
#include "spec.h"
#include <io.h>
#include <stdio.h>

/* Structures for configuration	and other data */
GLOBAL struct videoconfig vc = {0};

/* Structure for menu attributes (variables for color and monochrome) */
struct mnuAtr {
	int	fgNormal, fgSelect, fgBorder;
	long	bgNormal, bgSelect, bgBorder;
	int     centered;
	char	nw[2], ne[2], se[2], sw[2], ns[2], ew[2];
};

struct mnuAtr menus = {
	0x00, 0x0f, 0x04,
	0x03, 0x04, 0x03,
	YES,
	"Ú", "¿", "Ù", "À", "³", "Ä"
};

struct mnuAtr bwmenus = {
        0x07, 0x00, 0x07,
        0x00, 0x07, 0x00,
	YES,
	"Ú", "¿", "Ù", "À", "³", "Ä"
};


/* Arrays for video modes */

char *mnuModes[] = {
	"MRES4COLOR ",
	"MRESNOCOLOR",
	"HRESBW",
	"MRES16COLOR",
	"HRES16COLOR",
	"ERESCOLOR",
	"VRES2COLOR",
	"VRES16COLOR",
	"MRES256COLOR",
	NULL };

static char presskey[] ={"  Press any key to continue"};

void graphsp(char * path7,
	KREC far *pkrec,
	LONG far *pdata)
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
	int vmode;
	double dmin;
	int isgraph;
	
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
			mnuModes[3] = NULL;
			vmode =	_MRES4COLOR;
			break;
		case _HGC :
			mnuModes[6] = NULL;
			vmode = _HERCMONO;
			break;
		case _EGA :
			mnuModes[6] = NULL;
			vmode = _ERESNOCOLOR;
			break;
		case _VGA :
			vmode = _VRES2COLOR;
			break;
		case _MCGA :
			vmode =	_MRES256COLOR;
			break;
	}
/* menus always BW because set nocolor above *******
	switch (vc.mode) {
		case _TEXTBW80 :
		case _TEXTBW40 :
		case _TEXTMONO :	/*??? as **
		case _HERCMONO :
			menus = bwmenus;
			break;
		case _ERESNOCOLOR :
			menus =	bwmenus;
			vmode = _ERESNOCOLOR;
	}
********************************************************/
	menus = bwmenus;
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
	pstat(path7,pkrec,menus,isgraph);

	if (isgraph)
		{
		_settextwindow (24,1,25,80);
		_settextcolor(0x07);
		_settextposition(25,1);
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