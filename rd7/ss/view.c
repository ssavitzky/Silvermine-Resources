/*********************************************************************\
**
** View.c	--	View data on the screen
**
**	880422 SS	major changes:  view data structure
**	880111 SS	create PC version from DD
**
\*********************************************************************/

#include <stdio.h>
#include "../lib/curse.h"
#include "../lib/ibmchars.h"

#include "rd7.h"
#include "coops.h"
#include "trees.h"
#include "dirs.h"

#undef  global
#define global

#include "view.h"


extern Object objInit(), objClone(), objKill();
extern Object objDoesNotImplement();

TreeViewRec rFileView, rDosFileView;


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
** Line-Drawing Routines
**
\*********************************************************************/

static hline (win, row, width, beg, mid, end)
	WINDOW *win;
	int row, width;
	char beg, mid, end;
{
	register int i;
	wmove(win, row, 0);
	waddch(win, beg);
	for (i = 1; ++i < width; ) waddch(win, mid);
	waddch(win, end);
}

static vlines (win, row, width, height, chr)
	WINDOW *win;
	int row, width, height;
	char chr;
{
	register int i;
	for (i = 0; i < height; ++i, ++row) {
		wmove(win, row, 0);
		waddch(win, chr);
		wmove(win, row, width - 1);
		waddch(win, chr);
	}
}


/*********************************************************************\
**
** Generic Operations
**
\*********************************************************************/

static Object vuInit(v)
	View v;
{
	char ul = D_LEFT, ur = D_RIGHT, ll = D_LL, lr = D_LR;
	ushort nrows = v -> view.rows;
	ushort ncols = v -> view.cols;
	WINDOW *box = newwin(nrows + 2, ncols + 2, 
						 v -> view.y - 1, v -> view.x - 1);

	if (v -> view.x > 1) {
		ul = D_TOP; 
		ll = D_BOT;
	}
	if (v -> view.x + ncols + 2 < swidth) {
		ur = D_TOP;
		lr = D_BOT;
	}
	v -> view.box = box;
	v -> view.win = subwin(box, nrows, ncols, 1, 1);
	wclear(box);
	hline (box, 0, ncols + 2, ul, D_HORIZ, ur);
	vlines (box, 1, ncols + 2, nrows, D_VERT);
	hline (box, nrows + 1, ncols + 2, ll, D_HORIZ, lr);
	return ((Object) v);
}

static String vName(v)
	View v;
{
	return(v -> view.name);
}

static Object vOpen(v)
	View v;
{
	static void vLabel();
	static void vRefresh();

	vLabel(v, gName(v), "=\315");
	vRefresh(v);
	return ((Object) v);
}


static void vRefresh(v)
	View v;
{
	if (v -> view.do_update) gVuUpdAll(v);
	if (v -> view.partner && v -> view.partner -> view.do_update) 
		gVuUpdAll(v -> view.partner);
	if (v -> view.partner && v -> view.partner -> view.do_refresh) {
		wrefresh(v -> view.partner -> view.box);
		v -> view.partner -> view.do_refresh = FALSE;
	}
	wrefresh(v ->view.box);
	v -> view.do_refresh = FALSE;
}

static void vLabel(v, s, g)
	View v;
	String s;
	String g;
{
	register WINDOW *win = v -> view.win;
	register int i;
	int x = win -> begx;
	int y = win -> begy - 1;		/* in the box */
	int w = win -> maxx;
	int h = win -> maxy;

	win = v -> view.box;
	v -> view.do_refresh = TRUE;

	/*
	** First char. of geometry is =, +, or - for center, left, right.
	** If =, second char is padding if present.
	** Find out where the window is relative to its box.
	** Set position accordingly.
	*/
	if (!g) g = "=";

	switch (*g) {
	 case '=':
	 	if (g[1]) {
			wmove(win, y, x);
			for (i = 0; i < w; ++i) waddch(win, g[1]);
		}
	 	wmove(win, y, x + (w - strlen(s)) / 2);
		break;
	 case '+':
		wmove(win, y, x);
		break;
	 case '-':
	 	wmove(win, y, x + w - strlen(s));
		break;
	}
	waddstr(win, s);
}

static void vLnUp(v)
	View v;
{
	gVuMvLns(v, -1);
}

static void vLnDn(v)
	View v;
{
	gVuMvLns(v, 1);
}

static void vPgUp(v)
	View v;
{
	gVuMvLns(v, 0 - v -> view.rows - v -> view.cur_row);
}

static void vPgDn(v)
	View v;
{
	gVuMvLns(v, 2 * v -> view.rows - v -> view.cur_row - 1);
}

static void vLast(v)
	View v;
{
	int n;

	gVuMvLns(v, 32000);
}

/*********************************************************************\
**
** Generic Tree View Operations
**
\*********************************************************************/

static void vTreeFirst(v)
	TreeView v;
{
	v -> view.cur_row = 0;
	v -> treeview.cur = v -> treeview.top = v -> treeview.root;
	v -> view.do_refresh = TRUE;
	v -> view.do_update = TRUE;
}

static void vTreeSet(v, t)
	TreeView v;
	Tree t;
{
	v -> treeview.root = t;
	gVuFirst(v);
}

