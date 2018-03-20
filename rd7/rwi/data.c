/*********************************************************************\
**
** data.c	--	data structure (object) declarations
**
**	890813	SS	drag pieces in from all over
**
\*********************************************************************/

#include <stdio.h>
#include "../lib/curse.h"
#include "../lib/ibmchars.h"
#include "../lib/ibmkeys.h"

#include "coops.h"
#include "trees.h"
#include "dirs.h"
#include "viewer.h"
#include "view.h"
#include "convert.h"
#include "help.h"
#include "disk.h"
#include "irmx.h"

#define MenuMACROS 1
#include "../lib/menu.h"
#include "screen.h"

#undef  global
#define global
#include "data.h"

#define V_CVT 1

/*********************************************************************\
**
** Contents:
**		Layout Parameters
**		Status-display functions
**		Views
**		Key Handlers
**		Menus
**		Screens
**		Initialization functions
**
\*********************************************************************/

/*********************************************************************\
**
** Parameters
**
\*********************************************************************/

#define VIEWROWS 19
#define VIEWCOLS 78
#define DIRCOLS  37
#define FILECOLS (VIEWCOLS - DIRCOLS - 1)
#define DATAROW	  4
#define DIRCOL	  1
#define FILECOL	  (DIRCOLS + 2)

/* Page Layout */

int swidth    = 80;			/* width of the screen 				*/
int sheight   = 25;			/* height of the screen 			*/

int statusRow = 0;			/* row for status display			*/
int menuRow   = 1;			/* row for menu display				*/
int headerRow = 3;			/* row for headers					*/
int dataRow	  = 4;			/* first data row					*/
int bottomRow = VIEWROWS+5;	/* bottom row.						*/

/*********************************************************************\
**
** Status Routines
**
\*********************************************************************/

char pathbuf[256];

/*
** Put standard stuff on bottom line.
*/
void botLine()
{
	move(stdscr -> maxy - 1, 0);
	clrtoeol();
	wprintf(stdscr, "  From: %c:%s  To: %c:%s  ", 
			dInputDrive - 1 + 'A', dInputFS, 
			dOutputDrive - 1 + 'A', dOutputFS
		   );
	move(24, 40);

	addstr("Write to: ");
	addstr(dOutputDrive ? dOutputPath : "<output directory invalid>");
}

/*
** Standard stuff for top line in directory views
*/
void treeStat(scr)
	Screen scr;
{
	View v = scr -> view;
	Dir d = ((Dir) gVrGet(v -> view.root));
	register int i;
	static char s[80];

	sprintf(s, "\315 %s ", v -> view.name);
	if (ISNULL(d)) {
		sprintf(s + strlen(s), "[empty] ");
	} else {
		sprintf(s + strlen(s), "%s %4d / %d(%ldKb) ", 
			    gName(d), d -> dir.dcount, d -> dir.fcount,
  		        (d -> dir.fsize + 1023) / 1024);
		if (d -> dir.tcount) {
			sprintf(s + strlen(s), " *%d(%ldKb)", d -> dir.tcount,
  		            (d -> dir.tsize + 1023) / 1024);
		}
	}
	move(statusRow,1);
	addstr(s);
	for (i = strlen(s) + 2; i < swidth; ++i) addch(D_HORIZ);
}

/*
** This one's for directory views.  Make sure the partner is
** up to date.
*/
void dirStat(scr)
	Screen scr;
{
	TreeVr vl, vr;

	vl = (TreeVr) (scr -> view -> view.cur);
	vr = (TreeVr) (scr -> partner -> view -> view.root);

	if (vl -> treeVr.cur != vr -> treeVr.root) {
		setRoot(scr -> partner -> view, vl -> treeVr.cur);
	}
	treeStat(scr);
}

/*********************************************************************\
**
** Views and Viewers
** 
**		most of the views have fixed viewers.
**
\*********************************************************************/

/* Imported classes */

extern ViewClassRec crLineView;
extern ViewClassRec crPageView;

extern VrClassRec crTreeVr;
extern VrClassRec crLeafVr;
extern VrClassRec crTextVr;
extern VrClassRec crFileVr;
extern VrClassRec crHelpVr;
extern VrClassRec crDriveVr;
extern VrClassRec crVolVr;

static TreeVrRec sfv[4] = {
	{ (Class) &crLeafVr, {0, 0, 0, 1} },
	{ (Class) &crLeafVr, {0, 0, 0, 1} },
	{ (Class) &crLeafVr, {0, 0, 0, 1} },
	{ (Class) &crLeafVr, {0, 0, 0, 1} },
};

ViewRec rSrcFileView = {
	(Class)	&crLineView,
   {	
	(WINDOW *) 0L,		/* the window the view is in */
	(WINDOW *) 0L,		/* the window for the box around it */
	"Files",			/* the name */
	0, 0, 0, 0,	0,		/* flag word */
	FILECOL,			/* window X position */
	DATAROW,			/* window Y position */
	FILECOLS,			/* window width */
	VIEWROWS,			/* window height */
	0,					/* data rows */
	(View)NIL,			/* associated view */
	0,					/* cursor row */
	0,					/* cursor column */
	(Viewer)&sfv[0], (Viewer)&sfv[1], (Viewer)&sfv[2], (Viewer)&sfv[3],
   },
};

