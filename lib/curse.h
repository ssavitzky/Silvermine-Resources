/*** Header: "CURSE.H 1.0 copyright 1987 S. Savitzky" ***/

/*********************************************************************\
**
**	CURSE	--	cheap version of "curses"
**
**		A subset of the Unix "curses" package with a few PC extensions.
**
**		(eventually) The environment variable 'TTYTYPE' should be one of:
**			ANSI
**			BIOS
**			MDA
**			CGA
**		If absent, curse attempts to find out by looking at the
**		(undocumented) BIOS screen descriptor.
**	
**	870807 SS	create PC version.
**
\*********************************************************************/

typedef struct WINDOW_ {
	short		cury, curx;		/* current cursor coordinates */
	short		maxy, maxx;		/* size */
	short		begy, begx;		/* origin in parent */
	short		flags;
	short		curattr;		/* current attributes */
	char		normattr;		/* normal attributes */
	char		standattr;		/* standout attributes */
	short		far* *y;		/* array of row pointers (with attributes) */
	short		far* x;			/* pointer to current character */
	struct WINDOW_	*sub,		/* subwindows */
					*sup,		/* superwindow (parent) */
					*sib;		/* next sibling */
} WINDOW;

/* flags */

#define WIN_SUBWIN	0x0001		/* window is a subwindow */
#define WIN_SCREEN	0x0002		/* window occupies full screen */
#define WIN_DIRECT	0x0004		/* window.y points direct to screen buffer */
#define WIN_LEAVE	0x0008		/* leave cursor on screen */
#define WIN_SCROLLOK 0x0010		/* OK to scroll window */

/* Window attributes */

#define WA_NORMAL	0x07		/* normal video attributes */
#define WA_REVERSE	0x70		/* reverse video attributes */
#define WA_ULINED	0x01		/* underlined video attributes */
#define WA_INVERT	0x77		/* xor with this to invert */
#define WA_BOLD		0x08		/* xor with this to intensify */
#define WA_ULINE	0x06		/* xor with this to underline */
#define WA_BLINK	0x80		/* xor with this to blink */

/* Standard Screen */

extern WINDOW *stdscr;

/* Window Functions */

extern WINDOW *newwin();	/* (lines, cols, begy, begx) create a window */
extern WINDOW *subwin();	/* (win, lines, cols, begy, begx)  sub-window */
extern void delwin();		/* (win)	 delete window */

extern void waddch();		/* (win, ch) add a character to a window */
extern void waddstr();		/* (win, str) add a string to window */
extern void wprintf();		/* (win, fmt, ...) printf to window */
extern void wclear();		/* (win) clear the window */
extern void wclrtobot();	/* (win) clear to bottom of window */
extern void wclrtoeol();	/* (win) clear to end of line */
extern void wprintw();		/* (win, fmt, arg1...) window printf */
extern void wrefresh();		/* (win) refresh window -- noop */
extern void wstandout();	/* (win) start standout mode */
extern void wmove();		/* (win, y, x) move to coords */
extern void wstandend();	/* (win) end standout mode */
extern void winsch();		/* (win, c) insert char (push right) */
extern void wdelch();		/* (win) delete char (pull left) */
extern void winsertln();	/* (win) insert blank line (push down) */
extern void wdeleteln();	/* (win) delete line (pull up) */

extern void box();			/* (win, vert, horiz) draw box around window */
extern char winch();		/* (win, y, x)	return the char at y, x */
extern void getyx();		/* (win, &y, &x) get coords */

/* Screen functions (done as macros) */

#define addch(c)	waddch(stdscr,(c))
#define addstr(s)	waddstr(stdscr,(s))
#define clear()		wclear(stdscr)
#define clrtobot()	wclrtobot(stdscr)
#define clrtoeol()	wclrtoeol(stdscr)
#define move(y,x)	wmove(stdscr,(y),(x))
#define refresh()	wrefresh(stdscr)
#define standout()	wstandout(stdscr)
#define standend()	wstandend(stdscr)
#define insch(c)	winsch(stdscr,(c))
#define delch()		wdelch(stdscr)
#define insertln()	winsertln(stdscr)
#define deleteln()	wdeleteln(stdscr)

#define inch(y,x)	winch(stdscr,(y),(x))

extern void printw();		/* (fmt, arg1...) screen printf */

/* initialization */

extern void initscr();		/* initialize */
extern void endwin();		/* ()		 clean up before exiting */

/* input */

extern int wgetch();		/* (win)	get char with echo in window */
extern void wgetstr();		/* (win, str) get string from window */
/*
** If echoing is on, wgetstr displays the cursor. 
** 
** The curses functions getch and getstr are NOT implemented;
** they are defined in the PC's standard library.
*/

/* terminal mode functions */

extern void crmode();		/* leave cbreak mode */
extern void nocrmode();		/* enter cbreak mode */
extern void echo();			/* echo characters to screen */
extern void noecho();		/* don't echo characters to screen */
extern void raw();			/* enter raw mode (no editing) */
extern void noraw();		/* leave raw mode */

/* PC extended functions */

extern void attr();			/* (attr)      set attribute byte */
extern void wattr();		/* (win, attr) set attribute byte */
extern void sattr();		/* (attr)      set standout attribute byte */
extern void wsattr();		/* (win, attr) set standout attribute byte */
extern short wincha();		/* (win, y, x)	char & attributes at y, x */
extern void waddcha();		/* (win, cha)	add char & attrs */
