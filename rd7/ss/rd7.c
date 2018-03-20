/***/ static char *pgmid = "RD7 1.0 copyright 1989 S. Savitzky"; /***/

/*********************************************************************\
**
**	RD7 -- selectively copy files from 7000 to PC format
**
**	Command-line options
**
**	880111 SS	create PC version from DD
**
\*********************************************************************/

#include <stdlib.h>
#include <malloc.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <direct.h>
#include <dos.h>

#include "../lib/curse.h"
#include "../lib/ibmkeys.h"

#include "rd7.h"
#include "coops.h"
#include "trees.h"
#include "dirs.h"
#include "view.h"
#include "disk.h"
#include "help.h"
#include "convert.h"

#define MenuMACROS 1
#include "../lib/menu.h"

#undef  global
#define global

/*********************************************************************\
**
** Program Organization:
**
**		rd7.c			main program; menu command handlers
**		help.c			help screens
**		dirs.c			directory and file tree structures
**		disk.c			data handling for 7000 format disks
**		view.c			screen handling for directory and contents windows
**
**	Library Modules:
**
**		coops.c			C object-oriented programming system
**		trees.c			tree object class for Coops
**		curse.c			subset of Unix curses packages
**		menu.c			menu system
**
\*********************************************************************/


/*********************************************************************\
**
**	V A R I A B L E S
**	
\*********************************************************************/

#define dRoot		((Dir)(oDirView -> treeview.root))
#define dDosRoot	((Dir)(oDosView -> treeview.root))

global String dDosDrive;		/* MS-DOS drive string */
static char dosDriveBuf[3] = "A:";

global int debugf = 0;

String errorMsg;				/* set by errorSet					*/

View theView;					/* currently-active view			*/
View helpedView;				/* theView before invoking Help		*/
View viewedFileView;			/* current view of a file			*/

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
	gVuSet(oInitView, errorText);
	gOpen(oInitView);
	getCmd();			 /* === ought to use menu === */
	if (theView -> view.partner) gOpen(theView -> view.partner);
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
** botLine			put useful stuff on the bottom line
** savescr()		save the screen during graphics
** restscr()		restore the screen after graphics
**
\*********************************************************************/

int getCmd()
{
	return(getch());
}

void botLine()
{
	move(stdscr -> maxy - 1, 0);
	clrtoeol();
	wprintf(stdscr, "  Idris: %d  Dos: %c:  ", dDrive, 
			(!dDosDrive || *dDosDrive == '.') ? dWorkingPath[0]
											  : *dDosDrive
		   );
	move(24, 40);

	if (dJcampHdr) {
		move(24, 21);
		wprintf(stdscr, "J: %s  ", dJcampHdr);
		if (strlen(dJcampHdr) < 14)	 move(24, 40);
	}

	addstr("Write to: ");
	addstr(dOutputDrive ? dOutputPath : "<output directory invalid>");
}

static int x, y, scrx, scry;		/* screen-saver variables */
static short *oldscr;

static void savescr()
{
	scrx = stdscr -> maxx;
	scry = stdscr -> maxy;
	oldscr = (short *) malloc(scrx * scry * sizeof(short));
	for (y = 0; y < stdscr -> maxy; ++y)
		for (x = 0; x < stdscr -> maxx; ++x)
			oldscr[y * scrx	+ x] = wincha(stdscr, y, x);
}

static void restscr()
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


static View dViewToUpdate;

static void dStatusEachDir(d)
	Dir d;
{
	move(24, 0);
	addstr(gPath(d, NIL));
	clrtoeol();
	gVuUpdLine(oInitView);
}


static void dReadStart(n)
	Dir  n;
{
	View v = dViewToUpdate;

	gVuSet(v, n);
	gVuSet(oInitView, readingText);
	oInitView -> view.partner = v;
	gOpen(oInitView);
	dEachNewDir = dStatusEachDir;
}

