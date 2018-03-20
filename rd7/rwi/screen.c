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

#include "../lib/curse.h"
#include "../lib/ibmkeys.h"
#include "../lib/menu.h"


#include "coops.h"
#include "trees.h"
#include "dirs.h"
#include "viewer.h"
#include "view.h"
#include "disk.h"
#include "help.h"
#include "convert.h"

#undef  global
#define global
#include "screen.h"
#include "data.h"

extern Dir curFile();
extern void setRoot();

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
	"[Hit any key to continue.]",
	(char *)0L	
};

int errorView(msg)
	char *msg;
{
	errorText[9] = msg;
	setRoot(oMsgView, errorText);
	gOpen(oMsgView);
	getCmd();			 /* === ought to use menu === */
	if (theView -> view.detail) gOpen(theView -> view.detail);
	gOpen(theView);
	errorMsg = 0L;
	return(1);
}

int errorPrintf(fmt)
	char *fmt;
{
	va_list arg_ptr;
	char buf[512];
	
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
** getCmd()			get command character
** savescr()		save the screen during graphics
** restscr()		restore the screen after graphics
**
\*********************************************************************/

int getCmd()
{
	return(getch());
}

static int x, y, scrx, scry;		/* screen-saver variables */
static short *oldscr;

void savescr()
{
	scrx = stdscr -> maxx;
	scry = stdscr -> maxy;
	oldscr = (short *) malloc(scrx * scry * sizeof(short));
	for (y = 0; y < stdscr -> maxy; ++y)
		for (x = 0; x < stdscr -> maxx; ++x)
			oldscr[y * scrx	+ x] = wincha(stdscr, y, x);
}

void restscr()
{
	for (y = 0; y < stdscr -> maxy; ++y) {
		wmove(stdscr, y, 0);
		for (x = 0; x < stdscr -> maxx; ++x) {
			waddcha(stdscr, oldscr[y * scrx	+ x]);
		}
	}
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
	View v;

	if (theScreen -> stat) (*theScreen -> stat)(theScreen);
	if (theScreen -> partner) {
		v = theScreen -> partner -> view;
		v -> view.is_active  = FALSE;
		v -> view.do_refresh = TRUE;
		gVuLabel(v, gName(v), "=\315");
		gVuUpdLine(v);
		gVuRefresh(v);
	}
}

void scrSet(s, d)
	Screen s;
	Tree d;
{
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
	scrOnUpdate();
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
	FileMode m;
{
	if (f -> dir.isDir) {
		for (f = (Dir) gDown(f); !ISNULL(f); f = (Dir) gNext(f)) 
			tag1(f, m);
		return(1);
	}
	if (m == jcamp || m == css || m == spectrum) {
		/* 
		** Check for name ending in ".sp"
		*/
		register char *p = strrchr(f -> dir.name, '.');
		if (! (p && strcmp(p, ".sp") == 0) &&
		    ! (p && strcmp(p, ".SP") == 0)    ) return(0);
	}
	dTag(f);
	if (m != unknown) f -> dir.mode = m;
	return(1);
}

int tag(m)
	FileMode m;
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
	return(tag(unknown));
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