static TreeVrRec dfv[4] = {
	{ (Class) &crLeafVr, {0, 0, 0, 1} },
	{ (Class) &crLeafVr, {0, 0, 0, 1} },
	{ (Class) &crLeafVr, {0, 0, 0, 1} },
	{ (Class) &crLeafVr, {0, 0, 0, 1} },
};
ViewRec rDstFileView = {
	(Class)	&crLineView,
   {	
	(WINDOW *) 0L,		/* the window the view is in */
	(WINDOW *) 0L,		/* the window for the box around it */
	"Files",			/* the name */
	0, 0, 0, 0,	0,		/* flag word */
	FILECOL,			/* window X position */
	DATAROW,			/* window Y position */
	FILECOLS,			/* window width */
	VIEWROWS,			/* window height */
	0,					/* data rows */
	(View)NIL,			/* associated view */
	0,					/* cursor row */
	0,					/* cursor column */
	(Viewer)&dfv[0], (Viewer)&dfv[1], (Viewer)&dfv[2], (Viewer)&dfv[3],
   },
};

static TreeVrRec sdv[4] = {
	{ (Class) &crTreeVr, {0, 0, 1, 0} },
	{ (Class) &crTreeVr, {0, 0, 1, 0} },
	{ (Class) &crTreeVr, {0, 0, 1, 0} },
	{ (Class) &crTreeVr, {0, 0, 1, 0} },
};
ViewRec rSrcDirView = {
	(Class)&crLineView,
   {	
	(WINDOW *) 0L,		/* the window the view is in */
	(WINDOW *) 0L,		/* the window for the box around it */
	"From",				/* name */
	0, 0, 0, 0,	0,		/* flag word */
	DIRCOL,				/* window X position */
	DATAROW,			/* window Y position */
	DIRCOLS,			/* window width */
	VIEWROWS,			/* window height */
	0,					/* data rows */
	(View)&rSrcFileView,			/* associated view */
	0,					/* cursor row */
	0,					/* cursor column */
	(Viewer)&sdv[0], (Viewer)&sdv[1], (Viewer)&sdv[2], (Viewer)&sdv[3],
   },
};

static TreeVrRec ddv[4] = {
	{ (Class) &crTreeVr, {0, 0, 1, 0} },
	{ (Class) &crTreeVr, {0, 0, 1, 0} },
	{ (Class) &crTreeVr, {0, 0, 1, 0} },
	{ (Class) &crTreeVr, {0, 0, 1, 0} },
};
ViewRec rDstDirView = {
	(Class)&crLineView,
   {	
	(WINDOW *) 0L,		/* the window the view is in */
	(WINDOW *) 0L,		/* the window for the box around it */
	"To",				/* the name */
	0, 0, 0, 0,	0,		/* flag word */
	DIRCOL,				/* window X position */
	DATAROW,			/* window Y position */
	DIRCOLS,			/* window width */
	VIEWROWS,			/* window height */
	0,					/* data rows */
	(View)&rDstFileView,		/* associated view */
	0,					/* cursor row */
	0,					/* cursor column */
	(Viewer)&ddv[0], (Viewer)&ddv[1], (Viewer)&ddv[2], (Viewer)&ddv[3],
   },
};

#ifndef FILE_VIEWER
static TextVrRec tv[4] = {
	{ (Class) &crTextVr, {0L, } },
	{ (Class) &crTextVr, {0L, } },
	{ (Class) &crTextVr, {0L, } },
	{ (Class) &crTextVr, {0L, } },
};
#else
static FileVrRec tv[4] = {
	{ (Class) &crFileVr, {0L, } },
	{ (Class) &crFileVr, {0L, } },
	{ (Class) &crFileVr, {0L, } },
	{ (Class) &crFileVr, {0L, } },
};
#endif
ViewRec rTextView = {
	(Class)	&crPageView,
   {	
	(WINDOW *) 0L,		/* the window the view is in */
	(WINDOW *) 0L,		/* the window for the box around it */
	pathbuf,			/* the name (replaced by filename) */
	0, 0, 0, 0,	0,		/* flag word */
	DIRCOL,				/* window X position */
	DATAROW,			/* window Y position */
	VIEWCOLS,			/* window width */
	VIEWROWS,			/* window height */
	0,					/* data rows */
	(View) NIL,			/* associated view */
	0,					/* cursor row */
	0,					/* cursor column */
	(Viewer)&tv[0], (Viewer)&tv[1], (Viewer)&tv[2], (Viewer)&tv[3],
   },
};

static HelpVrRec hv[4] = {
	{ (Class) &crHelpVr, {0, } },
	{ (Class) &crHelpVr, {0, } },
	{ (Class) &crHelpVr, {0, } },
	{ (Class) &crHelpVr, {0, } },
};
ViewRec rHelpView = {
	(Class)	&crPageView,
   {	
	(WINDOW *) 0L,		/* the window the view is in */
	(WINDOW *) 0L,		/* the window for the box around it */
	"Help",				/* the name */
	0, 0, 0, 0,	0,		/* flag word */
	DIRCOL,				/* window X position */
	DATAROW,			/* window Y position */
	VIEWCOLS,			/* window width */
	VIEWROWS,			/* window height */
	0,					/* data rows */
	(View) NIL,			/* associated view */
	0,					/* cursor row */
	0,					/* cursor column */
	(Viewer)&hv[0], (Viewer)&hv[1], (Viewer)&hv[2], (Viewer)&hv[3],
   },
};

