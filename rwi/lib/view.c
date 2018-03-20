/*********************************************************************\
**
** View.c	--	View data on the screen
**
**	880422 SS	major changes:  view data structure
**	880111 SS	create PC version from DD
**
\*********************************************************************/

#include <stdio.h>
#include "curse.h"
#include "ibmchars.h"

#include "coops.h"
#include "tree.h"
#include "dir.h"
#include "viewer.h"

#undef  global
#define global

#include "view.h"


extern Object objInit(), objClone(), objKill();
extern Object objDoesNotImplement();

/*
** Things useful for formatting
*/
static char buf[81];

/*********************************************************************\
**
** Line-Drawing Routines
**
\*********************************************************************/

hline (win, row, width, beg, mid, end)
	WINDOW *win;
	int row, width;
	char beg, mid, end;
{
	register int i;
	wmove(win, row, 0);
	waddch(win, beg);
	for (i = 1; ++i < width; ) waddch(win, mid);
	waddch(win, end);
}

vlines (win, row, width, height, chr)
	WINDOW *win;
	int row, width, height;
	char chr;
{
	register int i;
	for (i = 0; i < height; ++i, ++row) {
		wmove(win, row, 0);
		waddch(win, chr);
		wmove(win, row, width - 1);
		waddch(win, chr);
	}
}


/*********************************************************************\
**
** V I E W S
**
\*********************************************************************/

/*********************************************************************\
**
** Utilities
**
\*********************************************************************/

/*
** Advance a viewer by +-n, and return the unused remainder.
*/
static long vrAdvance(vr, n)
	Viewer vr;
	long   n;
{
	if (n > 0) {
		for ( ; n > 0; --n) if (!gVrNext(vr)) break;
	} else {
		for ( ; n < 0; ++n) if (!gVrPrev(vr)) break;
	}
	return (n);
}

/*
** Un-hilite a row
*/
static void unHilite(v, line)
	View v;
	int	 line;
{
	register int c, ch;
	WINDOW	*win = v -> view.win;

	wmove(win, line, 0);
	for (c = 0; c < v -> view.cols; ++c) {
		ch = winch(win, line, c);
		waddch(win, ch);
	}
}


/*********************************************************************\
**
** Generic View Operations
**
\*********************************************************************/

static Object vInit(v)
	View v;
{
	extern int swidth;
	char ul = D_LEFT, ur = D_RIGHT, ll = D_LL, lr = D_LR;
	ushort nrows = v -> view.rows;
	ushort ncols = v -> view.cols;
	WINDOW *box;
	
	box = newwin(nrows + 2, ncols + 2, 
						 v -> view.org_y - 1, v -> view.org_x - 1);
	if (v -> view.org_x > 1) {
		ul = D_TOP; 
		ll = D_BOT;
	}
	if (v -> view.org_x + ncols + 2 < swidth) {
		ur = D_TOP;
		lr = D_BOT;
	}
	v -> view.box = box;
	v -> view.win = subwin(box, nrows, ncols, 1, 1);
	v -> view.x = v -> view.y = 0;
	wclear(box);
	hline (box, 0, ncols + 2, ul, D_HORIZ, ur);
	vlines (box, 1, ncols + 2, nrows, D_VERT);
	hline (box, nrows + 1, ncols + 2, ll, D_HORIZ, lr);
	return ((Object) v);
}

static String vName(v)
	View v;
{
	return(v -> view.name);
}

static void vRefresh(v)
	View v;
{
	if (v -> view.do_update) gVuUpdate(v);
	wrefresh(v ->view.box);
	v -> view.do_refresh = FALSE;
}

static void vLabel(v, s, g)
	View v;
	String s;
	String g;
{
	register WINDOW *win = v -> view.win;
	register int i;
	int x = win -> begx;
	int y = win -> begy - 1;		/* in the box */
	int w = win -> maxx;
	int h = win -> maxy;

	win = v -> view.box;
	v -> view.do_refresh = TRUE;

	/*
	** First char. of geometry is =, +, or - for center, left, right.
	** If =, second char is padding if present.
	** Find out where the window is relative to its box.
	** Set position accordingly.
	*/
	if (!g) g = "=";

	switch (*g) {
	 case '=':
	 	if (g[1]) {
			wmove(win, y, x);
			for (i = 0; i < w; ++i) waddch(win, g[1]);
		}
	 	wmove(win, y, x + (w - strlen(s)) / 2);
		break;
	 case '+':
		wmove(win, y, x);
		break;
	 case '-':
	 	wmove(win, y, x + w - strlen(s));
		break;
	}
	if (v -> view.is_active) wstandout(win);
	waddstr(win, s);
	if (v -> view.is_active) wstandend(win);
}

