/*********************************************************************\
**
**	CURSE	--	cheap version of "curses"
**
**		This will eventually be able to use a variety of screen-access
**		methods,  although it will stay specialized to the PC.
**
**		Unlike the Unix curses package, curse maintains both characters
**		and attributes in its idea of the screen.  It is also able to
**		write directly to the screen buffer if its address is known.
**	
**	870807 SS	create PC version.
**
\*********************************************************************/

#include <dos.h>
#include <memory.h>
#include <stdio.h>
#include <stdarg.h>

#include "curse.h"
#include "ibmchars.h"

extern char* calloc();

#define global
#define local	static
#define DEBUG	if (debugf) {
#define DEBEND }

typedef unsigned char  uchar;
typedef unsigned short ushort;
typedef unsigned long  ulong;
typedef unsigned char  bool;

#define TRUE 	1
#define FALSE 	0

#define NULLWIN (WINDOW*)0L

typedef short far* ScreenData;
typedef ScreenData *ScreenBuf;

#define ISSCREEN(w)		((w)->flags & WIN_SCREEN)
#define ISSUBWIN(w)		((w)->flags & WIN_SUBWIN)
#define ISDIRECT(w)		((w)->flags & WIN_DIRECT)
#define LEAVECSR(w)		((w)->flags & WIN_LEAVE)
#define SCROLLOK(w)		((w)->flags & WIN_SCROLLOK)

#define CURCHAR(w)		((w)->y[(w)->cury][(w)->curx])
#define ANYCHAR(w,Y,X)	((w)->y[Y][X])

#ifdef NOTFAR	/* set this if data pointers are shorts */
#define memcpy(d,s,c)	movedata(FP_SEG(s),FP_OFF(ds),FP_SEG(d),FP_OFF(d),(c))
#endif /* NOTFAR */

/*********************************************************************\
**
**	V A R I A B L E S
**	
\*********************************************************************/

global WINDOW *stdscr;	/* -> standard screen */
static WINDOW *curscr;	/* -> current state of the screen, if different */
static WINDOW *curwin;	/* -> window currently being refreshed */


/* 
** Information about the terminal interface
**		normally initialized from TERMCAP
*/
static bool direct = TRUE;					/* assume direct output */
static ScreenData scrAddr = 
				(ScreenData)0xb0000000;		/* to the MDA 			*/
static short scrRows = 25;
static short scrCols = 80;

static bool cr_mode = TRUE;
static bool raw_mode= FALSE;
static bool echoing = FALSE;

/* BIOS call registers and parameters */

static union REGS chrregs, 				/* character output */
				  movregs, 				/* cursor move */
				  inregs, outregs;		/* general-purpose */

#define scrattr chrregs.h.bl
#define repcount chrregs.x.cx
#define scrx	movregs.h.dl
#define scry	movregs.h.dh

char oldy, oldx;						/* where the cursor was */

/*********************************************************************\
**
** Screen Buffer functions
**
**		The following functions (and macros) deal with screen buffers,
**		accessed through an array of pointers to rows.	If so specified,
**		this writes directly to the screen.
**
\*********************************************************************/

static ScreenBuf mkRowList(theData, ymax, xmax)
	ScreenData theData;
	short ymax, xmax;
	/*
	** Make a row list (w->y) given the address of the screen data
	**		In addition to its use in mkScrBuf, it is used for
	**		initialization when output can go directly to the screen.
	*/
{
	ScreenBuf  theRows;		/* the row pointers */
	register   short i;

	theRows = (ScreenBuf)calloc(ymax, sizeof(*theRows));
	for (i = 0; i < ymax; ++i) theRows[i] = &theData[xmax * i];
	return (theRows);
}

static ScreenBuf mkScrBuf(ymax, xmax)
	short ymax, xmax;
	/*
	** make a screen data buffer and row list 
	** for a window of the specified size
	*/
{
	ScreenData theData;		/* the data blocks */

	theData = (ScreenData)calloc(ymax * xmax, sizeof(*theData));
	return(mkRowList(theData, ymax, xmax));
}