static HelpVrRec iv[4] = {
	{ (Class) &crHelpVr, {0, } },
	{ (Class) &crHelpVr, {0, } },
	{ (Class) &crHelpVr, {0, } },
	{ (Class) &crHelpVr, {0, } },
};
ViewRec rInitView = {
	(Class)	&crPageView,
   {	
	(WINDOW *) 0L,		/* the window the view is in */
	(WINDOW *) 0L,		/* the window for the box around it */
	"",					/* the name */
	TRUE, 0, 0, 0, 0,	/* flag word -- is_centered = TRUE */
	DIRCOL,				/* window X position */
	DATAROW,			/* window Y position */
	VIEWCOLS,			/* window width */
	VIEWROWS,			/* window height */
	0,					/* data rows */
	(View) NIL,			/* associated view */
	0,					/* cursor row */
	0,					/* cursor column */
	(Viewer)&iv[0], (Viewer)&iv[1], (Viewer)&iv[2], (Viewer)&iv[3],
   },
};

#if 0
static TextVrRec mv[4] = {
	{ (Class) &crTextVr, {0L, } },
	{ (Class) &crTextVr, {0L, } },
	{ (Class) &crTextVr, {0L, } },
	{ (Class) &crTextVr, {0L, } },
};
#else
static HelpVrRec mv[4] = {
	{ (Class) &crHelpVr, {0, } },
	{ (Class) &crHelpVr, {0, } },
	{ (Class) &crHelpVr, {0, } },
	{ (Class) &crHelpVr, {0, } },
};
#endif
ViewRec rMsgView = {
	(Class)	&crPageView,
   {	
	(WINDOW *) 0L,		/* the window the view is in */
	(WINDOW *) 0L,		/* the window for the box around it */
	"",					/* the name */
	TRUE, 0, 0, 0, 0,	/* flag word -- is_centered = TRUE */
	DIRCOL,				/* window X position */
	DATAROW,			/* window Y position */
	VIEWCOLS,			/* window width */
	VIEWROWS,			/* window height */
	0,					/* data rows */
	(View) NIL,			/* associated view */
	0,					/* cursor row */
	0,					/* cursor column */
	(Viewer)&mv[0], (Viewer)&mv[1], (Viewer)&mv[2], (Viewer)&mv[3],
   },
};

static DriveVrRec dv[4] = {
	{ (Class) &crDriveVr, {0, 0} },
	{ (Class) &crDriveVr, {0, 0} },
	{ (Class) &crDriveVr, {0, 0} },
	{ (Class) &crDriveVr, {0, 0} },
};

ViewRec rDriveView = {
	(Class)	&crLineView,
   {	
	(WINDOW *) 0L,		/* the window the view is in */
	(WINDOW *) 0L,		/* the window for the box around it */
	"Drives",			/* the name */
	0, 0, 0, 0,	0,		/* flag word */
 	DIRCOL,				/* window X position */
	DATAROW,			/* window Y position */
	VIEWCOLS,			/* window width */
	VIEWROWS,			/* window height */
	0,					/* data rows */
	(View)NIL,			/* associated view */
	0,					/* cursor row */
	0,					/* cursor column */
	(Viewer)&dv[0], (Viewer)&dv[1], (Viewer)&dv[2], (Viewer)&dv[3],
   },
};

static VolVrRec vv[4] = {
	{ (Class) &crVolVr, {0, 0} },
	{ (Class) &crVolVr, {0, 0} },
	{ (Class) &crVolVr, {0, 0} },
	{ (Class) &crVolVr, {0, 0} },
};

ViewRec rVolView = {
	(Class)	&crLineView,
   {	
	(WINDOW *) 0L,		/* the window the view is in */
	(WINDOW *) 0L,		/* the window for the box around it */
	"Volume Formats",	/* the name */
	0, 0, 0, 0,	0,		/* flag word */
 	DIRCOL,				/* window X position */
	DATAROW,			/* window Y position */
	VIEWCOLS,			/* window width */
	VIEWROWS,			/* window height */
	0,					/* data rows */
	(View)NIL,			/* associated view */
	0,					/* cursor row */
	0,					/* cursor column */
	(Viewer)&vv[0], (Viewer)&vv[1], (Viewer)&vv[2], (Viewer)&vv[3],
   },
};

/*********************************************************************\
**
** View Globals
**
\*********************************************************************/

global View	oSrcDirView = &rSrcDirView;		/* Src Directory list */
global View	oDstDirView = &rDstDirView;		/* Dst Directory list */
global View	oSrcFileView= &rSrcFileView;	/* File list */
global View	oDstFileView= &rDstFileView;	/* File list */
global View	oTextView   = &rTextView;		/* File text */
global View	oHelpView   = &rHelpView;		/* Help Text */
global View	oInitView   = &rInitView;		/* Initial Screen */
global View	oMsgView    = &rMsgView;		/* Messages */
global View oDriveView  = &rDriveView;		/* Drive list */
global View oVolView    = &rVolView;		/* Volume list */

/*********************************************************************\
**
** Imported commands for menus
**
\*********************************************************************/

extern int cQuit();
extern int cToMain();
extern int cHelp();
extern int cHelpEsc();

extern int cVuBin();
extern int cVuEsc();
extern int cVuSlash();
extern int cVuAsc();
extern int cVuSlash();

extern int cDrives();
extern int cSelectDr();
extern int cSelectFS();
extern int cDriveType();
extern int cMediaType();

extern int cVolFmt();
extern int cVolMap();
extern int cVolFns();
extern int cVolExt();

extern int cReadTree();
extern int cWriteDir();
extern int cViewFile();
extern int cWriteFile();
extern int cNewDosD();
extern int cDosW();
extern int cSrcDrive();
extern int cDstDrive();
extern int cSelVol();