static void vTreeReset(v)
	TreeView v;
{
	if (ISNULL(v -> treeview.root)) return;
	v ->treeview.root = (Tree)NIL;
	gVuFirst(v);
	if (v -> view.partner)
		gVuReset(v -> view.partner);
}


/*********************************************************************\
**
** Directory View
**
**		Note that these keep the File View up to date in parallel.
**
\*********************************************************************/

static void vDirStat(v)
	TreeView v;
{
	register Dir d = ((Dir) v -> treeview.root);
	register int i;
	static char s[80];

	sprintf(s, "\315 %s ", v -> view.name);
	if (ISNULL(d)) {
		sprintf(s + strlen(s), "[empty]");
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


static Tree dirPrev(d)
	Tree d;
{
	for (d = gPred(d); NOTNULL(d) && !ISDIR(d); d = gPred(d)) ;
	return (d);
}

static Tree dirNext(d)
	Tree d;
{
	for (d = gSucc(d); NOTNULL(d) && !ISDIR(d); d = gSucc(d)) ;
	return (d);
}

static Tree nextSib(d)
	Tree d;
{
	for (d = gNext(d); NOTNULL(d) && !ISDIR(d); d = gNext(d)) ;
	return (d);
}

static Tree nextKid(d)
	Tree d;
{
	for (d = gDown(d); NOTNULL(d) && !ISDIR(d); d = gNext(d)) ;
	return (d);
}

static void vIndentDir(win, node)
	WINDOW *win;
	Tree node;
{
	Tree p;

	if (ISNULL(node)) return;
	if (NOTNULL(p = gUp(node))) vIndentDir(win, p);
	if (NOTNULL(nextSib(node))) waddstr(win, "\263 ");
	else           		   		waddstr(win, "  ");
}

static void vUpdDirPartner(v)
	TreeView v;
{
	if (NOTNULL(v -> view.partner)) {
		vTreeSet(v -> view.partner, gDown(v -> treeview.cur));
		gVuUpdAll(v -> view.partner);
	} else {
		vDirStat(v);
	}
}


static void vUpdDir(v, line, node)
	TreeView v;
	int line;
	Dir node;
{
	register int i;
	register Dir p;
	WINDOW		*win = v -> view.win;

	wmove(win, line, 0);
	wclrtoeol(win);
								/* if line is empty, just exit */
	if (ISNULL(node)) return;
								/* indent according to depth */
	vIndentDir(win, (p = (Dir)gUp(node)));
	/*
	** Decide on L vs. T for each end of the branch line
	*/
	if (NOTNULL(nextSib(node)))	waddstr(win, "\303\304");
	else if (NOTNULL(p))		waddstr(win, "\300\304");
	else						waddstr(win, "  ");
	if (NOTNULL(nextKid(node)))	waddch (win, 0302);
	else						waddch (win, 0304);

								/* if line has the cursor, set standout */
	if (node == (Dir) v -> treeview.cur)
		wstandout(win);
								/* put out the node's name */
	waddstr(win, gName(node));
								/* if node is tagged, so mark it */
	if (node -> dir.tcount) {
		waddch(win, '*');
	} else {
		waddch(win, ' ');
	}
								/* remove standout */
	wstandend(win);
}

global void vDirAll(v)
	TreeView v;
{
	Tree n;
	int i;

	v -> view.do_update = FALSE;
	v -> view.do_refresh= TRUE;
	wclear(v -> view.win);
	if (ISNULL(v -> treeview.root)) {
		wmove(v -> view.win, 0, 0);
		waddstr(v -> view.win, "    <no directories read>");
		vTreeReset(v -> view.partner);
		gVuUpdAll(v -> view.partner);
		return;
	}
	if (v -> view.cur_row >= v -> view.rows) {
		i = v -> view.cur_row = v -> view.rows - 1;
		for (n = v -> treeview.top = v -> treeview.cur; 
			 i && (n = dirPrev(n)); --i, v -> treeview.top = n) ;
		v -> view.cur_row -= i;
	}
	if (v -> view.cur_row < 0) {
		v -> view.cur_row = 0;
		v -> treeview.top = v -> treeview.cur;
	}
	for (n = v -> treeview.top, i = 0; 
		 NOTNULL(n) && i < v -> view.rows; 
		 ++i, n = dirNext(n)
		) vUpdDir(v, i, n);
	wrefresh(v -> view.win);
	vUpdDirPartner(v);
}

global void vDirLine(v)
	TreeView v;
{
	v -> view.do_update = FALSE;
	if (v -> treeview.cur) {
		vUpdDir(v, v -> view.cur_row, v -> treeview.cur);
	}
	wrefresh(v -> view.win);
	if (NOTNULL(v -> view.partner)) {
		vTreeSet(v -> view.partner, gDown(v -> treeview.cur));
		gVuUpdAll(v -> view.partner);
	} else {
		vDirStat(v);
	}
}

static void vDirMvLns(v, n)
	TreeView v;
	int n;
{
	Tree f;
	int  r;
	int  oldRow = v -> view.cur_row;
	Tree oldCur = v -> treeview.cur;

	if (ISNULL(v -> treeview.root)) return;
	v -> view.do_refresh= TRUE;
	r = oldRow;

	if (n < 0)
		for ( ; n && NOTNULL(f = dirPrev(v -> treeview.cur)); ++n) {
			-- r;
			v -> treeview.cur = f;
		}
	else
		for ( ; n && NOTNULL(f = dirNext(v -> treeview.cur)); --n) {
			++ r;
			v -> treeview.cur = f;
		}
	v -> view.cur_row = r;
	if (v -> view.do_update) {						/* already refreshing */
		return; /* don't bother */
	} else if (r >= 0 && r < v -> view.rows) {		/* on-screen */
		vUpdDir(v, oldRow, oldCur);
		vUpdDir(v, r, v -> treeview.cur);
		vUpdDirPartner(v);
	} else if (r == -1) {							/* off the top */
		vUpdDir(v, oldRow, oldCur);
		v -> treeview.top = v -> treeview.cur;
		wmove(v -> view.win, 0, 0);
		winsertln(v -> view.win);
		v -> view.cur_row = 0;
		vUpdDir(v, 0, v -> treeview.cur);
		vUpdDirPartner(v);
	} else if (r == v -> view.rows) {				/* off the bottom */
		vUpdDir(v, oldRow, oldCur);
		v -> treeview.top = dirNext(v -> treeview.top);
		wmove(v -> view.win, 0, 0);
		wdeleteln(v -> view.win);
		v -> view.cur_row = --r;
		vUpdDir(v, r, v -> treeview.cur);
		vUpdDirPartner(v);
	} else {										/* too far: refresh */
		v -> view.do_update = TRUE;
	}
}

ViewClassRec crDirView = {
	&crClass,				/* class */				/* Object Part */
   {												/* ObjectClass Part */
	sizeof (ViewRec),		/* instance size */
	"DirView",				/* class name */
	&crObject,				/* super */
	NIL,					/* method dict */
	vuInit, 				/* initialize */
	objDoesNotImplement,	/* new object */
	objClone,				/* clone self */
	objKill,				/* kill self */
	vOpen,					/* open */
	objDoesNotImplement,	/* close */
	vName,					/* name */
   }, {												/* ViewClassPart */
	vRefresh,				/* refresh display */
	vDirAll,				/* update function */
	vDirLine,				/* update line function */
	vLabel,					/* label */
	vDirMvLns,				/* move n lines */
	vLnUp,					/* goes up a line */
	vLnDn,					/* goes down a line */
	vPgUp,					/* goes up a page */
	vPgDn,					/* goes down a page */
	vTreeFirst,				/* first directory */
	vLast,					/* last directory */
	vTreeSet,				/* set tree */
	vTreeReset,				/* reset tree to null */
   },
};

TreeViewRec rDirView = {
	(Class)&crDirView,
   {	
	(WINDOW *) 0L,		/* the window the view is in */
	(WINDOW *) 0L,		/* the window for the box around it */
	"Idris",			/* name */
	0, 0, 0, 0,	0,		/* flag word */
	DIRCOL,				/* window X position */
	DATAROW,			/* window Y position */
	DIRCOLS,			/* window width */
	VIEWROWS,			/* window height */
	0,					/* cursor row */
	0,					/* cursor column */
	(View)&rFileView,			/* associated view */
   },
};

TreeViewRec rDosView = {
	(Class)&crDirView,
   {	
	(WINDOW *) 0L,		/* the window the view is in */
	(WINDOW *) 0L,		/* the window for the box around it */
	"DOS",				/* the name */
	0, 0, 0, 0,	0,		/* flag word */
	DIRCOL,				/* window X position */
	DATAROW,			/* window Y position */
	DIRCOLS,			/* window width */
	VIEWROWS,			/* window height */
	0,					/* cursor row */
	0,					/* cursor column */
	(View)&rDosFileView,		/* associated view */
   },
};



/*********************************************************************\
**
** File View
**
\*********************************************************************/

static void vUpdFile(v, line, node)
	TreeView v;
	int line;
	Dir node;
{
	WINDOW *win = v -> view.win;
	char buf[81];

	wmove(win, line, 1);
	if (NOTNULL(node)) {
		if (v -> view.is_active && node == (Dir) (v -> treeview.cur))
			wstandout(win);	   /* if line has the cursor, set standout */
		gHeader(node, buf, (Cardinal) v -> view.cols - 1);
		waddstr(win, buf);
		wstandend(win);
	}
	wclrtoeol(win);
}

static Tree filePrev(d)
	Tree d;
{
	for (d = gPrev(d); NOTNULL(d) && ISDIR(d); d = gPrev(d)) ;
	return (d);
}

static Tree fileNext(d)
	Tree d;
{
	for (d = gNext(d); NOTNULL(d) && ISDIR(d); d = gNext(d)) ;
	return (d);
}

static void vFileAll(v)
	TreeView v;
{
	Tree n;
	int i;

	v -> view.do_update = FALSE;
	v -> view.do_refresh= TRUE;
	wclear(v -> view.win);
	if (v -> view.cur_row >= v -> view.rows) {
		i = v -> view.cur_row = v -> view.rows - 1;
		for (n = v -> treeview.top = v -> treeview.cur; 
			 i && n && (n = filePrev(n)); --i, v -> treeview.top = n) ;
		v -> view.cur_row -= i;
	}
	if (v -> view.cur_row < 0) {
		v -> view.cur_row = 0;
		v -> treeview.top = v -> treeview.cur;
	}
	for (n = v -> treeview.top, i = 0; 
		 NOTNULL(n) && i < v -> view.rows; 
		 n = fileNext(n)
		) if (!ISDIR(n)) {vUpdFile(v, i, n); ++i;}
	if (i == 0) {
		wmove(v -> view.win, 0, 0);
		waddstr(v -> view.win, "    <no files>");
	}
	if (v -> view.partner) vDirStat(v -> view.partner);
	wrefresh(v -> view.win);
}

static void vFileLine(v)
	TreeView v;
{
	v -> view.do_update = FALSE;
	if (NOTNULL(v -> treeview.cur)) {
		vUpdFile(v, v -> view.cur_row, v -> treeview.cur);
	}
	if (v -> view.partner) vDirStat(v -> view.partner);
	wrefresh(v -> view.win);
}

static void vFileMvLns(v, n)
	TreeView v;
	int  n;
{
	Tree f;
	int  r;
	int  oldRow = v -> view.cur_row;
	Tree oldCur = v -> treeview.cur;

	if (ISNULL(v -> treeview.root)) return;
	v -> view.do_refresh= TRUE;
	r = oldRow;

 	if (n < 0)
		for ( ; n && NOTNULL(f = filePrev(v -> treeview.cur)); ++n) {
			-- r;
			v -> treeview.cur = f;
		}
	else
		for ( ; n && NOTNULL(f = fileNext(v -> treeview.cur)); --n) {
			++ r;
			v -> treeview.cur = f;
		}
	v -> view.cur_row = r;
	if (v -> view.do_update) {						/* already refreshing */
		return; /* don't bother */
	} else if (r >= 0 && r < v -> view.rows) {		/* on-screen */
		vUpdFile(v, oldRow, oldCur);
		vUpdFile(v, r, v -> treeview.cur);
	} else if (r == -1) {							/* off the top */
		vUpdFile(v, oldRow, oldCur);
		v -> treeview.top = v -> treeview.cur;
		wmove(v -> view.win, 0, 0);
		winsertln(v -> view.win);
		v -> view.cur_row = 0;
		vUpdFile(v, 0, v -> treeview.cur);
	} else if (r == v -> view.rows) {				/* off the bottom */
		vUpdFile(v, oldRow, oldCur);
		v -> treeview.top = fileNext(v -> treeview.top);
		wmove(v -> view.win, 0, 0);
		wdeleteln(v -> view.win);
		v -> view.cur_row = --r;
		vUpdFile(v, r, v -> treeview.cur);
	} else {										/* too far: refresh */
		v -> view.do_update = TRUE;
	}
}

void vFileFirst (v)
	TreeView v;
{
	vTreeFirst(v);
	for ( ; 
		 NOTNULL(v -> treeview.cur) && ISDIR(v -> treeview.cur); 
		 v -> treeview.cur = gNext(v -> treeview.cur)
		) ;
}


ViewClassRec crFileView = {
	&crClass,				/* class */				/* Object Part */
   {												/* ObjectClass Part */
	sizeof (ViewRec),		/* instance size */
	"FileView",				/* class name */
	&crObject,				/* super */
	NIL,					/* method dict */
	vuInit, 				/* initialize */
	objDoesNotImplement,	/* new object */
	objClone,				/* clone self */
	objKill,				/* kill self */
	vOpen,					/* open */
	objDoesNotImplement,	/* close */
	vName,					/* name */
   }, {												/* ViewClassPart */
	vRefresh,				/* refresh display */
	vFileAll,				/* update function */
	vFileLine,				/* update line function */
	vLabel,					/* label */
	vFileMvLns,				/* move n lines */
	vLnUp,					/* goes up a line */
	vLnDn,					/* goes down a line */
	vPgUp,					/* goes up a page */
	vPgDn,					/* goes down a page */
	vFileFirst,				/* first file */
	vLast,					/* last file */
	vTreeSet,				/* set tree */
	vTreeReset,				/* reset tree to null */
   },
};

TreeViewRec rFileView = {
	(Class)	&crFileView,
   {	
	(WINDOW *) 0L,		/* the window the view is in */
	(WINDOW *) 0L,		/* the window for the box around it */
	"Files",			/* the name */
	0, 0, 0, 0,	0,		/* flag word */
	FILECOL,			/* window X position */
	DATAROW,			/* window Y position */
	FILECOLS,			/* window width */
	VIEWROWS,			/* window height */
	0,					/* cursor row */
	0,					/* cursor column */
	(View)&rDirView,	/* associated view */
   },
};

TreeViewRec rDosFileView = {
	(Class)	&crFileView,
   {	
	(WINDOW *) 0L,		/* the window the view is in */
	(WINDOW *) 0L,		/* the window for the box around it */
	"Files",			/* the name */
	0, 0, 0, 0,	0,		/* flag word */
	FILECOL,			/* window X position */
	DATAROW,			/* window Y position */
	FILECOLS,			/* window width */
	VIEWROWS,			/* window height */
	0,					/* cursor row */
	0,					/* cursor column */
	(View)&rDosView,	/* associated view */
   },
};


/*********************************************************************\
**
** Text View
**
\*********************************************************************/

static void vUpdText(v, row, off)
	FileView v;
	int  row;
	long off;
{
	register int r, c;
	WINDOW *w = v -> view.win;
	char huge *p = v -> fileview.origin + off;
	char huge *lim = v -> fileview.origin + v -> fileview.limit;

	/*
	** Update the row
	*/
	wmove(w, row, 0);
	for (c = 0; *p && *p != '\n' && p < lim; ++p, ++c) {
		if (*p == '\t') c = (c + 8) & ~7;
		if (c < v -> view.cols && *p != '\r') waddch (w, *p); 
	}
	wclrtoeol(w);
}

static long textPrev(org, n)
	char huge *org;
	long n;
{
	for ( ; n && org[n] != '\n'; --n) ;
	if (n == 0) return(n);
	for (--n; n && org[n] != '\n'; --n) ;
	return (org[n] == '\n'? n + 1 : n);
}

global void vTextAll(v)
	FileView v;
{
	register int i, r, c;
	WINDOW *w = v -> view.win;
	long n;
	char huge *org = v -> fileview.origin;
	char huge *lim = org + v -> fileview.limit;
	char huge *p;

	v -> view.do_update = FALSE;
	v -> view.do_refresh= TRUE;
	wclear(v -> view.win);
	if (v -> view.cur_row >= v -> view.rows) {
		i = v -> view.cur_row = v -> view.rows - 1;
		for (n = v -> fileview.top = v -> fileview.cur; 
			 i && (n = v -> fileview.top = textPrev(org, n));
			 --i) ;
		v -> view.cur_row -= i;
	}
	if (v -> view.cur_row < 0) {
		v -> view.cur_row = 0;
		v -> fileview.top = v -> fileview.cur;
	}
	for (r = 0, p = org + v -> fileview.top; r < v -> view.rows; ++r, ++p) {
		wmove(w, r, 0);
		for (c = 0; *p && *p != '\n' && p < lim; ++p, ++c) {
			if (*p == '\t') c = (c + 8) & ~7;
			if (c < v -> view.cols && *p != '\r') waddch (w, *p); 
		}
		wclrtoeol(w);
	}
}

void vTextLine(v)
	FileView v;
{
	vTextAll(v);
}

static void vTextMvLns(v, n)
	FileView v;
	int  n;
{
	register char huge *org = v -> fileview.origin;
	register ulong lim = v -> fileview.limit;
	long off = v -> fileview.cur;
	int  r;
	int  oldRow = v -> view.cur_row;
	long oldCur = v -> fileview.cur;

	if (ISNULL(org)) return;
	v -> view.do_refresh= TRUE;
	r = oldRow;

	if (n >= 0) {
		for (; off < lim && n; ++off)
			if (org[off] == '\n') --n, ++r;
	} else {
		if (off > 0) --off;		/* in case we're at EOL and no CR */
		for ( ; off > 0 && n; --off)
			if (org[off] == '\n') ++n, --r;
		if (n == 0 && off > 0) {
			for ( ; off > 0 && org[off] != '\n'; --off) ;
			if (off > 0) ++off;
		}
	}
	v -> view.cur_row = r;
	v -> fileview.cur = off;
	if (v -> view.do_update) {						/* already refreshing */
		return; /* don't bother */
	} else if (r >= 0 && r < v -> view.rows) {		/* on-screen */
		vUpdText(v, r, v -> fileview.cur);
	} else if (r == -1) {							/* off the top */
		v -> fileview.top = v -> fileview.cur;
		wmove(v -> view.win, 0, 0);
		winsertln(v -> view.win);
		v -> view.cur_row = 0;
		vUpdText(v, 0, v -> fileview.cur);
	} else if (r == v -> view.rows) {				/* off the bottom */
		for (; 
			 org[v -> fileview.top] != '\n'; 
			 ++ v -> fileview.top) ;
		++ v -> fileview.top;
		wmove(v -> view.win, 0, 0);
		wdeleteln(v -> view.win);
		v -> view.cur_row = --r;
		vUpdText(v, r, v -> fileview.cur);
	} else {										/* too far: refresh */
		v -> view.do_update = TRUE;
	}
}

int vTextFirst(v)
	FileView v;
{
	v -> view.do_update = TRUE;
	v -> fileview.top = v -> fileview.cur = 0;
	v -> view.cur_row = 0;
}

void vTextLast(v)
	FileView v;
{
	v -> view.do_update = TRUE;
	v -> fileview.top = v -> fileview.cur = v -> fileview.limit;
	v -> view.cur_row = v -> view.rows - 1;
	vTextMvLns(v, - (v -> view.rows - 1));
	v -> fileview.top = v -> fileview.cur;
}

static void vFileSet(v, s)
	FileView	v;
	char  huge *s;
{
	v -> fileview.origin = s;
	gVuFirst(v);
}

static void vFileReset(v)
	FileView	v;
{
	v -> fileview.origin = NULL;
	gVuFirst(v);
}

/*
** The following line up and down operations also apply to HelpView.
** They take into account the fact that the current line is not
** highlighted.
*/
void vTextLnUp(v)
	View v;
{
	gVuMvLns(v, - (v -> view.cur_row + 1));
}

void vTextLnDn(v)
	View v;
{
	gVuMvLns(v, v -> view.rows - v -> view.cur_row);
}


ViewClassRec crTextView = {
	&crClass,				/* class */				/* Object Part */
   {												/* ObjectClass Part */
	sizeof (ViewRec),		/* instance size */
	"TextView",				/* class name */
	&crObject,				/* super */
	NIL,					/* method dict */
	vuInit, 				/* initialize */
	objDoesNotImplement,	/* new object */
	objClone,				/* clone self */
	objKill,				/* kill self */
	vOpen,					/* open */
	objDoesNotImplement,	/* close */
	vName,					/* name */
   }, {												/* ViewClassPart */
	vRefresh,				/* refresh display */
	vTextAll,				/* update function */
	vTextLine,				/* update line function */
	vLabel,					/* label */
	vTextMvLns,				/* move n lines */
	vTextLnUp,				/* goes up a line */
	vTextLnDn,				/* goes down a line */
	vPgUp,					/* goes up a page */
	vPgDn,					/* goes down a page */
	vTextFirst,				/* first directory */
	vTextLast,				/* last directory */
	vFileSet,	  			/* set tree */
	vFileReset,				/* reset tree to null */
   },
};

static char pathbuf[256];

FileViewRec rTextView = {
	(Class)	&crTextView,
   {	
	(WINDOW *) 0L,		/* the window the view is in */
	(WINDOW *) 0L,		/* the window for the box around it */
	pathbuf,			/* the name (replaced by filename) */
	0, 0, 0, 0,	0,		/* flag word */
	DIRCOL,				/* window X position */
	DATAROW,			/* window Y position */
	VIEWCOLS,			/* window width */
	VIEWROWS,			/* window height */
	0,					/* cursor row */
	0,					/* cursor column */
	(View) NIL,			/* associated view */
   },
};


/*********************************************************************\
**
** Binary View
**
\*********************************************************************/

static char *hexdigits = "0123456789abcdef";

static void vUpdBin(v, row)
	FileView v;
	int  row; 		/* row 0 is first DATA row, actually row 1 of screen */
{
	register int c;
	WINDOW *w = v -> view.win;
	ulong adr = v -> fileview.top + row * 16;	/* address origin in file */
	ulong lim = v -> fileview.limit;				/* address limit */
	char huge *p   = v -> fileview.origin + adr;	/* first char. of row */

	wmove(w, row + 1, 0);
	if (adr >= lim) {
		wclrtoeol(w);
		return;
	}
	wprintf(w, "%8lx: ", adr);
	for (c = 0; c < 16; ++c) {
		register ushort d = p[c] & 255;
		if (adr + c < lim) {
			waddch(w, hexdigits[d >> 4]);
			waddch(w, hexdigits[d & 15]);
		} else {
			waddch(w, ' ');
			waddch(w, ' ');
		}
		waddch(w, ' ');
	}
	waddch(w, ' ');
	for (c = 0; c < 16; ++c) {
		register int cc = p[c] & 255;
		if (adr + c < lim) {
			waddch(w, cc >= ' ' && cc < 0x7f ? cc : '.');
		} else {
			waddch(w, ' ');
		}
	}
	wclrtoeol(w);
}

global void vBinAll(v)
	FileView v;
{
	register int r;
	WINDOW *w = v -> view.win;

	v -> view.do_update = FALSE;
	v -> view.do_refresh= TRUE;
	wmove(w, 0, 0); 
	wclrtoeol(w);
	for (r = 0; r < 16; ++r) {
		vUpdBin(v, r);
	}
	wmove(w, r + 1, 0);
	wclrtobot(w);
}

void vBinLine(v)
	FileView v;
{
	vBinAll(v);
}

void vBinMvLns(v, n)
	FileView v;
	int  n;
{
	long off  = v -> fileview.top + (long) n * 16L;

	v -> view.do_refresh= TRUE;
	if (off < 0L) {
		n -= off / 16L;
		off = 0L;
	}
	if (off > (v -> fileview.limit & 0xffffff00L)) {
		vBinLast(v); return;
	} else
		v -> fileview.top = off;
	/*
	** Now figure out whether a simple scroll will do it.
	*/
	if (n == 1 && !v -> view.do_update) {	 	  /* down 1 */
		wmove(v -> view.win, 1, 0);
		wdeleteln(v -> view.win);
		vUpdBin(v, 15);
	} else if (n == -1 && !v -> view.do_update) { /* up 1 */
		wmove(v -> view.win, 1, 0);
		winsertln(v -> view.win);
		vUpdBin(v, 0);
	} else {
		v -> view.do_update = TRUE;
	}
}

static void vBinPgUp(v)
	View v;
{
	gVuMvLns(v, -16);
}

static void vBinPgDn(v)
	View v;
{
	gVuMvLns(v, 16);
}

int vBinFirst(v)
	FileView v;
{
	v -> view.do_update = TRUE;
	v -> view.do_refresh= TRUE;
	v -> fileview.top = 0;
}

int vBinLast(v)
	FileView v;
{
	v -> view.do_update = TRUE;
	v -> view.do_refresh= TRUE;
	if (v -> fileview.limit > 256L) {
		v -> fileview.top = (v -> fileview.limit) & 0xffffff00L;
	} else {
		v -> fileview.top = 0;
	}
}


ViewClassRec crBinaryView = {
	&crClass,				/* class */				/* Object Part */
   {												/* ObjectClass Part */
	sizeof (ViewRec),		/* instance size */
	"BinaryView",			/* class name */
	&crObject,				/* super */
	NIL,					/* method dict */
	vuInit, 				/* initialize */
	objDoesNotImplement,	/* new object */
	objClone,				/* clone self */
	objKill,				/* kill self */
	vOpen,					/* open */
	objDoesNotImplement,	/* close */
	vName,					/* name */
   }, {												/* ViewClassPart */
	vRefresh,				/* refresh display */
	vBinAll,				/* update function */
	vBinLine,				/* update line function */
	vLabel,					/* label */
	vBinMvLns,				/* move n lines */
	vLnUp,					/* goes up a line */
	vLnDn,					/* goes down a line */
	vBinPgUp,				/* goes up a page */
	vBinPgDn,				/* goes down a page */
	vBinFirst,				/* first directory */
	vBinLast,				/* last directory */
	vFileSet,				/* set tree */
	vFileReset,				/* reset tree to null */
   },
};

FileViewRec rBinaryView = {
	(Class)	&crBinaryView,
   {	
	(WINDOW *) 0L,		/* the window the view is in */
	(WINDOW *) 0L,		/* the window for the box around it */
	pathbuf,			/* the name (replaced by filename) */
	0, 0, 0, 0,	0,		/* flag word */
	DIRCOL,				/* window X position */
	DATAROW,			/* window Y position */
	VIEWCOLS,			/* window width */
	VIEWROWS,			/* window height */
	0,					/* cursor row */
	0,					/* cursor column */
	(View) NIL,			/* associated view */
   },
};


/*********************************************************************\
**
** Help
**
\*********************************************************************/

static void vUpdHelp(v, row)
	HelpView v;
	int  row;
{
	WINDOW *w = v -> view.win;
	char *p = v -> helpview.origin[v -> helpview.top + row];
	register int i, r;

	/*
	** Update the row
	*/
	wmove(w, row, 0);
	if (p && v -> view.is_centered)
		for (i = (v -> view.cols - strlen(p))/2; i > 0; --i)
			waddch(w, ' ');
	if (p) waddstr(w, p);
	wclrtoeol(w, row);
}

void vHelpAll(v)
	HelpView v;
{
	WINDOW *win = v -> view.win;
	char **hmsg;
	register int i, r;

	v -> view.do_update = FALSE;
	v -> view.do_refresh= TRUE;
	wclear(v -> view.win);
	if (v -> view.cur_row >= v -> view.rows) {
		v -> view.cur_row = v -> view.rows - 1;
		if (v -> view.cur_row > v -> helpview.cur) 
			v -> view.cur_row = v -> helpview.cur;
	}
	if (v -> view.cur_row < 0) {
		v -> view.cur_row = 0;
		v -> helpview.top = v -> helpview.cur;
	}
	v -> helpview.top = v -> helpview.cur - v -> view.cur_row;
	hmsg = v -> helpview.origin + v -> helpview.top;
	for (r = 0; *hmsg && r < v -> view.rows; ++hmsg, ++r) {
		wmove(win, r, 0);
		if (v -> view.is_centered)
			for (i = (v -> view.cols - strlen(*hmsg))/2; i > 0; --i)
				waddch(win, ' ');
		waddstr(win, *hmsg);
		wclrtoeol(win, r);
	}
	if (r < v -> view.rows)	{
		wmove(win, r, 0);
		wclrtobot(win);
	}
}

void vHelpLine(v)
	HelpView v;
{
	if (ISNULL(v -> view.partner))
		vHelpAll(v);
	else {
		vDirStat(v -> view.partner);
	}
}

void vHelpMvLns(v, n)
	HelpView v;
	int n;
{
	register char **org = v -> helpview.origin;
	register char **cp;
	register int i;
	long off = v -> helpview.cur;
	int  r;
	int  oldRow = v -> view.cur_row;
	long oldCur = v -> helpview.cur;

	v -> view.do_refresh= TRUE;
	r = oldRow;

	if (n >= 0) {
		for (; org[off] && n; --n, ++off, ++r) ;
	} else {
		if (off < -n) {
			off = 0;
			r = 0;
		} else {
			off += n;
			r += n;
		}
	}
	v -> view.cur_row = r;
	v -> helpview.cur = off;
	if (v -> view.do_update) {						/* already refreshing */
		return; /* don't bother */
	} else if (r >= 0 && r < v -> view.rows) {		/* on-screen */
		vUpdHelp(v, r);
	} else if (r == -1) {							/* off the top */
		v -> helpview.top = v -> helpview.cur;
		wmove(v -> view.win, 0, 0);
		winsertln(v -> view.win);
		v -> view.cur_row = 0;
		vUpdHelp(v, 0);
	} else if (r == v -> view.rows) {				/* off the bottom */
		++ v -> helpview.top;
		wmove(v -> view.win, 0, 0);
		wdeleteln(v -> view.win);
		v -> view.cur_row = --r;
		vUpdHelp(v, r);
	} else {										/* too far: refresh */
		v -> view.do_update = TRUE;
	}
}

static void vHelpFirst(v)
	HelpView v;
{
	v -> helpview.cur = v -> helpview.top = 0;
	v -> view.cur_row = 0;
	v -> view.do_update = TRUE;
}

static void vHelpSet(v, s)
	HelpView	v;
	char  **s;
{
	v -> helpview.origin = s;
	gVuFirst(v);
}

static void vHelpReset(v)
	HelpView	v;
{
	v -> helpview.origin = NULL;
	gVuFirst(v);
}

ViewClassRec crHelpView = {
	&crClass,				/* class */				/* Object Part */
   {												/* ObjectClass Part */
	sizeof (ViewRec),		/* instance size */
	"HelpView",				/* class name */
	&crObject,				/* super */
	NIL,					/* method dict */
	vuInit, 				/* initialize */
	objDoesNotImplement,	/* new object */
	objClone,				/* clone self */
	objKill,				/* kill self */
	vOpen,					/* open */
	objDoesNotImplement,	/* close */
	vName,					/* name */
   }, {												/* ViewClassPart */
	vRefresh,				/* refresh display */
	vHelpAll,				/* update function */
	vHelpLine,				/* update line function */
	vLabel,					/* label */
	vHelpMvLns,				/* move n lines */
	vTextLnUp,				/* goes up a line */
	vTextLnDn,				/* goes down a line */
	vPgUp,					/* goes up a page */
	vPgDn,					/* goes down a page */
	vHelpFirst,				/* first directory */
	vLast,					/* last directory */
	vHelpSet,				/* set tree */
	vHelpReset,				/* reset tree to null */
   },
};

HelpViewRec rHelpView = {
	(Class)	&crHelpView,
   {	
	(WINDOW *) 0L,		/* the window the view is in */
	(WINDOW *) 0L,		/* the window for the box around it */
	"Help",				/* the name */
	0, 0, 0, 0,	0,		/* flag word */
	DIRCOL,				/* window X position */
	DATAROW,			/* window Y position */
	VIEWCOLS,			/* window width */
	VIEWROWS,			/* window height */
	0,					/* cursor row */
	0,					/* cursor column */
	(View) NIL,			/* associated view */
   },
};

HelpViewRec rInitView = {
	(Class)	&crHelpView,
   {	
	(WINDOW *) 0L,		/* the window the view is in */
	(WINDOW *) 0L,		/* the window for the box around it */
	"",					/* the name */
	TRUE, 0, 0, 0, 0,	/* flag word -- is_centered = TRUE */
	DIRCOL,				/* window X position */
	DATAROW,			/* window Y position */
	VIEWCOLS,			/* window width */
	VIEWROWS,			/* window height */
	0,					/* cursor row */
	0,					/* cursor column */
	(View) NIL,			/* associated view */
   },
};


/*********************************************************************\
**
** Global Variables
**
\*********************************************************************/

global TreeView	oDirView    = &rDirView;		/* Idris Directory list */
global TreeView	oDosView    = &rDosView;		/* DOS Directory list */
global TreeView	oFileView   = &rFileView;		/* File list */
global TreeView	oDosFileView= &rDosFileView;	/* File list */
global FileView	oTextView   = &rTextView;		/* File text */
global FileView	oBinaryView = &rBinaryView;		/* File text */
global HelpView	oHelpView   = &rHelpView;		/* Help Text */
global HelpView	oInitView   = &rInitView;		/* Initial Text */
/* global MenuView	oMenuView = &rMenuView;	*/	/* Menus */

global ViewClass clDirView =  &crDirView;
global ViewClass clFileView = &crFileView;
global ViewClass clTextView = &crTextView;
global ViewClass clBinaryView = &crBinaryView;
global ViewClass clHelpViewC = &crHelpView;

/*********************************************************************\
**
** Initialization
**
\*********************************************************************/

global void vInit(imsg)
	char **imsg;
{
	extern WINDOW *MenuWindow;

	/* === At this point we ought to extract screen parameters from stdscr */

	/*
	** Initialize the views.
	*/
	gInit(oDirView);
	gInit(oDosView);
	gInit(oFileView);
	gInit(oDosFileView);
	gInit(oTextView);
	gInit(oHelpView);
	gInit(oInitView);

	oBinaryView -> view.win = oTextView -> view.win;
	oBinaryView -> view.box = oTextView -> view.box;

	gVuSet (oInitView, imsg);

	MenuWindow = newwin(2, swidth - 4, menuRow, 2);
	hline (stdscr, 0, swidth, D_UL, D_HORIZ, D_UR);
	vlines (stdscr, 1, swidth, headerRow - 1, D_VERT);
}

