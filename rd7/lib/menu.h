/*** Header: "MENU.H 1.0 copyright 1987 S. Savitzky" ***/

/*********************************************************************\
**
**	MENU --	General-purpose menu interpretor
**	
**	870719 SS	create PC version.
**
**	This is a general-purpose system for 1-line, full-screen, and
**	pop-up text menus on the PC.  Selection may be single-character,
**	cursor-based, or keyword-oriented. 	Menu items consist of:
**
**		A selector character
**		A keyword string (used in 1-line menus)
**		A help string (also used in multi-line full-screen menus)
**		An action routine and argument.
**
**  The action routine is passed one of
**		an optional string argument
**		a pointer to a character, in the case of a default action.
**	The action routine should return:
**		 1 for normal completion
**		 0 for an error beep
**		-1 to exit the menu system
**
**	Prefix:		Menu
**	Flags:		MenuMACROS	define to get menu-definition macros
**
\*********************************************************************/



/*********************************************************************\
**
**	D A T A   S T R U C T U R E S
**	
\*********************************************************************/

typedef int (*Action)();
typedef void *ActArg;

typedef struct MenuItem {
	char	selector;		/* selector character */
	char	itemType;		/* menu item type */
	char	*keyword;		/* keyword string */
	char	*help;			/* help string */
	Action	action;			/* action routine */
	ActArg	arg;			/* action routine argument */
} MenuItem, *Menu;

/* constants for itemType field */

#define MenuENDMARK	0		/* end of menu */
#define MenuITEM	1		/* normal menu item */
#define MenuEXIT	2		/* exit the menu system after action */
#define MenuRETN	3		/* return to top menu after action */

#define MenuHEAD	4		/* header text -- no selection allowed */
							/* 	if it has help text, it shows on entry */
							/*  action runs before reading each cmd */
#define MenuLINK	5		/* arg is link to another menu */
#define MenuDSCR	6		/* arg is MenuDescriptor */

#define MenuDFLT	7		/* default for normal keys */
#define MenuCTRL	8		/* default for ctrl/keypad keys */
							/*    (no cursor movement in menu if present) */

typedef enum MenuStyle {  	/* Menu display style */
	oneLine,					/* single line */
	twoLine,					/* second line = help on current selection */
	multiLine,					/* item per line */
	multiHelp					/* item per two lines (help on second) */
} MenuStyle;

typedef struct MenuDescriptor {
	char keywordSelect;		/* non-zero if full keywords required */
	char cursorSelect;		/* non-zero if cursor-motion keys can be used */
	short row;				/* starting row */
	short column;			/* starting column */
	short width;  			/* line width (0 = max of contents) */
	short height;			/* height (0 = # of contents) */
	short spacing;			/* spacing between lines */
	MenuStyle style;		/* display style */
} MenuDescriptor;

typedef struct MenuDataDscr {
	char *dest;				/* destination buffer */
	short maxlen;			/* maximum length of buffer */
	char *(*process)();		/* (dscr) -- process the string */
							/* 		 returns error message or NULL */
	char *(*value)();		/* (dscr) format default value string */
	char *format;			/* printf format string with %s for value */
	char *data;				/* -> the data itself */
} MenuDataDscr;


/*********************************************************************\
**
** D a t a
**
\*********************************************************************/

extern char (*MenuInput)();	/* where to get characters */
extern WINDOW *MenuWindow;	/* where to put output */
extern int MenuIsDisplayed;	/* TRUE if some menu is on the screen */
extern MenuDescriptor MenuDefaultDscr;	/* some random descriptor */

/*********************************************************************\
**
**  F u n c t i o n s
**	
\*********************************************************************/

extern int MenuStart();	/* (Menu, Descriptor)	top-level entry */

extern int MenuCall();	/* (Menu)	call a menu */
extern int MenuJump();	/* (Menu)	jump to a menu at the same level */
extern int MenuTop();	/* (Menu)	clear call stack and jump */
extern int MenuRtnTo();	/* (Menu)	clear call stack to inv. of menu */
extern int MenuPop();	/* ()		pop back a level */
extern int MenuPop0();	/* ()		pop back a level and select first */
extern int MenuHome();	/* ()		select first item this level */
extern int MenuNext();	/* ()		select next item */
extern int MenuPrev();	/* ()		select prev. item */
extern int MenuAct();	/* ()		do selected item */

extern int MenuHide();	/* ()		hide the current menu */
extern int MenuShow();	/* ()		show the menu but take no action */

extern int MenuData();	/* (&dscr)	read general data */

extern int MenuString();/* (&str)	read string data, max 256 chars */
extern int MenuLong();	/* (&long)	read long data */
extern int MenuShort();	/* (&short)	read short data */
extern int MenuFloat();	/* (&float) read float data */
extern int MenuDouble();/* (&double) read double data */


/*********************************************************************\
**
** Macros for menu construction
**
\*********************************************************************/

#ifdef MenuMACROS

#define MenuSTART(name)	MenuItem name[] = {
#define MenuEND			{0,0,0,0,0,0} };

/* Item constructors:  {selector, (type), keyword, help, action, arg} */

#define ITEM(s,k,h,act,arg) {s, MenuITEM, k, h, act, arg},
#define EXIT(s,k,h,act,arg) {s, MenuEXIT, k, h, act, arg},
#define RETN(s,k,h,act,arg) {s, MenuRETN, k, h, act, arg},
#define DFLT(act) 			{0, MenuDFLT, 0, 0, act, 0},
#define CTRL(act)		 	{0, MenuCTRL, 0, 0, act, 0},

#define CALL(s,k,h,m)	   	{s, MenuITEM, k, h, MenuCall, &m->selector},
#define JUMP(s,k,h,m)		{s, MenuITEM, k, h, MenuJump, &m->selector},
#define  TOP(s,k,h,m)		{s, MenuITEM, k, h, MenuTop, &m->selector},
#define TOP_(s,k,h)			{s, MenuITEM, k, h, MenuTop, (char*)0},
#define RTTO(s,k,h,m)		{s, MenuITEM, k, h, MenuRtnTo, &m->selector},
#define  POP(s,k,h)			{s, MenuITEM, k, h, MenuPop,  (char *)0},

#define DATA(s,k,h,d)		{s, MenuITEM, k, h, MenuData, (char *)&d},
#define STR(s,k,h,d)		{s, MenuITEM, k, h, MenuString, (char *)&d},
#define SHRT(s,k,h,d)		{s, MenuITEM, k, h, MenuShort, (char *)&d},
#define LONG(s,k,h,d)		{s, MenuITEM, k, h, MenuLong, (char *)&d},
#define FLOT(s,k,h,d)		{s, MenuITEM, k, h, MenuFloat, (char *)&d},
#define DOUB(s,k,h,d)		{s, MenuITEM, k, h, MenuDouble, (char *)&d},

#define LINK(m)				{0, MenuLINK, 0, 0, 0, &m->selector},
#define DSCR(m)				{0, MenuDSCR, 0, 0, 0, &m.keywordSelect},
#define HEAD(s,k,h,act,arg)	{s, MenuHEAD, k, h, act, arg},

#endif

