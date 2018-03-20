/***/ static char *pgmid = "screen 1.0 copyright 1989 S. Savitzky"; /***/

/*********************************************************************\
**
**	Screen.c -- screen management structures and operations
**
**	890716 SS	create from parts of main.c
**
\*********************************************************************/

#include <stdlib.h>
#include <malloc.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <signal.h>

#include "curse.h"
#include "ibmkeys.h"
#include "menu.h"

#include "coops.h"
#include "tree.h"
#include "dir.h"
#include "viewer.h"
#include "view.h"
#include "disk.h"

#undef  global
#define global
#include "screen.h"
					 		/* === some of this should be dragged in === */
extern Dir curFile();
extern void setRoot();
extern int cExit();

extern View oMsgView;

/*********************************************************************\
**
** Screen saving and restoring for error view.
**
\*********************************************************************/

static short *errscr;

void savescr()
{
	int x, y, scrx, scry;

	scrx = stdscr -> maxx;
	scry = stdscr -> maxy;
	if (!errscr)
		errscr = (short *) malloc(scrx * scry * sizeof(short));
	for (y = 0; y < scry; ++y)
		for (x = 0; x < scrx; ++x)
			errscr[y * scrx	+ x] = wincha(stdscr, y, x);
}

void restscr()
{
	int x, y, scrx, scry;

	scrx = stdscr -> maxx;
	scry = stdscr -> maxy;
	for (y = 0; y < scry; ++y) {
		wmove(stdscr, y, 0);
		for (x = 0; x < scrx; ++x) {
			waddcha(stdscr, errscr[y * scrx	+ x]);
		}
	}
}

/*********************************************************************\
**
** errorView		display error message string
** errorPrintf		format and display error msg
**
** errorClear		clear error message
** errorSet			set error message to display later
** errorCheck		check for & display message; return 1 if present
**
**		errorView clears any message set by errorSet.  errorSet etc.
**		are used mainly for dealing with hardware errors not detected
**		by the C library routines.
**
\*********************************************************************/

static char *errorText[] =  {
	"", "", "", "", "", "", "", "", "", 
	(char *)0, 
	"",
	"[Hit Q to quit, any other key to continue.]",
	(char *)0L	
};

int errorView(msg)
	char *msg;
{
	char c;

	savescr();
	errorText[9] = msg;
	setRoot(oMsgView, errorText);
	gOpen(oMsgView);
	c = getch();			 /* === ought to use menu === */
	if (c == 'q' || c == 'Q') cExit (1);
	restscr();
/*
	if (theScreen -> partner) gOpen(theScreen -> partner -> view);
	gOpen(theView);
*/
	errorMsg = 0L;
	return(1);
}

int errorPrintf(fmt)
	char *fmt;
{
	va_list arg_ptr;
	static char buf[512];
	
	va_start(arg_ptr, fmt);
	vsprintf(buf, fmt, arg_ptr);

	return (errorView(buf));
}

void errorClear()
{
	errorMsg = 0L;
}

void errorSet(msg)
	String msg;
{
	errorMsg = msg;
}

int errorCheck()
{
	if (errorMsg) {
		errorPrintf(errorMsg);
		errorClear();
		return(TRUE);
	} else {
		return(FALSE);
	}
}

/*********************************************************************\
**
** Stuff for updating the screen.
**
**		Updates are done while waiting for keystrokes.
**
\*********************************************************************/

Bool scrNeedsUpdate    = FALSE;
int  scrSkippedUpdates = 0;
#define MAXSKIP 5

void scrDoUpdate()
{	
	View v;

	scrNeedsUpdate    = FALSE;
	scrSkippedUpdates = 0;
	if (theScreen -> stat) (*theScreen -> stat)(theScreen);
	if (theScreen -> partner) {
		v = theScreen -> partner -> view;
		v -> view.is_active  = FALSE;
		v -> view.do_refresh = TRUE;
		gVuLabel(v, gName(v), "=\315");
		if (! v -> view.do_update) gVuUpdLine(v);
		gVuRefresh(v);
	}
}

/*********************************************************************\
**
** getCmd()			get command character
**
\*********************************************************************/

int getCmd()
{
	if (scrNeedsUpdate)	{
		if (!kbhit() || scrSkippedUpdates >= MAXSKIP) {
			scrDoUpdate();
		} else {
			++ scrSkippedUpdates;
		}
	}
	return(getch());
}



/*********************************************************************\
**
** Status view setup for read and write
**
**		This is a bit tricky.  A pointer to a callback routine is
**		placed in dEachNewDir.  This routine is called each time
**		a new Dir node is created.  It is initialized to dReadStart,
**		which is called with the root.
**
**		dReadStart immediately resets the callback to dStatusEachDir,
**		which is then called for each Dir under that root.
**
\*********************************************************************/

static char *readingText[] = {
	"", "", "", "", "", "", "", "", "", "READING", (char *)0L	
};

static char *writingText[] = {
	"", "", "", "", "", "", "", "", "", "WRITING",
	"Type <esc> to abort.", (char *)0L	
};


static void dStatusEachDir(d)
	Dir d;
{
	move(24, 0);
	addstr(gPath(d, NIL));
	clrtoeol();
	treeStat(theScreen);
}

global void dReadMessage()
{
	setRoot(oMsgView, readingText);
	gOpen(oMsgView);
}