global void dWriteStart(v, n)
	View v;
	Dir  n;
{
	if (v) {
		gVuSet(oInitView, writingText);
		oInitView -> view.partner = v;
		gOpen(oInitView);
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
**		The idea of partners needs to be re-thought some.
**
\*********************************************************************/

typedef struct screen *Screen;

typedef struct screen {
	char	*label;
	View	*view;
	Menu	menu;
	Screen	partner;
	Screen	left;
	Screen	right;
} ScreenRec;

Screen theScreen;


static int scrOpen(s)
	Screen s;
{
	static char lbl[32];

	theScreen = s;
	if (s -> view)
		theView = *s -> view;
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
	if (theScreen -> partner) {
		if (theScreen -> right == theScreen -> partner) {
			theView -> view.partner -> view.is_active  = FALSE;
		}
		theView -> view.partner -> view.do_refresh = TRUE;
		gVuLabel(theView -> view.partner, 
				 gName(theView -> view.partner), "=\315");
	}
	gVuRefresh(theView);
	botLine();
	if (theScreen -> menu)
		return(MenuJump(theScreen -> menu));
	else
		return(1);
}

/*********************************************************************\
**
** Screen Descriptors
**
\*********************************************************************/

MenuItem MainMenu[],	HelpMenu[],
		 DirMenu[],		FileMenu[], 
		 DosMenu[],		DosFileMenu[],
		 VuAscMenu[],	VuBinMenu[];

ScreenRec HelpScreen,		ViewScreen,
		  IdrisDirScreen,	IdrisFileScreen,
		  DosDirScreen, 	DosFileScreen;

ScreenRec MainScreen = {
	"rd7",				&(View)oInitView,		MainMenu,
	0L, 				&DosDirScreen,			&IdrisDirScreen,
};

ScreenRec IdrisDirScreen = {
	"Idris",			&(View)oDirView, 		DirMenu,
	&IdrisFileScreen, 	&DosDirScreen,			&IdrisFileScreen,
};

ScreenRec IdrisFileScreen = {
	"Files",			&(View)oFileView,		FileMenu,
	&IdrisDirScreen, 	&IdrisDirScreen,		&ViewScreen,
};

ScreenRec DosDirScreen = {
	"Dos",				&(View)oDosView,	 	DosMenu,
	&DosFileScreen, 	&IdrisDirScreen,		&DosFileScreen,
};

ScreenRec DosFileScreen = {
	"Files",			&(View)oDosFileView,	DosFileMenu,
	&DosDirScreen, 		&DosDirScreen,			&ViewScreen,
};

ScreenRec ViewScreen = {	/* ascii/binary gets filled in later */
	"View",				&(View)viewedFileView,	0L,
	0L, 				0L,						0L,
};

ScreenRec HelpScreen = {
	"Help",				&(View)oHelpView,		0L,
	0L, 				0L,						0L,
};


/*********************************************************************\
**
** Command Procedures accessed from Menus
**		All take an optional char* parameter.
**
\*********************************************************************/

/* 
** Any Menu Commands 
*/
int cNull(s)	char *s; {return(1);}
int cError(s)	char *s; {return(0);}

int cHelp(s)
	char *s;
{
	MenuCall(HelpMenu);				/* only do this to save previous one */
	HelpScreen.left = theScreen;
	gVuSet(oHelpView, s);
	return(scrOpen(&HelpScreen));
}

int cHelpEsc(s)
	char *s;
{
	scrOpen(HelpScreen.left);
	return(MenuPop());		 /* Ensure that we get back to the right menu */
}

int cQuit(s)
	char *s; 
{
	return(1);
}

int cToMain(s)
	char *s;
{
	gVuSet(oInitView, initMsg);
	return(scrOpen(&MainScreen));
}

/*********************************************************************\
**
** Generic Movement Commands
**
**		These operate on theView
**
\*********************************************************************/

static int cLnUp(s)
	char *s;
{
	gVuLnUp(theView);
	gVuRefresh(theView);
	return(1);
}

static int cLnDn(s)
	char *s;
{
	gVuLnDn(theView);
	gVuRefresh(theView);
	return(1);
}

static int cPgUp(s)
	char *s;
{
	gVuPgUp(theView);
	gVuRefresh(theView);
	return(1);
}

static int cPgDn(s)
	char *s;
{
	gVuPgDn(theView);
	gVuRefresh(theView);
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
	TreeView v = (TreeView) (theView -> view.partner);
	if (ISNULL(v -> treeview.root)) return(0);
	return(cRight(s));
}


/* These are just like the preceeding except they also used to MenuPop */

int cUpKey(s)
	char *s;
{
	cLnUp(s);
	return(1);
}

int cDnKey(s)
	char *s;
{
	cLnDn(s);
	return(1);
}

int cPgUpKey(s)
	char *s;
{
	cPgUp(s);
	return(1);
}

int cPgDnKey(s)
	char *s;
{
	cPgDn(s);
	return(1);
}


int cHomeKey(s)
	char *s;
{
	gVuFirst(theView);
	gVuRefresh(theView);
	return(1);
}

int cEndKey(s)
	char *s;
{
	gVuLast(theView);
	gVuRefresh(theView);
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
	TreeView v = (TreeView)theView;
	Dir f = (Dir)(v -> treeview.cur);

	if (ISNULL(v -> treeview.root) || ISNULL(f) || !tag1(f, m)) return(0);
	if (f -> dir.isDir) {
		gVuUpdAll(v);
	}
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

int cCss(s)
	char *s;
{
	return(tag(css));
}

int cJcamp(s)
	char *s;
{
	return(tag(jcamp));
}

int cCJcamp(s)
	char *s;
{
	MenuPop();
	return(tag(jcamp));
}

int cSpectrum(s)
	char *s;
{
	return(tag(spectrum));
}

int cCSpectrum(s)
	char *s;
{
	MenuPop();
	return(tag(spectrum));
}

int cTag(s)
	char *s;
{
	return(tag(unknown));
}


int cUntag(s)
	char *s;
{
	TreeView v = (TreeView)theView;
	Dir f = (Dir)(v -> treeview.cur);

	if (ISNULL(f) || ISNULL(v -> treeview.root)) return(0);
	dUntag(f);
	return (cLnDn());
}

/*********************************************************************\
**
** Menu-Specific Commands
**
\*********************************************************************/


int cVuEsc(s)
	char *s;
{
	FileView v = (FileView)theView;
	BufFree (v -> fileview.origin);
	return(cLeft(s));
}

int cVuSlash(s)
	char *s;
{
	FileView v = (FileView)theView;
	BufFree (v -> fileview.origin);
	return (cToMain(s));
}

int cVuAsc(s)
	char *s;
{
	register int i;
	FileView v  = (FileView)theView;
	FileView tv = (FileView)oTextView;

	theView = (View) tv;
	tv -> fileview.origin = v -> fileview.origin;
	tv -> fileview.limit  = v -> fileview.limit;
	/* scan back to BOL */
	for (i = v -> fileview.top;
		 i && tv -> fileview.origin[i-1] != '\n';
		 --i 
		) ;
	tv -> fileview.top = tv -> fileview.cur = i;
	tv -> view.do_update = TRUE;
	gOpen(theView);
	*ViewScreen.view = theView;
	return (MenuJump(VuAscMenu));
}

int cVuBin(s)
	char *s;
{
	FileView v  = (FileView)theView;
	FileView tv = (FileView)oBinaryView;

	theView = (View)oBinaryView;
	tv -> fileview.origin= v -> fileview.origin;
	tv -> fileview.top   = tv -> fileview.cur = v -> fileview.cur & ~0xfL;
	tv -> fileview.limit = v -> fileview.limit;
	tv -> view.do_update = TRUE;
	gOpen(theView);
	*ViewScreen.view = theView;
	return (MenuJump(VuBinMenu));
}

/*
** File menu commands 
*/

int cWriteF(s)
	char *s;
{
	TreeView v = (TreeView) theView;

	if (ISNULL(v -> treeview.root)) return (0);
	if (!dValidateDest() || !dValidateIdris(v -> treeview.cur)) 
		return (0);
	dCopyFile(v -> treeview.cur, TRUE);
	gVuRefresh(oDirView);
	botLine();
	return (cLnDn());
}


cViewF(s)
	char *s;
{
	TreeView v = (TreeView) theView;
	FileView fv;
	register Dir   f = (Dir)(v -> treeview.cur);
	char *cp;
	char huge *viewData;

	if (ISNULL(f) || ISNULL(v -> treeview.root)) return (0);
	if (! (viewData = dReadIdrisFile(f))) return (0);

	/* === ought to check for ascii? file being binary === */
	/* === if it is, beep and exit === */

	switch (f -> dir.mode) {
	 case unknown:
#ifdef V_SP
		cp = gName(f);
	  	if (strlen(cp) >= 3 && !strcmp(".sp", cp + strlen(cp) - 3))
			goto vuSpec;
#endif
	 case ascii:
	 	fv = oTextView;
		MenuJump(VuAscMenu);
		break;
#ifdef V_SP
	 case spectrum:
	 case jcamp:
	 case css:
     vuSpec:
	 	savescr();
		graphsp(f -> dir.name, viewData, FALSE);
		BufFree(viewData);
		restscr();
	 	return (1);
#endif
	 default:
		fv = oBinaryView;
	 	MenuJump(VuBinMenu);
	}

	theView = (View)fv;
	fv -> fileview.origin = viewData;
	fv -> fileview.limit  = f -> dir.size;
	fv -> fileview.top = fv -> fileview.cur = 0;
	gVuFirst(theView);
	GetPath(f, theView -> view.name);
	*ViewScreen.view = theView;
	ViewScreen.left = theScreen;
	return (scrOpen(&ViewScreen));
}

/*
** Directory menu commands 
*/
int cToDirs(s)
	char *s;
{
	return( scrOpen(&IdrisDirScreen));
}

int cIdrisRead(s)
	char *s;
{
	register TreeView v = oDirView;

	if (NOTNULL(v -> treeview.root)) {
		/*
		** There's an old tree: delete it.
		** (even if on same drive, might have swapped disks.)
		*/
		inUse[dDriveNum(gName(v -> treeview.root))] = FALSE;
		gVuReset(v);
	}
	dEachNewDir   = dReadStart;
	dViewToUpdate = (View) v;
	dReadDisk(dDrive);
	dEachNewDir   = (VoidFunc) 0;
	/*
	** Invalidate working and output drives if necessary
	*/
	if (dDrive == dWorkingDrive - 1) dWorkingDrive = 0;
	if (dDrive == dOutputDrive - 1) dOutputDrive = 0;
	return(cToDirs(""));
}


int cWriteD(s)
	char *s;
{
	TreeView v = (TreeView)theView;

 	if (ISNULL(v -> treeview.root)) return (0);
	if (!dValidateDest() || !dValidateIdris(v -> treeview.cur)) 
		return (0);
	dCopyDirs(v -> treeview.cur, TRUE);
	botLine();
	return (cLnDn());
}

/*
** DOS Menu Commands
*/
int cToDos(s)
	char *s;
{
	return(scrOpen(&DosDirScreen));
}

int cDosRead(s)
	char *s;
{
	register TreeView v = oDosView;
	register Dir d;
	static char drive[3] = {0, ':', 0};

	if (NOTNULL(v -> treeview.root)) {
		/*
		** Actually killing the old tree is left for dReadDosTree
		*/
		inUse[dDriveNum(gName(v -> treeview.root))] = FALSE;
		gVuReset(v);
	}
	if (!dDosDrive) {
		if (strlen(dWorkingPath) > 3)
			dDosDrive = ".";
		else {
			dDosDrive = drive;			/* we're at the top, so use the */
			drive[0]  = *dWorkingPath;	/* whole drive without any fuss */
		}
	}
	dEachNewDir   = dReadStart;
	dViewToUpdate = (View) v;
	d = dReadDosTree(dDosDrive);
	dEachNewDir   = (VoidFunc) 0;
	if (dDosDrive[0] == '.') 
		dOutputDir = d;
	else if (dOutputDrive == dDriveNum(dDosDrive) + 1) {
		/* 
		** We just read the drive that CONTAINS dOutputDir,
		** so we have to go look for it.
		*/
		dOutputDir = (Dir) gFind(d, dOutputPath);
		if (!dOutputDir) {
			errorPrintf("Warning:  couldn't find %s on this disk.",
						dOutputPath);
			GetPath(d, dOutputPath);	/* use root */
			dOutputDir = d;
		}
#ifdef DEBUG
		if (!strcmp(gPath(dOutputDir, NIL), dOutputPath)) {
			printf("Pathname screwup: %s vs %s\n", 
					gPath(dOutputDir, NIL), dOutputPath);
			exit(1);
		}
#endif /* DEBUG */
	}
	return(cToDos(""));
}

int cDosW(s)
	char *s;
{
	char *p;

	/* 
	** make this directory the destination for writes 
	*/
 	if (ISNULL(oDosView -> treeview.root)) return (0);
	dOutputDir = (Dir) oDosView -> treeview.cur;
	p = gPath(dOutputDir, NIL);
	if (*p == '.') {
		strcpy(dOutputPath, dWorkingPath);
		if (dOutputPath[strlen(dOutputPath) - 1] == '\\') 
			dOutputPath[strlen(dOutputPath) - 1] = 0;
		if (strlen(p) > 2) {
			if (p[1] != '\\') strcat(dOutputPath, "\\");
			strcat(dOutputPath, p + 1);
		}
	} else {
		strcpy(dOutputPath, p);
	}
	dOutputDrive = dDriveNum(dOutputPath) + 1;
	botLine();
	return (1);
}

int cWriteDosD(s)
	char *s;
{
	TreeView v = (TreeView) theView;

 	if (ISNULL(v -> treeview.root)) return (0);
	if (!dValidateDest() || !dValidateDos(v -> treeview.cur)) 
		return (0);
	dCopyDosDirs(v -> treeview.cur, TRUE);
	gVuUpdAll(oDosView);
	botLine();
	return (cLnDn());
}

int cNewDosD(s)
	char *s;
{
	TreeView v = (TreeView) theView;
	char fn[81];
	extern Dir dNewDir();

 	if (ISNULL(v -> treeview.root)) return (0);
	if (!dValidateDos(v -> treeview.cur)) 
		return (0);

	wmove(MenuWindow, 1, 0);
	waddstr(MenuWindow, "NAME: ");
	wclrtoeol(MenuWindow);
	echo();
	nocrmode();
	wgetstr(MenuWindow, fn);
	if (fn[strlen(fn) -1] <= ' ') fn[strlen(fn) -1] = 0;

	dNewDir(v -> treeview.cur, fn);

	gVuUpdAll(oDosView);
	return (1);
}

/*
** DOS File menu commands
*/
int cDosJ(s)
	char *s;
{
	/* make this file the current Jcamp header file */
	if (ISNULL(oDosFileView -> treeview.root)) return(0);
	GetPath(oDosFileView -> treeview.cur, dJcampBuf);
	dJcampHdr = dJcampBuf;
	botLine();
	return(1);
}

int cWriteDosF(s)
	char *s;
{
	TreeView v = (TreeView) theView;

	if (ISNULL(oDosFileView -> treeview.root)) return(0);
	if (!dValidateDest() || !dValidateDos(v -> treeview.cur)) 
		return (0);
	dCopyDosFile(v -> treeview.cur, TRUE);
	gVuUpdAll(oDosView);
	botLine();
	return (cLnDn());
}


cViewDosF(s)
	char *s;
{
	register Dir   f = (Dir)(oDosFileView -> treeview.cur);
	FileView fv;
	char huge *viewData;

	if (ISNULL(f) || ISNULL(oDosFileView -> treeview.root)) return (0);
	if (! (viewData = dReadDosFile(f))) return (0);

	/* === ought to check for ascii? file being binary === */
	/* === if it is, beep and exit === */

	switch (f -> dir.mode) {
	 case unknown:
	 case ascii:
	 	fv = oTextView;
		MenuJump(VuAscMenu);
		break;
#if 0 					/* can't view DOS spectra yet! */
	 case spectrum:
	 case jcamp:
	 case css:
	 	savescr();
		/* === change FALSE to TRUE if bytes in .sp files
		** === have already been flipped === */
		graphsp(f -> dir.name, viewData, FALSE);
		BufFree(viewData);
		restscr();
	 	return (1);
#endif
	 default:
		fv = oBinaryView;
	 	MenuJump(VuBinMenu);
	}

	theView = (View) fv;
	fv -> fileview.origin = viewData;
	fv -> fileview.limit  = f -> dir.size;
	fv -> fileview.top    = fv -> fileview.cur = 0;
	gVuFirst(theView);
	GetPath(f, theView -> view.name);
	ViewScreen.left = theScreen;
	*ViewScreen.view = theView;
	return (scrOpen(&ViewScreen));
}

cViewFile(s)
	char *s;
{
	/*
	** Called on right arrow from file view:  
	** decide which flavor of file to view.
	*/
	if (theView == (View) oDosFileView) return (cViewDosF(s));
	else						 		return (cViewF(s));
}


/*
** Select Input Drive, Output Drive, Jcamp Header Menu commands
*/

int cSelectI(s)
	char *s;
{
	int d;

	d = dDrive = *s - 'A';
	if (dWorkingDrive == dDrive + 1) dWorkingDrive = 0;
	MenuPop();
	if (NOTNULL(dDrives[d]) && dValidateIdris(dDrives[d])) {
		gVuSet(oDirView, dDrives[d]);
		return(cToDirs(""));
	} else {
		return(cIdrisRead(s));
	}
}

int cSelectO(s)
	char *s;
{
	int d;

	dDosDrive = s;
	d = dDriveNum(s);
	MenuPop();
	if (NOTNULL(dDrives[d]) && *gName(dDrives[d]) == *dDosDrive
	 && dValidateDos(dDrives[d])) {
		gVuSet(oDosView, dDrives[d]);
		return(cToDos(""));
	} else {
		return(cDosRead(s));
	}
}


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
	 case K_HOME:	return(cHomeKey(cp));
	 case K_UP:		return(cUpKey(cp));
	 case K_C_PGUP:
	 case K_PGUP:	return(cPgUpKey(cp));
	 case K_C_END:
	 case K_END:	return(cEndKey(cp));
	 case K_DOWN:	return(cDnKey(cp));
	 case K_C_PGDN:
	 case K_PGDN:	return(cPgDnKey(cp));
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
	 case K_HOME:	return(cHomeKey(cp));
	 case K_UP:		return(cUpKey(cp));
	 case K_C_PGUP:
	 case K_PGUP:	return(cPgUpKey(cp));
	 case K_C_END:
	 case K_END:	return(cEndKey(cp));
	 case K_DOWN:	return(cDnKey(cp));
	 case K_C_PGDN:
	 case K_PGDN:	return(cPgDnKey(cp));
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
	 case K_UP:		return(cUpKey(cp));
	 case K_DOWN:	return(cDnKey(cp));
	 case K_C_PGUP:
	 case K_PGUP:	return(cPgUpKey(cp));
	 case K_C_PGDN:
	 case K_PGDN:	return(cPgDnKey(cp));
	 case K_C_HOME:
	 case K_HOME:	return(cHomeKey(cp));
	 case K_C_END:
	 case K_END:	return(cEndKey(cp));
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
	 case K_UP:		return(cUpKey(cp));
	 case K_DOWN:	return(cDnKey(cp));
	 case K_C_PGUP:
	 case K_PGUP:	return(cPgUpKey(cp));
	 case K_C_PGDN:
	 case K_PGDN:	return(cPgDnKey(cp));
	 case K_C_HOME:
	 case K_HOME:	return(cHomeKey(cp));
	 case K_C_END:
	 case K_END:	return(cEndKey(cp));
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
	CALL('Q', "Quit",
		"Exit the program.",				 				QuitMenu)
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
	CALL('Q', "Quit",
		"Exit the program.",				 				QuitMenu)
	ITEM('/',	"/",
		"/: Return to main menu.",							cVuSlash, 0)
	ITEM('?', "?",		"? Displays help (any time)",		cHelp, ascHelp)
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
	CALL('Q',	"Quit",
		"Exit the program.",				 				QuitMenu)
	ITEM('/',	"/",
		"/: Return to main menu.", 							cVuSlash, 0)
	ITEM('?', "?",		"? Displays help (any time)",		cHelp, binHelp)
	ITEM(0x1b,	"Esc",		
		 "Stop Viewing.  (Return to File window.)",			cVuEsc, 0)
	CTRL(kView)
MenuEND


MenuSTART(SelectIMenu)
	HEAD(1, "Input Drive:",	
		 "Select an input drive for Idris files",
		 0, 0)
	ITEM('0',	"0:(A)",	"Drive 0 (Default)",			cSelectI, "A")
	ITEM('1',	"1:(B)",	"Drive 1",						cSelectI, "B")
	ITEM('2',	"2",		"Drive 2",						cSelectI, "C")
	ITEM('3',	"3",		"Drive 3",						cSelectI, "D")
	ITEM('4',	"4",		"Drive 4",						cSelectI, "E")
	ITEM('5',	"5",		"Drive 5",						cSelectI, "F")
	ITEM('6',	"6",		"Drive 6",						cSelectI, "G")
	ITEM('7',	"7",		"Drive 7",						cSelectI, "H")
	ITEM('8',	"8",		"Drive 8",						cSelectI, "I")
	ITEM('9',	"9",		"Drive 9",						cSelectI, "J")
	ITEM('?', 	"?",		"? Displays help (any time)",	cHelp, selectHelp)
	ITEM('/',	"/",		"/: Return to main menu.",		cToMain, 0)
	ITEM(0x1b,	"Esc",		"Return to Directory view.",	MenuPop, 0)
MenuEND

MenuSTART(ConvertMenu)
	ITEM('A', "Ascii",
		  "Mark files for Ascii conversion",				cCAscii, 0)
	ITEM('B', "Binary",
		  "Mark files for Binary (no) conversion",	 		cCBin, 0)
#ifdef V_SP
	ITEM('J', "JCAMP",
		  "Mark files for JCAMP conversion",		 		cCJcamp, 0)
	ITEM('S', "Spec",
		  "Mark files for Spectrum conversion",			 	cCSpectrum, 0)
#endif
	ITEM('/',	"/",		"/: Return to main menu.",		cToMain, 0)
	ITEM('?', 	"?",		"? Displays help (any time)",	cHelp, convertHelp)
	ITEM(0x1b,	"Esc",		"Return to Directory view.",	MenuPop, 0)
MenuEND

MenuSTART(DirMenu)
	HEAD(1, "Idris:", 	
		 "Idris Directory Window: cursor keys move up and down",	
		 0,		0)
#if 0
	ITEM('D', "DOS(\256)",
		 "Switch to the DOS directory window",				cToDos, 0)
	ITEM('F', "Files(\257)", 
		 "Switch to file window",							cToFiles, 0)
	CALL('C', "Conversion",
		 "Select file conversion type.",					ConvertMenu)
#endif
	ITEM('A', "Ascii",
		  "Mark files for Ascii conversion",				cAscii, 0)
	ITEM('B', "Binary",
		  "Mark files for Binary (no) conversion",	 		cBin, 0)
#ifdef V_SP
	ITEM('C', "CSS",
		  "Mark files for CSS conversion",			 		cCss, 0)
	ITEM('J', "JCAMP",
		  "Mark files for JCAMP conversion",		 		cJcamp, 0)
	ITEM('S', "Spec",
		  "Mark files for Spectrum conversion",			 	cSpectrum, 0)
#endif
	CALL('D', "Drive",	
		 "Select input drive.",								SelectIMenu)
	ITEM('R', "Read",	
		 "Read directory tree from disk.",					cIdrisRead, 0)
	ITEM('T', "Tag",	
		 "Tag everything under highlighted directory",		cTag,	0)
	ITEM('U', "Untag",	
		 "Untag everything under highlighted directory",	cUntag, 0)
#ifndef V_DE
	ITEM('W', "Write",
		 "Write tagged files under highlighted directory",	cWriteD, 0)
#else
	ITEM('W', "Write",
		 "Write tagged files under highlighted directory",	cHelp, demoHelp)
#endif
	CALL('Q', "Quit",   "Exit the program", 				QuitMenu)
	ITEM('/', "/",		"/: Return to main menu.",			cToMain, 0)
	ITEM('?', "?",		"? Displays help (any time)",		cHelp, dirHelp)
	CTRL(kDir)
MenuEND


MenuSTART(FileMenu)
	HEAD(1, "FILES:", 	
		 "File Window:  cursor keys move up and down",
		 0,	0)
	ITEM('A', "Ascii",
		  "Mark file for Ascii conversion",					cAscii, 0)
	ITEM('B', "Binary",
		  "Mark file for Binary (no) conversion",	 		cBin, 0)
#ifdef V_SP
	ITEM('C', "CSS",
		  "Mark files for CSS conversion",			 		cCss, 0)
	ITEM('J', "JCAMP",
		  "Mark file for JCAMP conversion",			 		cJcamp, 0)
	ITEM('S', "Spec",
		  "Mark file for Spectrum conversion",			 	cSpectrum, 0)
#endif
#if 0
	ITEM('D', "Dirs(\256)", 
		 "Switch to directory window",						cToDirs, 0)
#endif
	ITEM('T', "Tag",	
		 "Tag highlighted file",							cTag,	0)
	ITEM('U', "Untag",	
		 "Untag highlighted file",							cUntag, 0)
	ITEM('V', "View(\257)",
		 "View file (in hex if not Ascii type)",			cViewF, 0)
#ifndef V_DE
	ITEM('W', "Write",
		 "Write highlighted file to current directory",		cWriteF, 0)
#else
	ITEM('W', "Write",
		 "Write highlighted file to current directory",		cHelp, demoHelp)
#endif
	CALL('Q', "Quit",   "Exit the program", 				QuitMenu)
	ITEM('/', "/",		"Return to main menu.", 			cToMain, 0)
	ITEM('?', "?",		"? Displays help (any time)",		cHelp, fileHelp)
	CTRL(kFile)
MenuEND


MenuSTART(SelectOMenu)
	HEAD(1, "DOS Drive:",	
		 "Select a drive for DOS files",
		 0, 0)
	ITEM('.',	".",		"Current working directory",	cSelectO, ".")
	ITEM('A',	"A:(0)",	"Drive A",						cSelectO, "A:")
	ITEM('B',	"B:(1)",	"Drive B",						cSelectO, "B:")
	ITEM('C',	"C:",		"Drive C",						cSelectO, "C:")
	ITEM('D',	"D:",		"Drive D",						cSelectO, "D:")
	ITEM('E',	"E:",		"Drive E",						cSelectO, "E:")
	ITEM('F',	"F:",		"Drive F",						cSelectO, "F:")
	ITEM('G',	"G:",		"Drive G",						cSelectO, "G:")
	ITEM('H',	"H:",		"Drive H",						cSelectO, "H:")
	ITEM('I',	"I:",		"Drive I",						cSelectO, "I:")
	ITEM('J',	"J:",		"Drive J",						cSelectO, "J:")
	ITEM('?', 	"?",		"? Displays help (any time)",	cHelp, selectHelp)
	ITEM('/',	"/",		"Return to main menu.",			cToMain, 0)
	ITEM(0x1b,	"Esc",		"Return to Directory view.",	MenuPop, 0)
MenuEND


MenuSTART(DosMenu)
	HEAD(1, "DOS:", 	
		 "DOS Directory Window: cursor keys move up and down",	
		 0,		0)
#if 0
	ITEM('I', "Idris(\256)",
		 "Switch to the Idris directory view",				cToDirs, 0)
	ITEM('F', "Files(\257)", 
		 "Switch to File view (to the right)",				cToDosFiles, 0)
#endif
	CALL('D', "Drive",	"Select DOS drive.",				SelectOMenu)
	ITEM('N', "New", 	"create New directory",				cNewDosD, 0)
	ITEM('O', "Output",	
		 "Select output (working) directory",				cDosW, 0)
	ITEM('R', "Read",	"Read directory tree from disk.",	cDosRead, 0)
	ITEM('T', "Tag",	
		 "Tag everything under highlighted directory",		cTag,	0)
	ITEM('U', "Untag",	
		 "Untag everything under highlighted directory",	cUntag, 0)
#ifndef V_DE
	ITEM('W', "Write",
		 "Write tagged files under highlighted directory",	cWriteDosD, 0)
#else
	ITEM('W', "Write",
		 "Write tagged files under highlighted directory",	cHelp, demoHelp)
#endif
	CALL('Q', "Quit",   "Exit the program", 				QuitMenu)
	ITEM('/', "/",		"Return to main menu.", 			cToMain, 0)
	ITEM('?', "?",		"? Displays help (any time)",		cHelp, dosHelp)
	CTRL(kDir)
MenuEND


MenuSTART(DosFileMenu)
	HEAD(1, "FILES:", 	
		 "DOS File Window: cursor keys move up and down",
		 0,	0)
	ITEM('A', "Ascii",
		  "Mark file for Ascii viewing",					cAscii, 0)
	ITEM('B', "Binary",
		  "Mark file for Binary viewing",			 		cBin, 0)
#if 0
	ITEM('D', "Dirs(\256)", 
		 "Switch to Directory view (to the left)",			cToDirs, 0)
#endif
#ifdef V_SP
	ITEM('J', "Jcamp",	
		 "Select Jcamp header file",						cDosJ, 0)
#endif
	ITEM('T', "Tag",	
		 "Tag highlighted file",							cTag,	0)
	ITEM('U', "Untag",	
		 "Untag highlighted file",							cUntag, 0)
	ITEM('V', "View(\257)",
		 "View file (in hex if not Ascii type)",			cViewDosF, 0)
#ifndef V_DE
	ITEM('W', "Write",
		 "Write highlighted file to current directory",		cWriteDosF, 0)
#else
	ITEM('W', "Write",
		 "Write highlighted file to current directory",		cHelp, demoHelp)
#endif
	CALL('Q', "Quit",   "Exit the program", 				QuitMenu)
	ITEM('/', "/",		"Return to main menu.", 			cToMain, 0)
	ITEM('?', "?",		"? Displays help (any time)",		cHelp, dosFileHelp)
	CTRL(kFile)
MenuEND


MenuSTART(MainMenu)
	HEAD(1, "MAIN:",	
		 "Main menu:  space & backspace to move; Enter or letter to select",
		 0, 0)
	ITEM('A', "About",
		 "About this program (an overview)",				cHelp, ovHelp)
	ITEM('D', "DOS(\256)",
		 "To DOS directory window",							cLeft, 0)
	ITEM('I', "Idris(\257)",
		 "To Idris directory window",						cRight, 0)
	CALL('Q', "Quit",   "Exit the program", 				QuitMenu)
	ITEM('?', "?",		"? Displays help (any time)",		cHelp, mainHelp)
	CTRL(kMain)
MenuEND


/*********************************************************************\
**
** Hardware error trap handler
**
\*********************************************************************/

void hardwareErrorHandler(deverror, errcode, devhdr)
	unsigned deverror, errcode, far *devhdr;
{
	char *es = "Hardware Error";
	/* 
	** Presumably, MS-DOS will tell the application why this failed
	** by returning an appropriate error code.
	*/
	errcode &= 0xF;
	if (errcode == 0) es = "Disk is write protected";
	else if (errcode == 1) es = "Unknown unit";
	else if (errcode == 2) es = "Drive not ready";
	else if (errcode == 3) es = "Unknown command to disk";
	else if (errcode == 4) es = "CRC Error";
	else if (errcode == 5) es = "Bad drive-request structure length";
	else if (errcode == 6) es = "Seek error";
	else if (errcode == 7) es = "Sector not found";
	else if (errcode == 8) es = "Unknown media type";
	else if (errcode == 10) es = "Printer out of paper";
	else if (errcode == 11) es = "Write fault";
	else if (errcode == 12) es = "Read fault";
	else if (errcode == 13) es = "General failure";
	errorSet(es);
	_hardresume(_HARDERR_FAIL);
}


/*********************************************************************\
**
** Command-line and Environment Option Processing
**
\*********************************************************************/

static Bool doOption (opt, str)
	char opt;
	char *str;
{
	
	switch (opt) {
	 case 'i': case 'I':
	 	dDrive = atoi(str);
		break;

	 case 'o': case 'O':
	 	/*
		** What we really have to do is to cd to the specified dir.
		*/
		strcpy(dWorkingPath, str);
		if (str[1] == ':' && str[2] == 0) strcat(dWorkingPath, "\\");
		break;

	 case 'j': case 'J':
		strcpy(dJcampBuf, str);
		if (str[1] == ':' && str[2] == 0) strcat(dJcampBuf, "\\");
		dJcampHdr = dJcampBuf;
		break;

	 default:
	 	return (FALSE);
	}
	return (TRUE);

}

/*********************************************************************\
**
**	M A I N   P R O G R A M
**	
\*********************************************************************/

main(argc, argv)
int    argc;
char **argv;
{
	int c;
	unsigned d;
	char opts[256], *opt;
	int origDrive;
	char origPath[256];
	int x, y, scrx, scry;
	short *oldscr;

	/*
	** Set up defaults
	*/
	dDrive    = 0;
	dDosDrive = ".";
	dJcampHdr = (String) NULL;

	getcwd(dWorkingPath, sizeof(dWorkingPath));
	if (dWorkingPath[strlen(dWorkingPath) - 1] == '\\')
		dWorkingPath[strlen(dWorkingPath) - 1] = 0;
	_dos_getdrive(&dWorkingDrive);
	strcpy(origPath, dWorkingPath);
	origDrive = dWorkingDrive;

	_harderr(hardwareErrorHandler);

	/*
	** Process environment variable
	*/
	if (opt = getenv("RD7")) strcpy(opts, opt);
	else					 opts[0] = 0;

	for (opt = strtok(opts, " \t"); opt; opt = strtok(NULL, " \t")) {
		if (*opt == '-') {
			c = opt[1];
			if (!(opt = strtok(NULL, " \t")) || !doOption(c, opt)) {
			 	fprintf(stderr, 
					    "RD7: unknown environment option '-%c %s'\n", c, opt);
				exit(1);
			}
		} else {
			fprintf(stderr, "RD7: unknown environment option '%s'\n", opt);
			exit(1);
		}
	}

	/*
	** Process command line arguments
	*/
	for ( ; ++argv, --argc; ) {
		if (**argv == '-') {
			c = (*argv)[1];
			if (!(++argv, --argc) || !doOption(c, *argv)) {
			 	fprintf(stderr, "RD7: unknown option '-%c %s'\n", c, *argv);
				exit(1);
			}
		} else {
			fprintf(stderr, "RD7: unknown option '%s'\n", *argv);
			exit(1);
		}
	}

	/*
	** Initialize Screen
	*/
	initscr();
	scrx = stdscr -> maxx;
	scry = stdscr -> maxy;
	oldscr = (short *) malloc(scrx * scry * sizeof(short));
	for (y = 0; y < stdscr -> maxy; ++y)
		for (x = 0; x < stdscr -> maxx; ++x)
			oldscr[y * scrx	+ x] = wincha(stdscr, y, x);

	clear();
	initHelp();
	vInit(initMsg);
	scrOpen(&MainScreen);

	/*
	** Before we show the bottom line, try to CD to the working dir.
	*/
	if (strcmp(dWorkingPath, origPath)) {
		if (dWorkingPath[1] == ':') {
			dWorkingDrive = dWorkingPath[0] & 0x1F;
			_dos_setdrive(dWorkingDrive, &d);
			_dos_getdrive(&d);
			if (d != dWorkingDrive) goto fail;
		}
		if (chdir(dWorkingPath)) {
			/*
			** Unsuccessful -- put it back
			*/
		fail:
			_dos_setdrive(origDrive, &d);
			chdir(origPath);
			dWorkingDrive = origDrive;
			errorPrintf("Warning: unable to open %s; back to %s", 
						dWorkingPath, origPath);
			strcpy(dWorkingPath, origPath);
			vInit(initMsg);
			scrOpen(&MainScreen);
		} else {
			/*
			** Successful -- get DOS's idea of the path.
			*/
			getcwd(dWorkingPath, sizeof(dWorkingPath));
			if (dWorkingPath[strlen(dWorkingPath) - 1] == '\\')
				dWorkingPath[strlen(dWorkingPath) - 1] = 0;
		}
	}
	strcpy(dOutputPath, dWorkingPath);
	dOutputDrive = dWorkingDrive;
	botLine();
	refresh();

#ifdef V_DE
	/* 
	** Demo version -- check validity
	*/

#endif
	/*
	** Enter Interactive Mode
	*/
	MenuStart(MainMenu, &NormalMenuDscr);
	endwin();

	/*
	** Put things back the way we found them
	*/
	_dos_setdrive(origDrive, &d);
	chdir(origPath);
	for (y = 0; y < stdscr -> maxy; ++y) {
		wmove(stdscr, y, 0);
		for (x = 0; x < stdscr -> maxx; ++x) {
			waddcha(stdscr, oldscr[y * scrx	+ x]);
		}
	}
	exit(0);
}

/*********************************************************************\
**
** Stubs and Interface Glue for Spectra
**
**		This section contains two kinds of routines:
**		Stubs to eliminate the need for SP.LIB in RD7AB and RD7DE
**		Glue routines for RD7SP
**
\*********************************************************************/


#ifdef V_SP

/*
** GLUE
*/
global int rd7sp = TRUE;

global void errstrg(s)
	char *s;
{
	errorPrintf(s);
}

#else

/*
** STUBS
*/
global int rd7sp = FALSE;

global void jcampdo()
{
	/* stub */
}

global void cuvdo()
{
	/* stub */
}

global void dmdo()
{
	/* stub */
}

#endif