static ScreenBuf mkSubBuf(sup, ymax, xmax, begy, begx)
	ScreenBuf sup;
	short ymax, xmax, begy, begx;
	/*
	** make a row list for a sub-window
	*/
{
	ScreenBuf  theRows;		/* the row pointers */
	register   short i;

	theRows = (ScreenBuf)calloc(ymax, sizeof(*theRows));
	for (i = 0; i < ymax; ++i) theRows[i] = sup[begy + i] + begx;
	return (theRows);
}


static void freeScrBuf(buf)
	ScreenBuf buf;
{
	free((char *)buf[0]);
	free((char *)buf);
}

static void freeSubBuf(buf)
	ScreenBuf buf;
{
	free((char *)buf);
}

/*********************************************************************\
**
** WINDOW functions
**
\*********************************************************************/

static WINDOW *mkWindow(sup, lines, cols, begy, begx)
	WINDOW *sup;
	short lines, cols, begy, begx;
	/*
	** Create a window and initialize most of its fields.
	** does NOT initialize the row buffer; that is left to the caller.
	*/
{
	WINDOW *w;

	w = (WINDOW *)calloc(1, sizeof(WINDOW));
	w -> maxy = lines;
	w -> maxx = cols;
	w -> begy = begy;
	w -> begx = begx;

	w -> curattr = (w -> normattr = WA_NORMAL) << 8;
	w -> standattr = WA_REVERSE;

	if (w -> sup = sup) {
		w -> sib = sup -> sub;
		sup -> sub = w;
	}

	return (w);
}

static void freeWindow(w)
	WINDOW *w;
{
	if (ISSUBWIN(w)) freeScrBuf(w->y);
	else			 freeSubBuf(w->y);
	free(w);
}


/*********************************************************************\
**
** Bios Interface Functions
**
**		Most of these deal with curwin, the current window.
**		Note that the screen's cursor position is fixed up by refresh().
**
\*********************************************************************/

/* === this stuff is completely bogus at present === */

static void mkcur(win)		/* make win current if it isn't already */
	WINDOW *win;
{
	if (curwin != win) {
		curwin 	= win;
		scrattr = win->curattr;
		scrx	= win->curx;
		scry	= win->cury;
	}
	/* need to get old cursor position into oldy, oldx */
}

static void addcur(ch)		/* add char. to current window */
	char ch;
{
	chrregs.h.al = ch;
	int86(0x10, &chrregs, &outregs);
	++scrx;
}

static void movCsr(y, x)	/* put the screen cursor where it belongs */
{
	scry = y;
	scrx = x;
	int86(0x10, &movregs, &outregs);
}

/*********************************************************************\
**
** Box-drawing
**
\*********************************************************************/

static char cornTbl[] = {
	S_VERT, S_HORIZ, S_UL, S_UR, S_LL, S_LR,
	D_VERT, D_HORIZ, D_UL, D_UR, D_LL, D_LR,
	HD_VERT, HD_HORIZ, HD_UL, HD_UR, HD_LL, HD_LR,
	VD_VERT, VD_HORIZ, VD_UL, VD_UR, VD_LL, VD_LR,

	0
};

static char *corners(vert, horiz)
	char vert, horiz;
{
	register char *c;

	for (c = cornTbl; *c && c[0] != vert && c[1] != horiz; c += 6) ;
	if (*c) return(c);
	else	return((char *)NULL);
}

/*********************************************************************\
**
** Initialization
**
\*********************************************************************/

