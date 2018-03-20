/*** headerID = "view.h 2.0 (c) 1987 S. Savitzky ***/

/*********************************************************************\
**
**	View -- 	Header file for Data View utilities
**
**		Most of the globals are user-settable parameters, and
**		are initialized in view.c
**
**	890814 SS	more major changes: view/viewer split
**	880422 SS	major changes:  view data structure
**	880111 SS	create PC version from DD
**
\*********************************************************************/
/*********************************************************************\
**
** View Data Structure		displays data in an area of the screen
**
**		Prefix vu
**
**		Eventually we may get around to making View a subclass
**		of Tree, in which case the box field goes away.
**
**		data_rows non-zero implies that the actual data is
**		centered vertically.
**
\*********************************************************************/

typedef struct _ViewRec *View;
typedef struct _ViewClassRec *ViewClass;

typedef struct _ViewPart {
	struct 	WINDOW_	*win;		/* the window the view is in */
	struct 	WINDOW_	*box;		/* the window for the box around it */
	String		name;			/* the view's name */
	Bool		is_centered: 1;	/* text is centered horizontally */
	Bool		is_active:   1;	/* contains highlight */
	Bool		is_visible:  1;	/* view is on the screen */
	Bool		do_update:   1;	/* update needed before refresh */
	Bool		do_refresh:  1;	/* refresh needed */
	short		org_x, org_y;	/* window position on screen */
	short		cols;			/* window width */
	short		rows;			/* window height */
	short		data_rows;		/* # rows that actually show data */
	View		detail;			/* -> a view showing detail of this view */
	/* the following are initialized at run time */
	short		x;				/* cursor column */
	short		y;				/* cursor row */
	Viewer		root;			/* the object we're viewing */
	Viewer		top;			/* the sub-object at the top of the screen */
	Viewer		cur;			/* the sub-object being highlighted */
	Viewer		tmp;			/* sweep down the screen for updating */
} ViewPart;

typedef struct _ViewRec {
	ObjectPart 	object;
	ViewPart	view;
} ViewRec;

/*********************************************************************\
**
** View Class.
**
\*********************************************************************/

typedef struct _ViewClassPart {
	VoidFunc	set;			/* (v, Viewer) set new root viewer */
	VoidFunc	ref_win;		/* refresh display */
	VoidFunc	upd_all;		/* mark display for update */
	VoidFunc	upd_line;		/* mark current line for update */
	VoidFunc	label;			/* (string, geom) display label */
	VoidFunc	mv_lns;			/* (v, n) move down n lines, no update! */

	/* These automatically update if necessary */
	VoidFunc	ln_up;			/* up a line */
	VoidFunc	ln_dn;			/* down a line */
	VoidFunc	pg_up;			/* up a page */
	VoidFunc	pg_dn;			/* down a page */
	VoidFunc	first;			/* to beginning */
	VoidFunc	last;			/* to end */
} ViewClassPart;

typedef struct _ViewClassRec {
	ObjectPart		object;
	ObjectClassPart objectClass;
	ViewClassPart	viewClass;
} ViewClassRec;

/* Operations */

#define gVuRefresh(v)		METH0((v), ViewClass, viewClass.ref_win)
#define gVuUpdate(v)		METH0((v), ViewClass, viewClass.upd_all)
#define gVuUpdLine(v)		METH0((v), ViewClass, viewClass.upd_line)
#define gVuLabel(v,s,g)		METH2((v), ViewClass, viewClass.label, (s), (g))
#define gVuMvLns(v,n)		METH1((v), ViewClass, viewClass.mv_lns, (n))
#define gVuLnUp(v)			METH0((v), ViewClass, viewClass.ln_up)
#define gVuLnDn(v)			METH0((v), ViewClass, viewClass.ln_dn)
#define gVuPgUp(v)			METH0((v), ViewClass, viewClass.pg_up)
#define gVuPgDn(v)			METH0((v), ViewClass, viewClass.pg_dn)
#define gVuFirst(v)			METH0((v), ViewClass, viewClass.first)
#define gVuLast(v)			METH0((v), ViewClass, viewClass.last)
#define gVuSet(v,vr)		METH1((v), ViewClass, viewClass.set, (vr))

/*********************************************************************\
**
** View Classes
**
**		LineView:	contains a highlighted line
**		PageView:	the entire page scrolls
**
\*********************************************************************/

global ViewClass clLineView;
global ViewClass clPageView;


/*********************************************************************\
**
** Functions
**
\*********************************************************************/


global void viewInit();				/* Initialize view module		*/



