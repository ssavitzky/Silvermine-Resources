/***/ static char *pgmid = "rd3 1.0 copyright 1990 S. Savitzky"; /***/

/*********************************************************************\
**
**	rd3 -- selectively copy files between PETOS and PC format
**
**	900120 SS	create from RWi
**	890619 SS	create PC version from RD7
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
#include "../lib/menu.h"

#include "version.h"
#include "coops.h"
#include "tree.h"
#include "dir.h"
#include "viewer.h"
#include "filevr.h"
#include "view.h"
#include "disk.h"
#include "petos.h"
#include "convert.h"

#include "screen.h"
#include "data.h"

#undef  global
#define global

extern ViewRec rDstFileView;
static View dstFileView = &rDstFileView;
extern ViewRec rDstDirView;
static View dstDirView = &rDstDirView;

extern ClassRec crDir, crXDir;

extern char *initMsg[];
extern void initHelp();

#define CCINFO 0
#define LDINFO 1
#define MDINFO 3

extern  ExtListRec dXExts[];
#define defaultConv	(dXExts[0].ftype)

global void errstrg(s)
	char *s;
{
	errorPrintf(s);
}

/*********************************************************************\
**
** Product-specific modules:
**
**		main.c			main program; menu command handlers
**		data.c			data-structure (object) definitions
**		help.c			help screens
**		pecos.c			iRMX86 operating system utilities
**		convert.c		file format conversions
**
**	Generic modules:
**
**		screen.c		screen handling and generic menu commands
**		dirs.c			directory and file tree structures
**		view.c			screen handling for directory and contents windows
**		disk.c			Raw disk handling
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

global int debugf = 0;

/*********************************************************************\
**
** Viewing Utilities
**
**		These should become messages RSN
**
\*********************************************************************/

void setRoot(v, x)
	View v;
	Opaque x;
{
	gVrSet(v -> view.root, x);
	gVuSet(v, v -> view.root);
}

/*
** The following assume that v is a file view.
** They really ought to be messages.
*/
Dir curRoot(v)
	View v;
{
	return ((Dir) gVrGet(v -> view.root));
}

Dir curFile(v)
	View v;
{
	register TreeVr vr = (TreeVr) v -> view.cur;

	if (!vr) return ((Dir) NIL);
	return ((Dir) vr -> treeVr.cur);
}

/*********************************************************************\
**
** char *getUserString(prompt) -- get a typed-in string from the user
**
\*********************************************************************/

char *getUserString(prompt)
	char *prompt;
{
	static char s[81];

	wmove(MenuWindow, 1, 0);
	waddstr(MenuWindow, prompt);
	wclrtoeol(MenuWindow);
	echo();
	nocrmode();
	wgetstr(MenuWindow, s);
	for ( ; strlen(s) && s[strlen(s) -1] <= ' '; ) s[strlen(s) -1] = 0;
	return (s);
}

/*********************************************************************\
**
** Command Procedures accessed from Menus
**		All take an optional char* parameter.
**
\*********************************************************************/

/* 
** Any Menu Commands 
*/

int cHelp(s)
	char *s;
{
	MenuCall(HelpMenu);				/* only do this to save previous one */
	HelpScreen.left = theScreen;
	setRoot(oHelpView, s);
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
	setRoot(oInitView, initMsg);
	return(scrOpen(&MainScreen));
}

/*********************************************************************\
**
** Menu-Specific Commands
**
\*********************************************************************/

/*
** Viewing
*/

static Dir viewedFile;
extern void bufFlush();

#ifdef V_SP
extern void cvViewSpectrum();
#endif

cViewFile(s)
	char *s;
{
	register Dir f = curFile(theView);
	TextVr tv = (TextVr) oTextView -> view.root;

	if (ISNULL(f) || ISDIR(f)) return (0);

	if (fClass(f) == clXDir) {
		fileVrEOL = '\r';
	} else {
		fileVrEOL = '\n';
	}

	dReadMessage();
	viewedFile = f;
	switch (f -> dir.ftype) {		/* === should really use table === */
	 case unknown:
	 	break;
	 case ascii:
	 	tv -> textVr.hex = FALSE;
		break;
#ifdef V_SP
	 case css:
	 case jcamp:
	 case spectrum:
	 	savescr();
		cvViewSpectrum(f);
		restscr();
		scrOpen(theScreen);
		return (TRUE);
#endif
	 case binary:
	 default:
	 	tv -> textVr.hex = TRUE;
	}
	if (tv -> textVr.hex) 
	 	MenuJump(VuBinMenu);
	else
		MenuJump(VuAscMenu);

	gVrSet(oTextView -> view.root, f);
	gVuSet(oTextView, oTextView -> view.root);

	theView = (View) oTextView;
	GetPath(f, theView -> view.name);
	ViewScreen.left = theScreen;
	return (scrOpen(&ViewScreen));
}

