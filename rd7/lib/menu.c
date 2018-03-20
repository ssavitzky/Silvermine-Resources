/***/ static char *moduleid = "MENU.C 1.0 copyright 1987 S. Savitzky";

/*********************************************************************\
**
**	MENU --	General-purpose menu interpretor
**
**	870727 SS	create PC version.
**
\*********************************************************************/

#include <stdio.h>
#include "error.h"
#include "curse.h"
#include "menu.h"

extern char getch();

#define global
#define local	static
#define DEBUG	if (debugf) {
#define DEBEND }

#define TRUE 1
#define FALSE 0
#define MAXDEPTH 20 /* max menu stack depth */

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned long ulong;

/*********************************************************************\
**
**	V A R I A B L E S
**	
\*********************************************************************/

char (*MenuInput)() = getch;	/* where to get characters */
WINDOW *MenuWindow;				/* where to put them */
int MenuIsDisplayed = FALSE;	/* TRUE if some menu is on the screen */

MenuDescriptor MenuDefaultDscr = {
	FALSE,		/* keyword select */
	TRUE,		/* cursor select */
	2, 0,		/* row, column */
	80,	2,		/* width, height */
	1,			/* spacing */
	twoLine		/* style */
};

typedef struct MenuStackEntry {
	Menu	menu;			/* the menu displayed */
	MenuDescriptor *dscr;	/* its descriptor */
	int		selc;			/* index into it for selection */
} MenuStackEntry;

MenuStackEntry menuStack[MAXDEPTH];	/* stack of menus */
int	menuDepth;				/* menu stack depth */

#define TOP		(menuStack[menuDepth])
#define SELC	(*menuNth(TOP.selc))

static Menu menuNth(int);

/*********************************************************************\
**
** showItem(m, i)		display a menu item
**
\*********************************************************************/

static void showItem(m, i)
	Menu m;
	int  i;
{
	int x, y;

	if (! m->keyword || !*m->keyword) return;	/* suppress null keywords */

	if (i == TOP.selc && TOP.dscr->cursorSelect &&  SELC.selector) 
		wstandout(MenuWindow);

	/* === for now, don't worry about determining size === */

	switch (TOP.dscr->style) {
	 case oneLine:
	 case twoLine:
	 	waddstr(MenuWindow, m->keyword);
		break;
	 case multiLine:
	 	getyx(MenuWindow, &y, &x);
	 	waddstr(MenuWindow, m->keyword);
		break;
	 case multiHelp:
	 	getyx(MenuWindow, &y, &x);
	 	waddstr(MenuWindow, m->keyword);
		wmove(MenuWindow, ++y, x);
		waddstr(MenuWindow, m->help);
		break;
	}

	if (i == TOP.selc && TOP.dscr->cursorSelect) {
		wstandend(MenuWindow);
		if (TOP.dscr->style == twoLine) {
		 	getyx(MenuWindow, &y, &x);
			wmove(MenuWindow, y+1, TOP.dscr->column);
			waddstr(MenuWindow, m->help);
			wclrtoeol(MenuWindow);
			wmove(MenuWindow, y, x);
		}
	}

	switch (TOP.dscr->style) {
	 case oneLine:
	 case twoLine:
		waddch(MenuWindow, ' ');
		break;
	 case multiLine:
	 case multiHelp:
		wmove(MenuWindow, y + TOP.dscr->spacing, x);
	}
}

/*********************************************************************\
**
** menuAction(m)		perform a menu action on the selected item
** menuDefault(m, c)	perform a default menu action on a character
** menuCursor(c)		perform cursor actions if applicable
** menuLookup(c)		look up a character in the current menu
**
\*********************************************************************/

static int menuAction(m)
	Menu m;
{
	register int rc;
	if (! *m -> action) return(0);
	rc = (*m->action)(m->arg);
 	if (m->itemType == MenuRETN) menuDepth = 0;
	return(m->itemType == MenuEXIT? -1 : rc);
}

static int menuDefault(m, c)
	Menu m;
	char c;
{
	register int rc = (*m->action)(&c);
	return(m->itemType == MenuEXIT? -1 : rc);
}

static int menuCursor(c)
	char c;
{
	register int  rc = 0;
	register int  horiz = TOP.dscr->style <= twoLine;

	if (!TOP.dscr->cursorSelect) return(0);
	switch (c) {
	 case 0:							/* null -- scancode follows */
	 	switch (c = (*MenuInput)()) {
		 case 72: 						/* up */
		 	if (horiz ? MenuPop() : MenuPrev())  rc = 1;
			break;
		 case 75:						/* left */
			if (horiz? MenuPrev() : MenuPop())  rc = 1;
			break;
		 case 77:						/* right */
			if (!horiz) rc = menuAction(&SELC);
			else if (MenuNext()) rc = 1;
			break;
		 case 80:						/* down */
			if (horiz) rc = menuAction(&SELC);
			else if (MenuNext()) rc = 1;
			break;
		 case 71:						/* home */
		 	TOP.selc = 0;
			rc = 1;
			break;
		 case 79:						/* end */
			/* === */
			rc = 1;
			break;
		 default:
		 	rc = 0;
		}
		break;
	 case ' ':
	 	if (horiz && MenuNext()) rc = 1;
		break;
	 case '\b':
	 	if (horiz && MenuPrev()) rc = 1;
		break;
	 case '\n':							/* newline -- select this */
	 case '\r':							/* also accept return */
		rc = menuAction(&SELC);
		break;
	}
	return(rc);
}