#ifdef V_RAW
extern int cRawDens();
extern int cRawDs();
extern int cRawSeclen();
extern int cRawSpt();
#endif


/*********************************************************************\
**
** Cursor-key decoding
**
\*********************************************************************/

static int kCtrl(cp)
	char *cp;
{
	switch (*cp) {
	 case ' ':
		return (MenuNext());
	 case '\b':
	 	return (MenuPrev());
	 case '\n': case '\r':
	 	return (MenuAct());
	}
 	return(0);
}

static int kHelp(cp)
	char *cp;
{
	if (*cp != 0) return(kCtrl(cp));		/* only the arrows change */

	switch (getCmd()) {
	 case K_C_HOME:
	 case K_HOME:	return(cHome(cp));
	 case K_UP:		return(cLnUp(cp));
	 case K_C_PGUP:
	 case K_PGUP:	return(cPgUp(cp));
	 case K_C_END:
	 case K_END:	return(cEnd(cp));
	 case K_DOWN:	return(cLnDn(cp));
	 case K_C_PGDN:
	 case K_PGDN:	return(cPgDn(cp));
	 case K_LEFT:	return(cHelpEsc((String)NULL));
	}
 	return(0);
}

static int kView(cp)
	char *cp;
{
	if (*cp != 0) return(kCtrl(cp));		/* only the arrows change */

	switch (getCmd()) {
	 case K_C_HOME:
	 case K_HOME:	return(cHome(cp));
	 case K_UP:		return(cLnUp(cp));
	 case K_C_PGUP:
	 case K_PGUP:	return(cPgUp(cp));
	 case K_C_END:
	 case K_END:	return(cEnd(cp));
	 case K_DOWN:	return(cLnDn(cp));
	 case K_C_PGDN:
	 case K_PGDN:	return(cPgDn(cp));
	 case K_LEFT:	return(cVuEsc((String)NULL));
	}
 	return(0);
}


static int kDir(cp)
	char *cp;
{
	if (*cp != 0) return(kCtrl(cp));;		/* only the arrows change */

	switch (getCmd()) {
	 case K_C_LEFT:
	 case K_LEFT:	return(cLeft(cp));
	 case K_C_RIGHT:
	 case K_RIGHT:	return(cRightToFiles(cp));
	 case K_UP:		return(cLnUp(cp));
	 case K_DOWN:	return(cLnDn(cp));
	 case K_C_PGUP:
	 case K_PGUP:	return(cPgUp(cp));
	 case K_C_PGDN:
	 case K_PGDN:	return(cPgDn(cp));
	 case K_C_HOME:
	 case K_HOME:	return(cHome(cp));
	 case K_C_END:
	 case K_END:	return(cEnd(cp));
	}
 	return(0);
}

static int kFile(cp)
	char *cp;
{
	if (*cp != 0) return(kCtrl(cp));;		/* only the arrows change */

	switch (getCmd()) {
	 case K_C_LEFT:
	 case K_LEFT:	return(cLeft(cp));
	 case K_C_RIGHT:
	 case K_RIGHT:	return(cViewFile(cp));
	 case K_UP:		return(cLnUp(cp));
	 case K_DOWN:	return(cLnDn(cp));
	 case K_C_PGUP:
	 case K_PGUP:	return(cPgUp(cp));
	 case K_C_PGDN:
	 case K_PGDN:	return(cPgDn(cp));
	 case K_C_HOME:
	 case K_HOME:	return(cHome(cp));
	 case K_C_END:
	 case K_END:	return(cEnd(cp));
	}
 	return(0);
}

static int kDrive(cp)
	char *cp;
{
	if (*cp != 0) return(kCtrl(cp));;		/* only the arrows change */

	switch (getCmd()) {
	 case K_C_LEFT:
	 case K_LEFT:	return(cLeft(cp));
	 case K_C_RIGHT:
	 case K_RIGHT:	return(cRight(cp));
	 case K_UP:		return(cLnUp(cp));
	 case K_DOWN:	return(cLnDn(cp));
	 case K_C_PGUP:
	 case K_PGUP:	return(cPgUp(cp));
	 case K_C_PGDN:
	 case K_PGDN:	return(cPgDn(cp));
	 case K_C_HOME:
	 case K_HOME:	return(cHome(cp));
	 case K_C_END:
	 case K_END:	return(cEnd(cp));
	}
 	return(0);
}

int kMain(cp)
	char *cp;
{
	if (*cp != 0) return(kCtrl(cp));		/* only the arrows change */

	switch (getCmd()) {
	 case 75:
	 case 115:	return (cLeft(cp));
	 case 116:
	 case 77:	return (cRight(cp));
	}
	return(1);
}


/*********************************************************************\
**
** Menus
**
\*********************************************************************/

MenuDescriptor NormalMenuDscr = {
	FALSE,		/* keyword select */
	TRUE,		/* cursor select */
	0, 0,		/* row, column (relative to MenuWindow)*/
	80,	2,		/* width, height */
	1,			/* spacing */
	twoLine		/* style */
};

MenuSTART(QuitMenu)
	 POP('N', "NO",		 "Really quit?")
	EXIT('Y', "YES",	 "Really quit!",					cQuit, 0)
MenuEND

MenuSTART(HelpMenu)
	HEAD(1, "HELP:",
		 "Help:  cursor keys to move, Esc or \256 to quit.",	0, 0)

	CALL('Q',	"Quit",		"Exit the program.",			QuitMenu)
	ITEM('/',	"/",	"/: Return to main menu.",			cToMain, 0)
	ITEM(0x1b,	"Esc",		
		 "Leave Help (Return to previous screen.)",			cHelpEsc, 0)
	CTRL(kHelp)