global void initscr()				/* initialize */
{
	register WINDOW *w;

	/*
	** Try to figure out what kind of display we're using:
	**		Video mode 7 = MDA/HDA
	**		2 or 3 means it's a CGA
	**		It's not clear how we decide to use the BIOS,
	**		but that's not working now anyway.
	*/
	inregs.h.ah = 15;				/* what's the mode? */
	int86(0x10, &inregs, &outregs);
	switch (outregs.h.al) {
	 case 2:
	 case 3:								/* CGA */
	 	scrAddr = (ScreenData)0xb8000000L;
		/* === We're not going to worry about page yet === */
		break;
	 case 7:								/* MDA/HDA */
	 	scrAddr = (ScreenData)0xb0000000L;
		break;
	 default:
	 	direct = FALSE;
		fprintf(stderr, 
				"I don't know how to handle non-standard displays yet\n");
		exit(1);
	}

	/*
	** Set up the BIOS call registers
	*/
	chrregs.h.ah = 0x09;	/* registers for character output */
	scrattr = WA_NORMAL;	/*		attributes */
	chrregs.x.cx = 1;		/* 		character count */
	scrx = scry = 0;		/*		cursor position */
	movregs.h.ah = 0x02;	/* registers for cursor movement */
	chrregs.h.bh = movregs.h.bh = inregs.h.bh = 0;	/* screen number */
	/*
	** Set up standard screen
	*/
	w = stdscr = mkWindow(NULLWIN, scrRows, scrCols, 0, 0);
	if (direct) {
		w -> y = mkRowList(scrAddr, scrRows, scrCols);
		w -> x = &CURCHAR(w);
		w -> flags |= WIN_DIRECT;
		curscr = stdscr;
	} else {
		w -> y = mkScrBuf(scrRows, scrCols);
		w -> x = &CURCHAR(w);
		/*
		** If not direct, set up current screen, too.
		*/
		w = curscr = mkWindow(NULLWIN, scrRows, scrCols, 0, 0);
		w -> y = mkScrBuf(scrRows, scrCols);
		w -> x = &CURCHAR(w);
	}
	movCsr(stdscr -> maxy, 0);
}

global void endwin()		/* cleanup */
{
	/* clear screen and move cursor to bottom */
	clear();
	move(stdscr->maxy - 1, 0);
	stdscr->flags |= WIN_LEAVE;
	refresh();
}


/*********************************************************************\
**
** Window creation and destruction
**
\*********************************************************************/

global WINDOW *newwin(lines, cols, begy, begx)
	short lines, cols, begy, begx;
{
	WINDOW *w;

	w = mkWindow(NULLWIN, lines, cols, begy, begx);
	w -> y = mkScrBuf(lines, cols);
	w -> x = &CURCHAR(w);

	return(w);
}

global WINDOW *subwin(win, lines, cols, begy, begx)
	WINDOW *win;
	short lines, cols, begy, begx;
{
	WINDOW *w;

	w = mkWindow(win, lines, cols, begy, begx);
	w -> y = mkSubBuf(win -> y, lines, cols, begy, begx);
	w -> x = &CURCHAR(w);
	w -> flags |= WIN_SUBWIN | (win -> flags & WIN_DIRECT);

	return(w);
}

global void delwin(win)
	WINDOW *win;
{
	
}


/*********************************************************************\
**
** Operations on data in windows
**
\*********************************************************************/

void waddch(win, ch)				/* (ch) add a character to the screen */
	WINDOW *win;
	char ch;
{
	switch (ch) {
	 case '\r':
	 	win -> curx = 0;
		win -> x = &CURCHAR(win);
		break;
	 case '\n':
	 	win -> curx = 0;
		win -> cury += 1;
		win -> x = &CURCHAR(win);
		/* === don't worry about scrolling yet === */
		break;
	 case '\t':
	 	do {
			if (win -> curx >= win -> maxx) return;
			*(win->x++) = win->curattr | ' ';
		} while (++win->curx % 8); 
		break;
	 default:
	 	if (win -> curx >= win -> maxx) return;
		*(win->x++) = win->curattr | ((unsigned)ch & 255);
		++win -> curx;
		/* === don't worry about scrolling yet === */
	}
}

void waddstr(win, str)			/* (str) add a string to screen */
	WINDOW *win;
	char *str;
{
	for ( ; *str; ++str) waddch(win, *str);
}

void wprintf(win, fmt)
	WINDOW *win;
	char *fmt;
{
	va_list arg_ptr;
	char buf[512];
	
	va_start(arg_ptr, fmt);
	vsprintf(buf, fmt, arg_ptr);
	waddstr(win, buf);
}

void wattr(win, newattr) 		/* (attr) set attribute byte -- not curses */
	WINDOW *win;
	int newattr;
{
	win -> curattr = newattr << 8;
}