int cVuEsc(s)
	char *s;
{
	bufFlush(viewedFile);
	gCloseFile(viewedFile);
	return(cLeft(s));
}

int cVuSlash(s)
	char *s;
{
	bufFlush(viewedFile);
	gCloseFile(viewedFile);
	return (cToMain(s));
}

int cVuAsc(s)
	char *s;
{
	register long i;
	TextVr vr = (TextVr) theView -> view.top;

	vr -> textVr.hex = FALSE;		/* set ascii mode */
	for (i = vr -> textVr.cur;		/* scan back to BOL */
		 i && vr -> textVr.buf[i-1] != '\n';
		 --i 
		) ;
	vr -> textVr.cur = i;
	gVrCopy(theView -> view.cur, vr);
	theView -> view.y = 0;
	theView -> view.do_update = TRUE;
	scrOpen(theScreen);
	return (MenuJump(VuAscMenu));
}

int cVuBin(s)
	char *s;
{
	TextVr vr = (TextVr) theView -> view.top;

	vr -> textVr.hex = TRUE;		/* set ascii mode */
	vr -> textVr.cur &= ~0xfL;
	gVrCopy(theView -> view.cur, vr);
	theView -> view.y = 0;
	theView -> view.do_update = TRUE;
	scrOpen(theScreen);
	return (MenuJump(VuBinMenu));
}

/*
** Destination file menu commands
*/
#ifdef V_SP

int cHeader(s)
	char *s;
{
	Dir f = curFile(theView);

	GetPath(f, dJcampBuf);
	dJcampHdr = dJcampBuf;
	botLine();
	return(1);
	
}

int setConv(s)
	char *s;
{
	switch (*s)	{
	 case 'A': case 'a':	defaultConv = ascii; 	break;
	 case 'B': case 'b':	defaultConv = binary;	break;
	 case 'C': case 'c':	defaultConv = css;		break;
	 case 'J': case 'j':	defaultConv = jcamp;	break;
	 case 'S': case 's':	defaultConv = spectrum;	break;
	 default:	return (0);
	}
	return (1);
}

int cConv(s)
	char *s;
{
	MenuPop();
	return (setConv(s));
}
#endif

/*
** General Directory menu commands 
*/