global void dReadStart(n)
	Dir  n;
{
	setRoot(dViewToUpdate, n);
	dEachNewDir = dStatusEachDir;
}

global void dWriteStart(n)
	Dir  n;
{
	View v = theView;

	if (v) {
		setRoot(oMsgView, writingText);
		gOpen(oMsgView);
	}
	dEachNewDir = dStatusEachDir;
}

/*********************************************************************\
**
** Screens and Screen Operations
**
**		A screen consists of a view and a menu.
**		It has pointers to, and labels for, its left and right neighbors,
**		and may share the screen with one of them.
**
**		Note that the view pointer is double-indirect, so that we
**		can reference the existing view-pointer variables.
**		This is a kludge.
**
**		A screen really ought to be an object.
**		The idea of details needs to be re-thought some.
**
\*********************************************************************/

void scrOnUpdate()
{
	scrNeedsUpdate = TRUE;
}

void scrSet(s, d)
	Screen s;
	Tree d;
{
	if (!s) return;
	setRoot(s -> view, d);
	gVuRefresh(s -> view);
}

void scrUpdLine(s)
	Screen s;
{
	gVuUpdLine(theView);
	gVuRefresh(s -> view);
	scrOnUpdate();
}

int scrOpen(s)
	Screen s;
{
	static char lbl[32];

	theScreen = s;
	theView = s -> view;
	theView -> view.is_active = TRUE;
	theView -> view.do_update = TRUE;
	/* we don't use gOpen, so as not to mash the label line */
	gVuLabel(theView, gName(theView), "=\315");
	if (s -> left && s -> left -> label) {
		sprintf(lbl, "\256%s\256", s -> left -> label);
		wstandout(theView -> view.box);
		gVuLabel(theView, lbl, "+");
		wstandend(theView -> view.box);
	}
	if (s -> right && s -> right -> label) {
		sprintf(lbl, "\257%s\257", s -> right -> label);
		wstandout(theView -> view.box);
		gVuLabel(theView, lbl, "-");
		wstandend(theView -> view.box);
	}
	gVuRefresh(theView);
	scrDoUpdate();
	botLine();
	if (theScreen -> menu)
		return(MenuJump(theScreen -> menu));
	else
		return(1);
}

/*********************************************************************\
**
** Generic Movement Commands
**
**		These operate on theView
**
\*********************************************************************/

int cNull(s)	char *s; {return(1);}
int cError(s)	char *s; {return(0);}

int cLnUp(s)
	char *s;
{
	gVuLnUp(theView);
	gVuRefresh(theView);
	scrOnUpdate();
	return(1);
}

int cLnDn(s)
	char *s;
{
	gVuLnDn(theView);
	gVuRefresh(theView);
	scrOnUpdate();
	return(1);
}

int cPgUp(s)
	char *s;
{
	gVuPgUp(theView);
	gVuRefresh(theView);
	scrOnUpdate();
	return(1);
}

int cPgDn(s)
	char *s;
{
	gVuPgDn(theView);
	gVuRefresh(theView);
	scrOnUpdate();
	return(1);
}

int cLeft(s)
	char *s;
{
	if (!theScreen -> left) return(0);
	return (scrOpen(theScreen -> left));
}

int cRight(s)
	char *s;
{
	if (!theScreen -> right) return(0);
	return (scrOpen(theScreen -> right));
}

int cRightToFiles(s)
	char *s;
{
	if (ISNULL(curFile(theView))) return(0);
	return(cRight(s));
}

int cHome(s)
	char *s;
{
	gVuFirst(theView);
	gVuRefresh(theView);
	scrOnUpdate();
	return(1);
}

int cEnd(s)
	char *s;
{
	gVuLast(theView);
	gVuRefresh(theView);
	scrOnUpdate();
	return(1);
}


/*********************************************************************\
**
** Generic Tagging Commands
**
\*********************************************************************/

static int tag1(f, m)
	Dir f;
	ushort m;
{
	if (f -> dir.isDir) {
		for (f = (Dir) gDown(f); !ISNULL(f); f = (Dir) gNext(f)) 
			tag1(f, m);
		return(1);
	}
	dTag(f);
	if (m != unknown) f -> dir.ftype = m;
	return(1);
}

int tag(m)
	ushort m;
{
	View v = theView;
	Dir f = curFile(v);

	if (!tag1(f, m)) return(0);
	/* need to update partner ALWAYS */
	theView -> view.do_update = TRUE;
	if (theScreen -> partner) gVuUpdate(theScreen -> partner -> view);
	return (cLnDn());
}


int cAscii(s)
	char *s;
{
	return(tag(ascii));
}

int cCAscii(s)
	char *s;
{
	MenuPop();
	return(tag(ascii));
}

int cBin(s)
	char *s;
{
	return(tag(binary));
}

int cCBin(s)
	char *s;
{
	MenuPop();
	return(tag(binary));
}

int cTag(s)
	char *s;
{
	if (s)	return(tag((ushort)s));
	else	return(tag(unknown));
}


int cUntag(s)
	char *s;
{
	if (ISNULL(curFile(theView))) return(0);
	dUntag(curFile(theView));
	/* need to update partner ALWAYS */
	theView -> view.do_update = TRUE;
	if (theScreen -> partner) gVuUpdate(theScreen -> partner -> view);
	return (cLnDn());
}