void wclear(win)				/* clear the screen */
	WINDOW *win;
{
	win -> curx = win -> cury = 0;
	win -> x = &CURCHAR(win);
	wclrtobot(win);
}

void wclrtobot(win)				/* clear to bottom of screen */
	WINDOW *win;
{
	register short r, c;
	register short sp;
	register ScreenData rr;

	sp = win->curattr | ' ';
	for (r = win->cury; r < win->maxy; ++r)
		for (rr = win->y[r], c = 0; c < win->maxx; ++c, ++rr) *rr = sp;
}

void wclrtoeol(win)				/* clear to end of line */
	WINDOW *win;
{
	register short c;
	register short sp;
	register ScreenData rr;

	sp = win->curattr | ' ';
	for (c = win -> curx, rr = win -> x ; c < win->maxx; ++c, ++rr) *rr = sp;
}

void getyx(win, y, x)			/* (&y, &x) get coords -- curses has win */
	WINDOW *win;
	int *y, *x;
{
	*y = win->cury;
	*x = win->curx;
}

void wmove(win, y, x)			/* (y, x) move to coords */
	WINDOW *win;
	int y, x;
{
	win -> cury = y;
	win -> curx = x;
	win -> x = &CURCHAR(win);
}

void wrefresh(win)
	WINDOW *win;
{
	register WINDOW *w;
	short offy = 0, offx = 0;
	short r, c;
	ScreenData qq, rr;

	/*
	** Search up win's parent list.  compute offset in stdscr.
	**		if not a descendent of stdscr, copy contents to stdscr.
	**		if stdscr != curscr, copy to screen
	*/
	for (w = win; ISSUBWIN(w) && w -> sup; w = w -> sup) {
		offy += w -> begy;
		offx += w -> begx;
	}
	if (w != stdscr) {
		offy += w -> begy;
		offx += w -> begx;
		for (r = 0; r < win->maxy; ++r) {
			rr = win->y[r]; qq = stdscr->y[r + offy] + offx;
			memcpy(qq, rr, win -> maxx * sizeof(short));
		}
	}
	if (stdscr != curscr) {
		
	}
	/*
	** Restore screen cursor if necessary
	*/
	if (LEAVECSR(win)) 
		movCsr(win->cury + win->begy, win->curx + win->begx);
	else
		movCsr(stdscr -> maxy, 0);
}

void wstandout(win)				/* start standout mode */
	WINDOW *win;
{
	win->curattr = win->standattr << 8;
}

void wstandend(win)				/* end standout mode */
	WINDOW *win;
{
	win->curattr = win->normattr << 8;
}

void winsch(win, ch)			/* insert char (push right) */
	WINDOW *win;
	char ch;
{
	register short c;
	register short t, sp = (ch & 255) | win -> curattr;
	register ScreenData rr;

	for (c = win -> curx, rr = win -> x; c < win -> maxx; ++c, ++rr) {
		t = *rr;
		*rr = sp;
		sp = t;
	}
}

void wdelch(win)				/* delete char (pull left) */
	WINDOW *win;
{
	register short c;
	register ScreenData rr;

	for (c = win -> curx, rr = win -> x; c < win -> maxx - 1; ++c, ++rr) {
		*rr = rr[1];
	}
	*rr = ' ' | win -> curattr;
}

void winsertln(win)				/* insert blank line (push down) */
	WINDOW *win;
{
	register short r, c;
	register ScreenData rr;
	register short sp;

	for (r = win->maxy - 1; r > win -> cury; --r)
		memcpy(win -> y[r], win -> y[r - 1], win -> maxx * 2);

	sp = win->curattr | ' ';	/* clear current line */
	for (rr = win->y[win->cury], c = 0; c < win->maxx; ++c, ++rr) *rr = sp;
}