MenuEND

MenuSTART(VuAscMenu)
	HEAD(1, "ASCII:",	
		 "View Ascii File:  cursor keys to move, Esc or \256 to quit.",
		 /* status */ 0, 0)
	ITEM('B',	"Binary",
		 "Switch to Binary view.",							cVuBin, 0)
	ITEM('F',	"Files(\256)", 
		 "Switch to File window.",							cVuEsc, 0)

	CALL('Q',	"Quit",		"Exit the program.",			QuitMenu)
	ITEM('?', 	"?",		"? Displays help (any time)",	cHelp, selectHelp)
	ITEM('/',	"/",		"Return to main menu.",			cVuSlash, 0)
	ITEM(0x1b,	"Esc",		
		 "Stop Viewing.  (Return to File window.)",			cVuEsc, 0)
	CTRL(kView)
MenuEND


MenuSTART(VuBinMenu)
	HEAD(1, "BINARY:",	
		 "View Binary File:  cursor keys to move; Esc or \256 to quit.",
		 /* status */ 0, 0)
	ITEM('A',	"Ascii",
		 "Switch to Ascii view.",							cVuAsc, 0)
	ITEM('F',	"Files(\256)", 
		 "Switch to File window",							cVuEsc, 0)

	CALL('Q',	"Quit",		"Exit the program.",			QuitMenu)
	ITEM('?', 	"?",		"? Displays help (any time)",	cHelp, selectHelp)
	ITEM('/',	"/",		"Return to main menu.",			cVuSlash, 0)
	ITEM(0x1b,	"Esc",		
		 "Stop Viewing.  (Return to File window.)",			cVuEsc, 0)
	CTRL(kView)
MenuEND

#if 0
MenuSTART(SelectDrMenu)
	HEAD(1, "Drive:",	
		 "Select a drive",
		 0, 0)
	ITEM('A',	"A:(0)",	"Drive A (physical drive 0)",	cSelectDr, "A:")
	ITEM('B',	"B:(1)",	"Drive B (physical drive 1)",	cSelectDr, "B:")
	ITEM('C',	"C:",		"Drive C",						cSelectDr, "C:")
	ITEM('D',	"D:",		"Drive D",						cSelectDr, "D:")
	ITEM('E',	"E:",		"Drive E",						cSelectDr, "E:")
	ITEM('F',	"F:",		"Drive F",						cSelectDr, "F:")
	ITEM('G',	"G:",		"Drive G",						cSelectDr, "G:")
	ITEM('H',	"H:",		"Drive H",						cSelectDr, "H:")
	ITEM('I',	"I:",		"Drive I",						cSelectDr, "I:")
	ITEM('J',	"J:",		"Drive J",						cSelectDr, "J:")

	CALL('Q',	"Quit",		"Exit the program.",			QuitMenu)
	ITEM('?', 	"?",		"? Displays help (any time)",	cHelp, selectHelp)
	ITEM('/',	"/",		"Return to main menu.",			cToMain, 0)
	ITEM(0x1b,	"Esc",		"Return to Directory view.",	MenuPop, 0)
MenuEND
#endif

MenuSTART(SelectFSMenu)
	HEAD(1, "Operating System:",	
		 "Select an Operating System for the highlighted drive",
		 0, 0)
	ITEM('D',	"DOS",		"MS-DOS",						cSelectFS, DOS_FS)
	ITEM('I',	"iRMX-86",	"iRMX-86",						cSelectFS, RMX_FS)
#ifdef V_RAW
	ITEM('R',	"RAW",		"Raw Disk",						cSelectFS, RAW_FS)
#endif

	CALL('Q',	"Quit",		"Exit the program.",			QuitMenu)
	ITEM('?', 	"?",		"? Displays help (any time)",	cHelp, fileSysHelp)
	ITEM('/',	"/",		"Return to main menu.",			cToMain, 0)
	ITEM(0x1b,	"Esc",		"Return to Directory view.",	MenuPop, 0)
MenuEND

MenuSTART(DriveTypeMenu)
	HEAD(1, "Drive Type:",	
		 "Select a drive type",
		 0, 0)
	ITEM('A',	"AT",
		 "AT: high-density disk",							cDriveType, "A")
	ITEM('C',	"Compati-Card",
		 "Compati-Card (Toggle): able to read single-density", cDriveType, "C")
	ITEM('H',	"Hard",
		 "Hard disk disk",									cDriveType, "H")
	ITEM('P',	"PC",
		 "PC: normal (double) density disk",				cDriveType, "P")

	CALL('Q',	"Quit",		"Exit the program.",			QuitMenu)
	ITEM('?', 	"?",		"? Displays help (any time)",	cHelp, driveTypeHelp)
	ITEM('/',	"/",		"Return to main menu.",			cToMain, 0)
	ITEM(0x1b,	"Esc",		"Return to Directory view.",	MenuPop, 0)
MenuEND

#ifdef V_RAW

MenuSTART(RawDensMenu)
	HEAD(1, "Density:",	
		 "Select a density",
		 0, 0)
	ITEM('D',	"Double",	"Double Density (PC)",			cRawDens, "1")
	ITEM('H',	"High",		"High Density (AT)",			cRawDens, "2")
	ITEM('S',	"Single",	"Single Density",				cRawDens, "0")

	CALL('Q',	"Quit",		"Exit the program.",			QuitMenu)
	ITEM('?', 	"?",		"? Displays help (any time)",	cHelp, paramHelp)
	ITEM('/',	"/",		"/: Return to main menu.",		cToMain, 0)
	ITEM(0x1b,	"Esc",		"Return to Parameter Menu.",	MenuPop, 0)
