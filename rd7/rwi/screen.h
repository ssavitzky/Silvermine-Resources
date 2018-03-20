/*** headerID = "screen.h 1.0 (c) 1989 S. Savitzky ***/

/*********************************************************************\
**
**	Screen -- screen maintenance and generic commands.
**
**	890716	SS	create
**
\*********************************************************************/

/*********************************************************************\
**
** Screen data structure
**
\*********************************************************************/

typedef struct screen *Screen;

typedef struct screen {
	char	   *label;
	View	    view;
	Menu		menu;
	Screen		partner;
	Screen		left;
	Screen		right;
	VoidFunc 	stat;			/* (scr) called on refresh */
} ScreenRec;

/*********************************************************************\
**
**	V A R I A B L E S
**	
\*********************************************************************/

global String errorMsg;				/* set by errorSet					*/

global View theView;			   	/* currently-active view			*/
global View helpedView;				/* theView before invoking Help		*/
global Screen theScreen;		   	/* currently-active screen			*/

/*********************************************************************\
**
** Error-handling operations
**
\*********************************************************************/

global int  errorView();		/* (str) 	display error message string */
global int  errorPrintf();		/* (fmt...) format and display msg */
global void errorClear();		/* ()       clear message */
global void errorSet();			/* (msg)	set msg to display later */
global int  errorCheck();		/* rtn 1 & display msg if present */


/*********************************************************************\
**
** Screen operations
**
\*********************************************************************/

global int getCmd();			/* get a keyboard command character */
global void savescr();		   	/* save screen across graphics */
global void restscr();		   	/* restore saved screen */

global View dViewToUpdate;		/* view to update on read/write */
global void dReadStart();		/* (dir)  initial status for read ops */
global void dWriteStart();		/* (dir)  initial status for write ops */

global void scrSet();			/* (scr, tree)  set tree to display */
global void scrUpdLine();		/* (scr)  update cur. line */
global int  scrOpen();			/* (scr)  display/update a screen */
global void scrOnUpdate();		/* () update status & detail of theScreen */

/*********************************************************************\
**
** Generic action routines for use in menus
**
\*********************************************************************/

int cLnUp();
int cLnDn();
int cPgUp();
int cPgDn();
int cLeft();
int cRight();
int cRightToFiles();			/* beeps if no files present */
int cHome();
int cEnd();

int tag();						/* (mode) generic tagger. */
int cAscii();
int cCAscii();					/* does MenuPop */
int cBin();
int cCBin();					/* does MenuPop */
int cTag();
int cUntag();