/*
** This routine checks the drive we just read to see if it's the one we're
** supposedly writing to.  If so, it looks for the output path on it,
** and returns 1 so we know to select it in the view.
*/
static int checkDstDrive(d, drv)
	Dir d;
	int drv;
{
	char *s;

	if (ISNULL(d)) return (0);
	if (dDriveNum(dOutputPath) == drv) {
		/* 
		** We just read the drive that CONTAINS dOutputPath,
		** so we have to go look for it.
		*/
		s = gName(d);
		if (! strcmp(dOutputPath, s))
			dOutputDir = d;
		else if (! strncmp(dOutputPath, s, strlen(s)))
			dOutputDir = (Dir) gFind(d, dOutputPath + strlen(s));
		else
			dOutputDir = (Dir) gFind(d, strchr(dOutputPath, ':') + 1);
		if (!dOutputDir) {
			errorPrintf(
	"Warning:  couldn't find Output directory '%s' on this disk.  Writing to root.",
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
		return (1);
	}
	return (0);
}

int cReadTree(s)
	char *s;
{
	Dir f = curRoot(theView);
	int d, dst, i;

	if (NOTNULL(f)) {
		/*
		** There's an old tree: delete it.
		** (even if on same drive, might have swapped disks.)
		*/
		--inUse[dDriveNum(gName(f))];
		/* === may have to zap the old root === */
	}
	dst = (theScreen == &DstDirScreen);
	if (dst) {
		d = dOutputDrive - 1;
	} else {
		d = dInputDrive - 1;
	}
	dEachNewDir   = dReadStart;
	dViewToUpdate = theView;
	dReadMessage();

	cfReadTree(dFileSys[d])(dFileSys[d], d);
	dEachNewDir = (VoidFunc) 0;
	if (dInputDrive == dOutputDrive) {
		/* set up opposite screen if showing the same drive */
		/* === trouble with SrcAllScreen === */
		if (dst) scrSet(&SrcDirScreen, dDrives[d]);
		else	 scrSet(&DstDirScreen, dDrives[d]);
		if (dst) scrSet(&SrcFileScreen, dDrives[d]);
		else	 scrSet(&DstFileScreen, dDrives[d]);
		++inUse[d];
	}
	scrSet(theScreen, dDrives[d]);
	scrSet(theScreen -> partner, dDrives[d]);
	if (checkDstDrive(curRoot(theView), d)) {
		/*
		 * Make the current output directory the current line
		 */
		gVrSet(theView -> view.cur, dOutputDir);
		gVrSet(theView -> view.top, dOutputDir);
		for (i = 0; 
			 i < theView -> view.rows / 2 && gVrPrev(theView -> view.top); 
			 ++i) ;
		theView -> view.y = i;
	}
	botLine();
	return(scrOpen(theScreen));
}


int cWrite(s)
	char *s;
{
	Dir f = curFile(theView);

 	if (ISNULL(f)) return (0);
	if (!(NOTNULL(dOutputDir) && gValidate(dOutputDir) || dValidateDest()) 
	  || !gValidate(f)) 
		return (0);
	if (ISDIR(f))
		dCopyDirs(f, TRUE);
	else
		dCopyFile(f, TRUE);
	botLine();
	dstDirView -> view.do_update = dstDirView -> view.do_refresh = TRUE;
	if (NOTNULL(dOutputDir)) setRoot(dstFileView, dOutputDir);
	dstFileView -> view.do_update = dstFileView -> view.do_refresh = TRUE;
	scrOpen(theScreen);
	return (cLnDn());
}

/*
** Destination Directory Menu Commands
*/

int cDosW(s)
	char *s;
{
	char *p;

	/* 
	** make this directory the destination for writes 
	*/
 	if (ISNULL(curRoot(theView))) return (0);
	dOutputDir = curFile(theView);
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
	scrOpen(theScreen);
	return (1);
}


int cNewDosD(s)
	char *s;
{
	char fn[81];
	Dir f = curRoot(theView);

 	if (ISNULL(f)) return (0);
	if (!gValidate(f)) return (0);

	wmove(MenuWindow, 1, 0);
	waddstr(MenuWindow, "NAME: ");
	wclrtoeol(MenuWindow);
	echo();
	nocrmode();
	wgetstr(MenuWindow, fn);
	for ( ; strlen(fn) && fn[strlen(fn) -1] <= ' '; ) fn[strlen(fn) -1] = 0;
	if (strlen(fn) == 0) return (0);

	gCreateFile(curFile(theView), fn, TRUE);

	dstDirView -> view.do_update = TRUE;
	dstFileView -> view.do_update = TRUE;
	scrOpen(theScreen);
	return (1);
}

/*
** Select Drive	and File System
*/

void setSrcView(d)		/* select appropriate view for FROM drive */
	int d;
{
	/*
	 * If d is a PETOS drive, use full-width view,
	 * else use split dirs/files view.
	 */
	if (dFileSys[d] == &crXDir) {
		DstDirScreen.left = DriveScreen.left = MainScreen.left =
			&SrcAllScreen;
	} else {
		DstDirScreen.left = DriveScreen.left = MainScreen.left =
			&SrcDirScreen;
	}
}

void setDrive(d)		/* set drive for current view */
	int d;
{
	int dst;			/* true if setting destination drive */

	/* Source or destination? */

	dst = (theScreen == &DstDirScreen);
	if (dst) {
		if (dOutputDrive && inUse[dOutputDrive - 1]) 
			--inUse[dOutputDrive - 1];
		dOutputDrive = d + 1; 
	} else {    
		if (dInputDrive && inUse[dInputDrive - 1]) 
			--inUse[dInputDrive - 1];
		dInputDrive = d + 1;
	}
	++inUse[d];

	/* Validate and read disk. */

	botLine();
	if (NOTNULL(dDrives[d])) {
		scrSet(theScreen, dDrives[d]);
		scrSet(theScreen -> partner, dDrives[d]);
		botLine();
		scrOpen(theScreen);
	} else {
		cReadTree((char *)NULL);
	}
}

int cDrives(s)
	char *s;
{
	int d;
	int dst;

	dst = (theScreen == &DstDirScreen);
	if (dst) d = dOutputDrive - 1; 
	else     d = dInputDrive - 1;

	((DriveVr)oDriveView -> view.cur) -> driveVr.cur = d;
	oDriveView -> view.y = d;  /* hilite current drive */

	scrOpen(&DriveScreen);
	return (1);
}

int cSelectDr(s)
	char *s;
{
	int d;

	d = dDriveNum(s);
	MenuPop();

	setDrive(d);
	return (1);
}

int cSelectFS(s)
	char *s;
{
	int d = ((DriveVr)theView -> view.cur) -> driveVr.cur;

	dFileSys[d] = (Class)s;
	if (dFileSys[d] == &crXDir) {
	 	dDriveType[d] &= ~DRV_HARD;
	 	dDriveType[d] |= DRV_COMPAT; 
		dMediaType[d] = 16;
	}
	if (d == dInputDrive - 1) setSrcView(d);
	scrOpen(theScreen);
	return (MenuPop());
}

/*
** Selecting Source and Destination drives in the Drive view
*/
int cSrcDrive(s)
	char *s;
{
	int d = ((DriveVr)theView -> view.cur) -> driveVr.cur;

	setSrcView(d);			/* select appropriate view */
	cLeft();				/* switch to it */
	setDrive(d);			/* set drive in it */
	return (1);
}

int cDstDrive(s)
	char *s;
{
	int d = ((DriveVr)theView -> view.cur) -> driveVr.cur;

	cRight();				/* select appropriate view */
	setDrive(d);			/* set drive in it */
	return (1);
}

/*
** Disk parameter setting in the Drive view.
*/
int cDriveType(s)
	char *s;
{
	int d = ((DriveVr)theView -> view.cur) -> driveVr.cur;

	switch (*s) {
	 case 'h': case 'H':	dDriveType[d] = DRV_HARD;	break;
	 case 'p': case 'P':	dDriveType[d] = DRV_PC;		break;
	 case 'a': case 'A':	dDriveType[d] = DRV_AT;		break;
	 case 'c': case 'C':
	 	dDriveType[d] &= ~DRV_HARD;
	 	dDriveType[d] |= DRV_COMPAT; 
		dMediaType[d] = 16;
		break;
	}
	scrOpen(theScreen);
	return (MenuPop());
}

int cMediaType(s)
	char *s;
{
	int d = ((DriveVr)theView -> view.cur) -> driveVr.cur;
	extern int dMediaType[];

	dMediaType[d] = atoi(s);
	scrOpen(theScreen);
	return (MenuPop());
}

int cPhysDrive(s)
	char *s;
{
	int d = ((DriveVr)theView -> view.cur) -> driveVr.cur;
	char buf[81];
	int i;
	extern int dPhysDrive[];

	wmove(MenuWindow, 1, 0);
	waddstr(MenuWindow, "PHYSICAL DRIVE NUMBER: ");
	wclrtoeol(MenuWindow);
	echo();
	nocrmode();
	wgetstr(MenuWindow, buf);

	for (i = strlen(buf); buf[--i] <= ' '; ) buf[i] = 0;
	if (strlen(buf) == 0) return (0);
	dPhysDrive[d] = atoi(buf);

	scrOpen(theScreen);
	return (1);
}

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
	int opt;
	char *str;
{
	int d;
	int i;

	switch (opt) {
	 case 'i': case 'I':
	    d = (str[0] > 'Z') ? str[0] - 'a' : str[0] - 'A';
	 	dInputDrive = 1 + d;
		break;

	 case 'o': case 'O':
	 	/*
		** What we really have to do is to cd to the specified dir.
		*/
		strcpy(dWorkingPath, str);
		if (str[1] == ':' && str[2] == 0) strcat(dWorkingPath, "\\");
		break;

	 case 'c': case 'C':
#ifdef V_SP
		setConv(str);
#endif
		break;

	 case 'd': case 'D':
	 	/*
		** -d <drive-letter>[HAP][C]=<phys-drive#>
		**
		**		H = Hard / A = AT-type / P = PC-type
		**		C = CompatiCard driver
		*/
	    d = (str[0] > 'Z') ? str[0] - 'a' : str[0] - 'A';
		for (i = 1; str[i]; ++i) {
			switch (str[i]) {
			 case 'h': case 'H':	dDriveType[d] = DRV_HARD;	break;
			 case 'p': case 'P':	dDriveType[d] = DRV_PC;		break;
			 case 'a': case 'A':	dDriveType[d] = DRV_AT;		break;
			 case 'c': case 'C':	
			 	dDriveType[d] |= DRV_COMPAT; 
				dDriveType[d] &= ~DRV_HARD;
				dMediaType[d] = 16;
				break;
			 case '=':
			 	dPhysDrive[d] = atoi(&str[i+1]);
				goto done;
			 default:
			 	return (FALSE);
			}
		}
	done:
		break;

	 case 'h': case 'H':
#ifdef V_SP
		strcpy(dJcampBuf, str);
		if (str[1] == ':' && str[2] == 0) strcat(dJcampBuf, "\\");
		dJcampHdr = dJcampBuf;
#endif
		break;

	 default:
	 	return (FALSE);
	}
	return (TRUE);

}

/*********************************************************************\
**
** Clean up and exit
**
\*********************************************************************/

static int origDrive;
static char origPath[256];
static short *oldscr;
static int scrx, scry;

global int cExit(n)
	int n;
{
	int c, i;
	unsigned d;
	int x, y;

	/*
	** Put things back the way we found them
	*/
	endwin();
	dCloseDisk();
	_dos_setdrive(origDrive, &d);
	chdir(origPath);
	for (y = 0; y < stdscr -> maxy; ++y) {
		wmove(stdscr, y, 0);
		for (x = 0; x < stdscr -> maxx; ++x) {
			waddcha(stdscr, oldscr[y * scrx	+ x]);
		}
	}
	exit(n);
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
	int c, i;
	unsigned d;
	char opts[256], *opt, *arg;
	int x, y;
	extern MenuDescriptor NormalMenuDscr;
	typedef char (*CharFn)();

	/*
	** Set up defaults
	*/
	dClassifyDrives(); 
	for (dInputDrive = dMaxDrives; dInputDrive; --dInputDrive)
		if (dDriveType[dInputDrive - 1] & DRV_COMPAT) {
			break;
		}
	getcwd(dWorkingPath, sizeof(dWorkingPath));
	if (dWorkingPath[strlen(dWorkingPath) - 1] == '\\')
		dWorkingPath[strlen(dWorkingPath) - 1] = 0;
	_dos_getdrive(&dWorkingDrive);
	_dos_setdrive(dWorkingDrive, &dMaxDrives);
	strcpy(origPath, dWorkingPath);
	origDrive = dWorkingDrive;
	_harderr(hardwareErrorHandler);

#ifdef V_SP
	setConv("S");
#endif
	/*
	** Process environment variable
	*/
	if (opt = getenv(ENV_VAR)) strcpy(opts, opt);
	else					   strcpy(opts, "");

	for (opt = strtok(opts, " \t"); opt; opt = strtok(NULL, " \t")) {
		if (*opt == '-') {
			c = opt[1];
			if (opt[2]) arg = opt + 2;
			else if (opt = strtok(NULL, " \t")) arg = opt;
			else arg = 0;
			if (!doOption(c, arg)) {
			 	fprintf(stderr, 
					    "%s: unknown environment option '-%c %s'\n", 
						PROGRAM, c, opt);
				exit(1);
			}
		} else {
			fprintf(stderr, "%s: unknown environment option '%s'\n", 
					PROGRAM, opt);
			exit(1);
		}
	}

	/*
	** Process command line arguments
	*/
	for ( ; ++argv, --argc; ) {
		if (**argv == '-') {
			c = (*argv)[1];
			if ((*argv)[2])  arg = *argv + 2;
			else if (--argc) arg = *++argv;
			else 			 arg = 0;
			if (! doOption(c, arg)) {
			 	fprintf(stderr, "%s: unknown option '-%c %s'\n", 
						PROGRAM, c, *argv);
				exit(1);
			}
		} else {
			fprintf(stderr, "%s: unknown option '%s'\n", 
					PROGRAM, *argv);
			exit(1);
		}
	}

	/*
	** Initialize Screen
	*/
	initscr();
	MenuInput = (CharFn) getCmd;
	scrx = stdscr -> maxx;
	scry = stdscr -> maxy;
	oldscr = (short *) malloc(scrx * scry * sizeof(short));
	for (y = 0; y < stdscr -> maxy; ++y)
		for (x = 0; x < stdscr -> maxx; ++x)
			oldscr[y * scrx	+ x] = wincha(stdscr, y, x);

	clear();
	initHelp();
	viewInit();
	((DriveVr)oDriveView -> view.root) -> driveVr.lim = dMaxDrives;
	((DriveVr)oDriveView -> view.cur) -> driveVr.lim = dMaxDrives;
	dataInit(initMsg);

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
			dataInit(initMsg);
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
	for (i = 0; i < MAXDRIVES; ++i) dFileSys[i] = &crDir;
	if (!dInputDrive) {
		dInputDrive  = 1;
	} else {
		dFileSys[dInputDrive - 1] = &crXDir;
		dMediaType[dInputDrive - 1] = 16;
	}
	setSrcView(dInputDrive - 1);
	((DriveVr)oDriveView -> view.cur) -> driveVr.cur = dOutputDrive - 1;
	oDriveView -> view.y = dOutputDrive - 1;  /* hilite current drive */
	scrOpen(&MainScreen);
	botLine();
	refresh();

	/*
	** Enter Interactive Mode
	*/
	MenuStart(MainMenu, &NormalMenuDscr);
	cExit(0);
}