MenuEND

MenuSTART(RawDsMenu)
	HEAD(1, "Heads:",	
		 "Select # of heads (sides)",
		 0, 0)
	ITEM('1',	"1",		"Single Sided",					cRawDs, "0")
	ITEM('S',	"Single",	"Single Sided",					cRawDs, "0")
	ITEM('2',	"2",		"Double Sided (PC)",			cRawDs, "1")
	ITEM('D',	"Double",	"Double Sided (PC)",			cRawDs, "1")

	CALL('Q',	"Quit",		"Exit the program.",			QuitMenu)
	ITEM('?', 	"?",		"? Displays help (any time)",	cHelp, paramHelp)
	ITEM('/',	"/",		"/: Return to main menu.",		cToMain, 0)
	ITEM(0x1b,	"Esc",		"Return to Parameter Menu.",	MenuPop, 0)
MenuEND

MenuSTART(RawSeclenMenu)
	HEAD(1, "Sector Length:",	
		 "Select # bytes per sector",
		 0, 0)
	ITEM('0',	"0128",		"128",							cRawSeclen, "128")
	ITEM('2',	"256",		"256",							cRawSeclen, "256")
	ITEM('5',	"512",		"512",							cRawSeclen, "512")
	ITEM('1',	"1024",		"1024",							cRawSeclen, "1024")

	CALL('Q',	"Quit",		"Exit the program.",			QuitMenu)
	ITEM('?', 	"?",		"? Displays help (any time)",	cHelp, paramHelp)
	ITEM('/',	"/",		"/: Return to main menu.",		cToMain, 0)
	ITEM(0x1b,	"Esc",		"Return to Parameter Menu.",	MenuPop, 0)
MenuEND

MenuSTART(RawSptMenu)
	HEAD(1, "Track Length:",	
		 "Select # sectors per track",
		 0, 0)
	ITEM('2',	"2",		"2",							cRawSpt, "2")
	ITEM('4',	"4",		"4",							cRawSpt, "4")
	ITEM('8',	"8",		"8",							cRawSpt, "8")
	ITEM('9',	"9",		"9",							cRawSpt, "9")
	ITEM('1',	"15",		"15",							cRawSpt, "15")
	ITEM('1',	"16",		"16",							cRawSpt, "16")

	CALL('Q',	"Quit",		"Exit the program.",			QuitMenu)
	ITEM('?', 	"?",		"? Displays help (any time)",	cHelp, paramHelp)
	ITEM('/',	"/",		"/: Return to main menu.",		cToMain, 0)
	ITEM(0x1b,	"Esc",		"Return to Parameter Menu.",	MenuPop, 0)
MenuEND

MenuSTART(RawParamMenu)
	HEAD(1, "Parameters:",	
		 "Specify drive parameters",
		 0, 0)
	CALL('D',	"Density",	"Select Drive Density",			RawDensMenu)
	CALL('H',	"Heads",	"Select Heads (# sides)",		RawDsMenu)
	CALL('S',	"Sector",	"Select Sector Size",			RawSeclenMenu)
	CALL('T',	"Track",	"Select Track Size",			RawSptMenu)

	CALL('Q',	"Quit",		"Exit the program.",			QuitMenu)
	ITEM('?', 	"?",		"? Displays help (any time)",	cHelp, paramHelp)
	ITEM('/',	"/",		"/: Return to main menu.",		cToMain, 0)
	ITEM(0x1b,	"Esc",		"Return to Directory/File view.",	MenuPop, 0)
MenuEND

#endif

#if 0
MenuSTART(ConvertMenu)
	ITEM('A', "Ascii",
		  "Mark files for Ascii conversion",				cCAscii, 0)
	ITEM('B', "Binary",
		  "Mark files for Binary (no) conversion",	 		cCBin, 0)

	CALL('Q',	"Quit",		"Exit the program.",			QuitMenu)
	ITEM('/',	"/",		"/: Return to main menu.",		cToMain, 0)
	ITEM('?', 	"?",		"? Displays help (any time)",	cHelp, convertHelp)
	ITEM(0x1b,	"Esc",		"Return to Directory view.",	MenuPop, 0)
MenuEND
#endif

MenuSTART(SrcDirMenu)
	HEAD(1, "FROM:", 	
		 "Source Directory Window: cursor keys move up and down",	
		 0,		0)
#ifdef V_CVT
	ITEM('A', "Ascii",
		  "Mark files for Ascii conversion",				cAscii, 0)
	ITEM('B', "Binary",
		  "Mark files for Binary (no) conversion",	 		cBin, 0)
#endif
	ITEM('D', "Drives",
		 "Select drive in Drive list window",				cDrives, 0)
#ifdef V_RAW
	CALL('P', "Parameters",
		 "Set raw drive parameters.",						RawParamMenu)
#endif
	ITEM('R', "Read",	
		 "Read directory tree from disk.",					cReadTree, 0)
	ITEM('T', "Tag",	
		 "Tag everything under highlighted directory",		cTag,	0)
	ITEM('U', "Untag",	
		 "Untag everything under highlighted directory",	cUntag, 0)
#ifndef V_DE
	ITEM('W', "Write",
		 "Write tagged files under highlighted directory",	cWriteDir, 0)