static int menuLookup(c)
	char c;
{
	register Menu m = TOP.menu;
	register i = 0;
	for ( ; ; ++m, ++i) {
		switch (m->itemType) {
		 case MenuENDMARK:	return(menuCursor(c));
		 case MenuHEAD: 	break;
		 case MenuDSCR: 	break;
		 case MenuLINK: 	m = (Menu)m->arg - 1; break;

		 case MenuRETN:
		 case MenuEXIT:
		 case MenuITEM: 
		 	if ((c & 0xff) == (m->selector & 0xff) ||
				'a' <= c && 'z' >= c && c - 'a' + 'A' == m->selector) {
				TOP.selc = i;
				return(menuAction(m));
			}
			break;

		 case MenuCTRL:
		 	if (c <= ' ') return(menuDefault(m, c));
			break;
		 case MenuDFLT:
		 	if (c > ' ')  return(menuDefault(m, c));
			break;
		}
	}
}

static Menu menuNth(i)
	int i;
{
	register Menu m = TOP.menu;
	for ( ; i; ++m, --i) {
		switch (m->itemType) {
		 case MenuENDMARK:	return(0);
		 case MenuHEAD: break;
		 case MenuDSCR: break;
		 case MenuLINK: m = (Menu)m->arg - 1; break;

		 case MenuRETN:
		 case MenuEXIT:
		 case MenuITEM: 
			break;

		 case MenuCTRL:
			break;
		 case MenuDFLT:
			break;
		}
	}
	return (m);
}


/*********************************************************************\
**
** handleInput()		does all the work
**
\*********************************************************************/

void handleInput()
{
	register char c;		/* current character */
	register int  rc;		/* return code from item */

	for (;;) {
		MenuShow();
		/* 
		** decode input
		*/
		rc = menuLookup(c = (*MenuInput)());
		/* decode return code */
		switch (rc) {
		 case  1: continue;						/* action taken */
		 case -1: return;						/* exit */
		 case  0: 								/* nothing found */
			putchar(7);	   			/* beep ==== ought to call bios direct */
			fflush(stdout);
		}
	}
}

/*********************************************************************\
**
** Externally-defined routines.  These are mostly trivial.
**
\*********************************************************************/

int MenuStart(m, d)
	Menu m;
	MenuDescriptor *d;
{
	menuDepth = 0;
	TOP.menu = m;
	TOP.dscr = d;
	if (!MenuWindow) MenuWindow = stdscr;
	MenuHome();
	handleInput();
	return(1);
}

int MenuCall(m)
	Menu m;
{
	menuStack[menuDepth + 1].dscr = TOP.dscr;
	menuDepth++;
	TOP.menu = (m);
	return(MenuHome());
}

int MenuJump(m)
	Menu m;
{
	TOP.menu = m;
	return(MenuHome());
}

int MenuTop(m)
	Menu m;
{
	menuDepth = 0;
	if (m) {
		TOP.menu = m;
		MenuHome();
	}
	return(1);
}

int MenuRtnTo(m)
	Menu m;
{
	for ( ; menuDepth; --menuDepth) if (TOP.menu == m) return(1);
	return(0);
}

int MenuPop()
{
	if (--menuDepth < 0) return(-1);
	return(1);
}

int MenuPop0()
{
	MenuPop();
	return (MenuHome());
}

int MenuHome()
{
	TOP.selc = 0;
	for ( ; ; ++TOP.selc)
		switch (menuNth(TOP.selc)->itemType) {
		 case MenuITEM:
		 case MenuEXIT:
		 case MenuRETN:
		 	return(1);
		 case MenuDSCR:
		 	TOP.dscr = (MenuDescriptor*)SELC.arg;
			break;
		 case MenuENDMARK:
		 	return(-1);
		 case MenuHEAD:
		 	if (SELC.help) return(1);
			break;
		}
	return(1);
}

int MenuNext()
{
	switch (menuNth(TOP.selc + 1)->itemType) {
	 case MenuENDMARK:
	 case MenuLINK:
	 case MenuCTRL:
	 case MenuDFLT:
			return(0);
	}
	++TOP.selc;
	return(1);
}

int MenuPrev()
{
	if (TOP.selc == 0) return(0);
	--TOP.selc;
	return(1);
}

int MenuAct()
{
	return (menuAction(&SELC));
}


/*********************************************************************\
**
** Display Operations
**
\*********************************************************************/

int MenuHide()
{
	
}

int MenuShow()
{
	register int i;
	register Menu m = TOP.menu;
	int y, x, ny, nx;					/* preserve cursor position */

	getyx(MenuWindow, &y, &x);
	for (i = 0 ; ; ++m, ++i) {
		switch (m->itemType) {
		 case MenuENDMARK:
			wclrtoeol(MenuWindow);	/* === probably only needed for oneLine and twoLine */
			wmove(MenuWindow, y, x);
			wrefresh(MenuWindow);
			return(0);
			break;
		 case MenuDFLT:
		 case MenuCTRL:
		 	break;
		 case MenuDSCR:
		 	TOP.dscr = (MenuDescriptor*)(m->arg); 
		 	wmove(MenuWindow, TOP.dscr->row, TOP.dscr->column);
			break;
		 case MenuLINK: 
		 	m = (Menu)m->arg; 
			break;

		 case MenuHEAD:
			if (m->action) {
				getyx(MenuWindow, &ny, &nx);
				menuAction(m);
				wmove(MenuWindow, ny, nx);
			}
		 case MenuRETN:
		 case MenuEXIT:
		 case MenuITEM:
		 	if (i == 0) wmove(MenuWindow, TOP.dscr->row, TOP.dscr->column);
		 	showItem(m, i);
			break;
		}
	}
}