void wdeleteln(win)				/* delete line (pull up) */
	WINDOW *win;
{
	register short r, c;
	register ScreenData rr;
	register short sp;

	for (r = win->cury; r < win -> maxy - 1; ++r)
		memcpy(win -> y[r], win -> y[r + 1], win -> maxx * 2);

	sp = win->curattr | ' ';	/* clear bottom line */
	for (rr = win->y[r], c = 0; c < win->maxx; ++c, ++rr) *rr = sp;
}


void box(win, vert, horiz)		/* draw a box around the window */
	WINDOW *win;
	char vert, horiz;
{
 	register int i;
	register char *c;

	wmove(win, 0, 0);
	for (i = 0; i < win -> maxx; ++i) waddch(win, horiz);
	wmove(win, win->maxy - 1, 0);
	for (i = 0; i < win -> maxx; ++i) waddch(win, horiz);
	for (i = 1; i < win -> maxy - 1; ++i) {
		wmove(win, i, 0);
		waddch(win, vert);
		wmove(win, i, win -> maxx - 1);
		waddch(win, vert);
	}
	if (c = corners(vert, horiz)) {
		wmove(win, 0, 0);
		waddch(win, c[2]);
		wmove(win, 0, win -> maxx - 1);
		waddch(win, c[3]);
		wmove(win, win -> maxy - 1, 0);
		waddch(win, c[4]);
		wmove(win, win -> maxy - 1, win -> maxx - 1);
		waddch(win, c[5]);
	}
	wmove(win, 1, 1);
}

char winch(win, y, x)
	WINDOW *win;
	short y, x;
{
	return ((char)ANYCHAR(win, y, x));
}


short wincha(win, y, x)
	WINDOW *win;
	short y, x;
{
	return (ANYCHAR(win, y, x));
}

void waddcha(win, ch)				/* (ch) add a character with attrs */
	WINDOW *win;
	short ch;
{
	*(win->x++) = ch;
	++win -> curx;
}

/*********************************************************************\
**
** Character input, echoing, and editing
**
\*********************************************************************/


int wgetch(win)				/* (win)	get char with echo in window */
	WINDOW *win;
{
	register int c;
	
	c = getch();
	if (echoing) {
		if (c == '\t') {
			waddch(win, c);
		} else if (c < ' ' || c >= 0x7F) {
			;
		} else {
			waddch(win, c);
		}
	}
	return(c);
}

void wgetstr(win, s)		/* (win, str) get string from window */
	WINDOW *win;
	char   *s;
{
	register int c;
	register char *p = s;		/* cursor loc. */
	register char *q = s;		/* highwater mark */
	int flags;

	if (echoing) {
		flags = win -> flags;
		win -> flags |= WIN_LEAVE;
		wrefresh(win);
	}
	for (;;) {
		c = getch();
		if (echoing) {
			if (c == '\t') {
				waddch(win, c);
			} else if (!raw_mode && (c == '\b' || c == 0x7F)) {
				if (p <= s) continue;	/* at or before beginning */
				if (p < q) {
					/* === */
				} else {
					--p;
					--q;
				}
				/* === doesn't handle tabs or ctrl chars === */
				wmove(win, win -> cury, win -> curx - 1);
				wdelch(win);
				wrefresh(win);
				continue;
			} else if (c < ' ' || c >= 0x7F) {
				/* === ought to echo as ^x === */;
			} else {
				waddch(win, c);
			}
			wrefresh(win);
		}
		if (c == '\r' || c == '\n') {
			break;
		} 
		if (p < q) {	/* insert */
			/* === */
		} else {		/* append */
			*p++ = c;
			q = p;
		}
		if (!cr_mode && (c <= ' ' || c >= 0x7F)) {
			break;
		} 
	}
	*q = 0;
	if (echoing) win -> flags = flags;
	return;
}

/* terminal mode functions */

void crmode() 	{cr_mode = TRUE;}		/* leave cbreak mode */
void nocrmode() {cr_mode = FALSE;}		/* enter cbreak mode */
void echo()		{echoing = TRUE;}		/* echo characters to screen */
void noecho()	{echoing = FALSE;}		/* don't echo characters to screen */
void raw()		{raw_mode= TRUE;}		/* enter raw mode (no editing) */
void noraw()	{raw_mode= FALSE;}		/* leave raw mode */