static Object vOpen(v)
	View v;
{
	gVuLabel(v, gName(v), "=\315");
	gVuRefresh(v);
	return ((Object) v);
}

static void vPgUp(v)
	View v;
{
	gVuMvLns(v, (long) - (v -> view.rows + v -> view.y));
	gVuRefresh(v);
}

static void vPgDn(v)
	View v;
{
	gVuMvLns(v, (long) (2 * v -> view.rows - v -> view.y - 1));
	gVuRefresh(v);
}

static void vFirst(v)
	View v;
{
	v -> view.x = v -> view.y = 0;
	gVrRewind(v -> view.top);
	gVrCopy(v -> view.cur, v -> view.top);
	v -> view.do_update = v -> view.do_refresh = TRUE;
	gVuRefresh(v);
}

static void vLast(v)
	View v;
{
	int n;

	gVuMvLns(v, 320000L);
	v -> view.do_update = v -> view.do_refresh = TRUE;
	gVuRefresh(v);
}

static void vSet(v, vr)
	View v;
	Viewer vr;
{
	Viewer r = v -> view.root;

	if (r == vr) {
		/* simplest case: just resetting */
		gVrCopy(v -> view.top, vr);
	} else if (NOTNULL(r) && fClass(r) == fClass(vr)) {
		/* replace with same type */
		gKill(r);
		v -> view.root = vr;
		gVrCopy(v -> view.top, vr);
	} else {
		/* new: kill old root, etc. if present, and make new ones */
		if (r) gKill(r);
		if (v -> view.top) gKill(v -> view.top);
		if (v -> view.cur) gKill(v -> view.cur);
		if (v -> view.tmp) gKill(v -> view.tmp);
		v -> view.root = vr;
		v -> view.top = gClone(vr);
		v -> view.cur = gClone(vr);
		v -> view.tmp = gClone(vr);
	}
	/* this is vFirst without the refresh at the end. */
	v -> view.x = v -> view.y = 0;
	gVrRewind(v -> view.top);
	gVrCopy(v -> view.cur, v -> view.top);
	v -> view.do_update = v -> view.do_refresh = TRUE;
}


/*********************************************************************\
**
** Generic Update Operations
**
**		vUpdLine may be called with y = -1 or y = rows, to
**		indicate scrolling.  However, any necessary adjustments to
**		the top and cur viewers must already have been done.
**
\*********************************************************************/

static void vUpdate(v)
	View v;
{
	register int r, c;
	char 	*p;
	WINDOW	*win = v -> view.win;
	Bool     hl	 = fClass(v) == (Class)clLineView; /* hilite current line? */
												   /* === kludge alert === */
		  
	v -> view.do_update = FALSE;
	v -> view.do_refresh= TRUE;
	if (!v -> view.is_active) hl = FALSE;

	r = 0;
	wmove(win, r, 0);
	gVrCopy(v -> view.tmp, v -> view.top);
	for (r = 0; r < v -> view.rows; ++r) {
		wmove(win, r, 0);
		if (hl && r == v -> view.y) wstandout(win);
		p = gVrString(v -> view.tmp, v -> view.cols);
		if (v -> view.is_centered) {
			for (c = (v -> view.cols - strlen(p)) / 2; c > 0; --c)
				waddch(win, ' ');
		}
		waddstr(win, p);
		wclrtoeol(win);
		if (hl && r == v -> view.y) wstandend(win);
		if (!gVrNext(v -> view.tmp)) break;
	}
	for (++r; r < v -> view.rows; ++r) {
		wmove(win, r, 0);
		wclrtoeol(win);
	}
	wrefresh(v -> view.win);
}

static void vUpdLine(v)
	View v;
{
	register int c;
	char 	*p;
	WINDOW *win = v -> view.win;
	Bool    hl	= fClass(v) == (Class) clLineView; /* hilite current line? */
												   /* === kludge alert === */

	if (v -> view.do_update) return;
	v -> view.do_refresh = TRUE;
	if (!v -> view.is_active) hl = FALSE;

	if (v -> view.y == -1) { 						/* scroll down */
		wmove(win, 0, 0);
		winsertln(win);
		v -> view.y += 1;
	} else if (v -> view.y == v -> view.rows) {		/* scroll up */
		wmove(win, 0, 0);
		wdeleteln(win);
		v -> view.y -= 1;
	}

	wmove(win, v -> view.y, 0);
	if (hl) wstandout(win);
	p = gVrString(v -> view.cur, v -> view.cols);
	if (v -> view.is_centered) {
		for (c = (v -> view.cols - strlen(p)) / 2; c > 0; --c)
			waddch(win, ' ');
	}
	waddstr(win, p);
	wclrtoeol(win);
	if (hl) wstandend(win);

	wrefresh(v -> view.win);
}

