/***/ static char *moduleid = "MENUTST.C 1.0 copyright 1987 S. Savitzky";

/*********************************************************************\
**
**	MENUTST	-- test menu system
**
**	870808 SS	create.
**
\*********************************************************************/

#include <stdio.h>
#include "curse.h"

#define MenuMACROS
#include "menu.h"

extern int rawKeycode(), rawGetc();

#define global
#define local	static
#define DEBUG	if (debugf) {
#define DEBEND }
global int debugf = 0;

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned long ulong;

/*********************************************************************\
**
**	V A R I A B L E S
**	
\*********************************************************************/

FILE *f;			/* -F option file 			*/
char buf[512];		/* general-purpose buffer	*/
static int application();

MenuSTART(subMenu)
  HEAD("This is a header", "This is a header's help", 0, 0)
  ITEM('A', "subitem_A", "This is a menu item", application, "subitem A")
  RETN('B', "Back to top", "This goes back to top", application, "subitem B")
  ITEM('C', "subItem_C", "This is menu item C", application, "subitem C")
   POP('P', "Pop", "Pop to main menu")
MenuEND

MenuSTART(TopMenu) 
  ITEM('A', "AnItem", "This is a menu item", application, "item A")
  ITEM('B', "item_B", "This is menu item B", application, "item B")
  ITEM('C', "Item_C", "This is menu item C", application, "item C")
  CALL('D', "Submenu_D", "This is submenu D", subMenu)
  EXIT('E', "Exit", "This leaves the test", application, "item E")
MenuEND

/*********************************************************************\
**
** application(string)
**
\*********************************************************************/

static int application(arg)
	char *arg;
{
	move(20,4);
	addch('<');
	addstr(arg);
	addch('>');
	clrtoeol();
	refresh();
	return(1);
}


/*********************************************************************\
**
**	Main.
**	
\*********************************************************************/

main()
{
	int c;

	initscr();		/* initialize screen package */
	clear();		/* clear the screen */
	MenuStart(TopMenu, &MenuDefaultDscr);
	endwin();
}