#else
	ITEM('W', "Write",
		 "Write tagged files under highlighted directory",	cHelp, demoHelp)
#endif
	CALL('Q', "Quit",   "Exit the program", 				QuitMenu)
	ITEM('/', "/",		"/: Return to main menu.",			cToMain, 0)
	ITEM('?', "?",		"? Displays help (any time)",		cHelp, dirHelp)
	CTRL(kDir)
MenuEND


MenuSTART(SrcFileMenu)
	HEAD(1, "FILES:", 	
		 "File Window:  cursor keys move up and down",
		 0,	0)
#ifdef V_CVT
	ITEM('A', "Ascii",
		  "Mark file for Ascii conversion",					cAscii, 0)
	ITEM('B', "Binary",
		  "Mark file for Binary (no) conversion",	 		cBin, 0)
#endif
#ifdef V_RAW
	CALL('P', "Parameters",
		 "Set raw drive parameters for this track.",		RawParamMenu)
#endif
	ITEM('T', "Tag",	
		 "Tag highlighted file",							cTag,	0)
	ITEM('U', "Untag",	
		 "Untag highlighted file",							cUntag, 0)
	ITEM('V', "View(\257)",
		 "View file (in hex if not Ascii type)",			cViewFile, 0)
#ifndef V_DE
	ITEM('W', "Write",
		 "Write highlighted file to current directory",		cWriteFile, 0)
#else
	ITEM('W', "Write",
		 "Write highlighted file to current directory",		cHelp, demoHelp)
#endif
	CALL('Q', "Quit",   "Exit the program", 				QuitMenu)
	ITEM('/', "/",		"Return to main menu.", 			cToMain, 0)
	ITEM('?', "?",		"? Displays help (any time)",		cHelp, fileHelp)
	CTRL(kFile)
MenuEND


MenuSTART(DstDirMenu)
	HEAD(1, "TO:", 	
		 "Destination Directory Window: cursor keys move up and down",	
		 0,		0)
	ITEM('D', "Drives",
		 "Select drive in Drive list window",				cDrives, 0)
	ITEM('N', "New", 
		 "create New directory",							cNewDosD, 0)
	ITEM('O', "Output",	
		 "Select output (working) directory",				cDosW, 0)
	ITEM('R', "Read",
		 "Read directory tree from disk.",					cReadTree, 0)
	ITEM('T', "Tag",	
		 "Tag everything under highlighted directory",		cTag,	0)
	ITEM('U', "Untag",	
		 "Untag everything under highlighted directory",	cUntag, 0)
#ifndef V_DE
	ITEM('W', "Write",
		 "Write tagged files under highlighted directory",	cWriteDir, 0)
#else
	ITEM('W', "Write",
		 "Write tagged files under highlighted directory",	cHelp, demoHelp)
#endif
	CALL('Q', "Quit",   "Exit the program", 				QuitMenu)
	ITEM('/', "/",		"Return to main menu.", 			cToMain, 0)
	ITEM('?', "?",		"? Displays help (any time)",		cHelp, dstDirHelp)
	CTRL(kDir)
MenuEND


MenuSTART(DstFileMenu)
	HEAD(1, "FILES:", 	
		 "Destination File Window: cursor keys move up and down",
		 0,	0)
#ifdef V_CVT
	ITEM('A', "Ascii",
		  "Mark file for Ascii viewing",					cAscii, 0)
	ITEM('B', "Binary",
		  "Mark file for Binary viewing",			 		cBin, 0)
#endif
	ITEM('T', "Tag",	
		 "Tag highlighted file",							cTag,	0)
	ITEM('U', "Untag",	
		 "Untag highlighted file",							cUntag, 0)
	ITEM('V', "View(\257)",
		 "View file (in hex if not Ascii type)",			cViewFile, 0)
#ifndef V_DE
	ITEM('W', "Write",
		 "Write highlighted file to current directory",		cWriteFile, 0)
#else
	ITEM('W', "Write",
		 "Write highlighted file to current directory",		cHelp, demoHelp)
#endif
	CALL('Q', "Quit",   "Exit the program", 				QuitMenu)
	ITEM('/', "/",		"Return to main menu.", 			cToMain, 0)
	ITEM('?', "?",		"? Displays help (any time)",		cHelp, dstFileHelp)
	CTRL(kFile)
MenuEND


MenuSTART(MediaTypeMenu)
	HEAD(1, "Media Type:",	
		 "Select a media type",
		 0, 0)
/*	ITEM('S',	"Single(180K)",	
		"Single Density (180K)"								,cMediaType, "16") */
	ITEM('D',	"Double(360K)",	
		"Double Density (360K) -- Standard PC disk", 		cMediaType, "0")
	ITEM('M',	"Medium(720K)",	
		"Medium Density (720K) -- iRMX high density",		cMediaType, "1")
	ITEM('H',	"High(1.2M)",	
		"Double Density (1.2M) -- AT type disk",			cMediaType, "3")

	CALL('Q',	"Quit",		"Exit the program.",			QuitMenu)
	ITEM('?', 	"?",		"? Displays help (any time)",	cHelp, paramHelp)
	ITEM('/',	"/",		"/: Return to main menu.",		cToMain, 0)
	ITEM(0x1b,	"Esc",		"Return to Parameter Menu.",	MenuPop, 0)
MenuEND