static void vMvLns(v, n)
	View v;
	long n;
{
	int  r;
	int oldRow = v -> view.y;
	Bool hl	= fClass(v) == (Class) clLineView; /* hilite current line? */
		 									   /* === kludge alert === */

	if (ISNULL(v -> view.root)) return;
	v -> view.do_refresh= TRUE;
	r = v -> view.y + n - vrAdvance(v -> view.cur, n);
	v -> view.y = r;

	if (v -> view.do_update) {						/* already refreshing */
		/* fall through to fixup */
	} else if (r >= 0 && r < v -> view.rows) {		/* on-screen */
		if (hl) unHilite(v, oldRow);
		vUpdLine(v);
		return;
	} else if (r == -1) {							/* off the top */
		if (hl) unHilite(v, oldRow);
		gVrCopy(v -> view.top, v -> view.cur);
		vUpdLine(v);
		return;
	} else if (r == v -> view.rows) {				/* off the bottom */
		if (hl) unHilite(v, oldRow);
		vrAdvance(v -> view.top, 1L);
		vUpdLine(v);
		return;
	} else {										/* too far: refresh */
		v -> view.do_update = TRUE;
	}
	if (r < 0) {
		gVrCopy(v -> view.top, v -> view.cur);
		v -> view.y = 0;
	} else if (r >= v -> view.rows) {
		vrAdvance(v -> view.top, (long) (r - (v -> view.rows - 1)));
		v -> view.y = v -> view.rows - 1;
	}
}	

/*********************************************************************\
**
** Line View Operations	and Class
**
\*********************************************************************/

static void vLnUp(v)
	View v;
{
	gVuMvLns(v, -1L);
	gVuRefresh(v);
}

static void vLnDn(v)
	View v;
{
	gVuMvLns(v, 1L);
	gVuRefresh(v);
}

ViewClassRec crLineView = {
	&crClass,				/* class */				/* Object Part */
   {												/* ObjectClass Part */
	sizeof (ViewRec),		/* instance size */
	"LineView",				/* class name */
	&crObject,				/* super */
	NIL,					/* method dict */
	vInit,	 				/* initialize */
	objDoesNotImplement,	/* new object */
	objClone,				/* clone self */
	objKill,				/* kill self */
	vOpen,					/* open */
	objDoesNotImplement,	/* close */
	vName,					/* name */
   }, {												/* ViewClassPart */
	vSet,					/* set tree */
	vRefresh,				/* refresh display */
	vUpdate,				/* update function */
	vUpdLine,				/* update line function */
	vLabel,					/* label */
	vMvLns,					/* move n lines */
	vLnUp,					/* goes up a line */
	vLnDn,					/* goes down a line */
	vPgUp,					/* goes up a page */
	vPgDn,					/* goes down a page */
	vFirst,					/* first directory */
	vLast,					/* last directory */
   },
};

global ViewClass clLineView = &crLineView;


/*********************************************************************\
**
** Page View Operations	and Class
**
\*********************************************************************/

void vPageLnUp(v)
	View v;
{
	gVuMvLns(v, (long) (- (v -> view.y + 1)));
}

void vPageLnDn(v)
	View v;
{
	gVuMvLns(v, (long) (v -> view.rows - v -> view.y));
}

ViewClassRec crPageView = {
	&crClass,				/* class */				/* Object Part */
   {												/* ObjectClass Part */
	sizeof (ViewRec),		/* instance size */
	"PageView",				/* class name */
	&crObject,				/* super */
	NIL,					/* method dict */
	vInit,					/* initialize */
	objDoesNotImplement,	/* new object */
	objClone,				/* clone self */
	objKill,				/* kill self */
	vOpen,					/* open */
	objDoesNotImplement,	/* close */
	vName,					/* name */
   }, {												/* ViewClassPart */
	vSet,					/* set tree */
	vRefresh,				/* refresh display */
	vUpdate,				/* update function */
	vUpdLine,				/* update line function */
	vLabel,					/* label */
	vMvLns,					/* move n lines */
	vPageLnUp,				/* goes up a line */
	vPageLnDn,				/* goes down a line */
	vPgUp,					/* goes up a page */
	vPgDn,					/* goes down a page */
	vFirst,					/* first directory */
	vLast,					/* last directory */
   },
};

global ViewClass clPageView = &crPageView;



/*********************************************************************\
**
** Initialization
**
\*********************************************************************/

global void viewInit()
{
}

