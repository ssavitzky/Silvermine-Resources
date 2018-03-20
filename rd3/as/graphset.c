/* graphset - determines graphics & sets up mode */

#include <graph.h>
#include <stdlib.h>
#include <string.h>
#include <std.h>

/* Structures for configuration	and other data */
GLOBAL struct videoconfig vc;

int graphset(void)
	{
	int isgraph;		/* use for return value */
	int vmode;

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

	if (!vc.numxpixels || !vc.numypixels || !isgraph)
		{
		printf("Graphics mode not available\n");
		isgraph = NO;		/* probably redundant, but makes sure */
		}
	return(isgraph);
	}