MenuSTART(VolMenu)
	HEAD(1, "VOLUME:",	
		 "Volume Format Menu:  examine and set volume formats.",
		 0, 0)
	ITEM('E', "ExtSize",
		 "Enter Extension Size for highlighted volume",		cVolExt, 0)
	ITEM('F', "Files",
		 "Enter Files for highlighted volume",				cVolFns, 0)
	ITEM('M', "MapStart",
		 "Enter Fnode Map Start for highlighted volume",	cVolMap, 0)
	ITEM('S', "Select",
		 "Select hilited format for current drive",			cSelVol, 0)
	CALL('Q', "Quit",   "Exit the program", 				QuitMenu)
	ITEM('/', "/",		"Return to main menu.", 			cToMain, 0)
	ITEM('?', "?",		"? Displays help (any time)",		cHelp, volHelp)
	ITEM(0x1b,	"Esc",	"Return to Drive view.",			cLeft, 0)
	CTRL(kDrive)
MenuEND


MenuSTART(DriveMenu)
	HEAD(1, "DRIVES:",	
		 "Drive Menu:  examine and set drive characteristics.",
		 0, 0)
	CALL('D', "DriveType", 
		 "Select type of drive (PC, AT, etc.)",				DriveTypeMenu)
	CALL('M', "MediaType", 
		 "Select type of media (HD:1.2, MD:720, DD:360)",	MediaTypeMenu)
	CALL('S', "System", 
		 "Select operating System for highlighted drive",	SelectFSMenu)
	ITEM('V', "VolumeFmt",
		 "Specify volume format for highlighted drive",		cVolFmt, 0)
	ITEM('F', "From(\256)",
		 "Specify Source drive; go to From dir. view",		cSrcDrive, 0)
	ITEM('T', "To(\257)",
		 "Specify Destination drive; go to To dir. view",	cDstDrive, 0)
	CALL('Q', "Quit",   "Exit the program", 				QuitMenu)
	ITEM('/', "/",		"Return to main menu.", 			cToMain, 0)
	ITEM('?', "?",		"? Displays help (any time)",		cHelp, driveHelp)
	ITEM(0x1b,	"Esc",	"Go to From view.",					cLeft, 0)
	CTRL(kDrive)
MenuEND


MenuSTART(MainMenu)
	HEAD(1, "MAIN:",	
		 "Main menu:  space & backspace to move; Enter or letter to select",
		 0, 0)
	ITEM('A', "About",
		 "About this program (an overview)",				cHelp, ovHelp)
	ITEM('D', "Drives",
		 "To Drive list window",							scrOpen, &DriveScreen)
	ITEM('F', "From(\256)",
		 "To Source directory window",						cLeft, 0)
	ITEM('T', "To(\257)",
		 "To Destination directory window",					cRight, 0)
	CALL('Q', "Quit",   "Exit the program", 				QuitMenu)
	ITEM('?', "?",		"? Displays help (any time)",		cHelp, mainHelp)
	CTRL(kMain)
MenuEND


/*********************************************************************\
**
** Screen Descriptors
**
\*********************************************************************/

ScreenRec MainScreen = {
	"RWi",				&rInitView,				MainMenu,
	0L, 				&SrcDirScreen,			&DstDirScreen,
	0,
};

ScreenRec DriveScreen = {
	"Drives",			&rDriveView,			DriveMenu,
	0L,			 		&SrcDirScreen,			&DstDirScreen,
	0,
};

ScreenRec VolScreen = {
	"Volume Formats",	&rVolView,				VolMenu,
	0L,			 		&DriveScreen,			&DriveScreen,
	0,
};

ScreenRec SrcDirScreen = {
	"From",				&rSrcDirView,			SrcDirMenu,
	&SrcFileScreen, 	&DstDirScreen,			&SrcFileScreen,
	dirStat,
};

ScreenRec SrcFileScreen = {
	"Files",			&rSrcFileView,			SrcFileMenu,
	&SrcDirScreen, 		&SrcDirScreen,			&ViewScreen,
	treeStat,
};

ScreenRec DstDirScreen = {
	"To",				&rDstDirView,	 		DstDirMenu,
	&DstFileScreen, 	&SrcDirScreen,			&DstFileScreen,
	dirStat,
};

ScreenRec DstFileScreen = {
	"Files",			&rDstFileView,			DstFileMenu,
	&DstDirScreen, 		&DstDirScreen,			&ViewScreen,
	treeStat,
};

ScreenRec ViewScreen = {
	"View",				&rTextView,				0L,
	0L, 				0L,						0L,
	0,
};

ScreenRec HelpScreen = {
	"Help",				&rHelpView,				0L,
	0L, 				0L,						0L,
	0,
};

/*********************************************************************\
**
** Initialization
**
\*********************************************************************/

global void dataInit(imsg)
	char **imsg;
{
	extern WINDOW *MenuWindow;
	extern void hline(), vlines();

	/* === At this point we ought to extract screen parameters from stdscr */

	/*
	** Initialize the views.
	*/
	gInit(oSrcDirView);
	gInit(oDstDirView);
	gInit(oSrcFileView);
	gInit(oDstFileView);
	gInit(oTextView);
	gInit(oHelpView);
	gInit(oInitView);
	gInit(oMsgView);
	gInit(oDriveView);
	gInit(oVolView);

	setRoot(oInitView, imsg);
	setRoot(oDriveView, NULL);
	setRoot(oVolView, NULL);

	MenuWindow = newwin(2, swidth - 4, menuRow, 2);
	hline (stdscr, 0, swidth, D_UL, D_HORIZ, D_UR);
	vlines(stdscr, 1, swidth, headerRow - 1, D_VERT);
}

