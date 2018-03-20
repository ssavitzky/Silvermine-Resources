/*** headerID = "view.h 1.0 (c) 1987 S. Savitzky ***/

/*********************************************************************\
**
**	View -- 	Header file for Data View utilities
**
**	=== This needs to get split into generic view stuff, and
**	=== RD7's specific views, but it's a little chaotic yet.
**
**		Most of the globals are user-settable parameters, and
**		are initialized in view.c
**
**	880422 SS	major changes:  view data structure
**	880111 SS	create PC version from DD
**
\*********************************************************************/

/*********************************************************************\
**
** View Data Structure
**
**		Eventually we may get around to making View a subclass
**		of Tree, in which case the box field goes away.
**
\*********************************************************************/

typedef struct _ViewRec *View;
typedef struct _ViewClassRec *ViewClass;
global	Class	clView;

typedef struct _ViewPart {
	struct 	WINDOW_	*win;		/* the window the view is in */
	struct 	WINDOW_	*box;		/* the window for the box around it */
	String		name;			/* the view's name */
	unsigned	is_centered: 1;	/* text is centered */
	unsigned	is_active:   1;	/* contains highlight */
	unsigned	is_visible:  1;	/* view is on the screen */
	unsigned	do_update:   1;	/* update needed before refresh */
	unsigned	do_refresh:  1;	/* refresh needed */
	ushort		x, y;			/* window position on screen */
	ushort		cols;			/* window width */
	ushort		rows;			/* window height */
	ushort		cur_row;		/* cursor row */
	ushort		cur_col;		/* cursor column */
	View		partner;		/* parallel (files/dirs) view */
} ViewPart;

typedef struct _TreeViewPart {
	Tree		root;			/* the root of the tree being viewed */
	Tree		top;			/* top line on screen */
	Tree		cur;			/* node containing the cursor */
} TreeViewPart;

typedef struct _FileViewPart {
	char   huge	*origin;		/* origin of text */
	long		limit;			/* size of text */
	long		top;			/* index of start of top line */
	long		cur;			/* index of start of current line */
} FileViewPart;

typedef struct _HelpViewPart {
	char		**origin;		/* start of array of text pointers */
	long		limit;			/* index of last line */
	long		top;			/* index of top line */
	long		cur;			/* index of current line */
} HelpViewPart;



typedef struct _ViewRec {
	ObjectPart 	object;
	ViewPart	view;
} ViewRec;

typedef struct _TreeViewRec {
	ObjectPart 	object;
	ViewPart	view;
	TreeViewPart treeview;
} TreeViewRec, *TreeView;

typedef struct _FileViewRec {
	ObjectPart 	object;
	ViewPart	view;
	FileViewPart fileview;
} FileViewRec, *FileView;

typedef struct _HelpViewRec {
	ObjectPart 	object;
	ViewPart	view;
	HelpViewPart helpview;
} HelpViewRec, *HelpView;


/*********************************************************************\
**
** View Class.
**
**		It is expected that views will be invisibly subclassed,
**		using the class operations to express the differences between
**		the various flavors of view.
**
\*********************************************************************/

typedef struct _ViewClassPart {
	VoidFunc	ref_win;		/* refresh display */
	VoidFunc	upd_all;		/* update display */
	VoidFunc	upd_line;		/* update line */
	VoidFunc	label;			/* (string, geom) display label */
	VoidFunc	mv_lns;			/* (v, nlines) move by nlines */
	VoidFunc	ln_up;			/* up a line */
	VoidFunc	ln_dn;			/* down a line */
	VoidFunc	pg_up;			/* up a page */
	VoidFunc	pg_dn;			/* down a page */
	VoidFunc	first;			/* to beginning */
	VoidFunc	last;			/* to end */
	VoidFunc	set;			/* (v, tree) set new tree */
	VoidFunc	reset;			/* clear current tree to null */
} ViewClassPart;

typedef struct _ViewClassRec {
	ObjectPart		object;
	ObjectClassPart objectClass;
	ViewClassPart	viewClass;
} ViewClassRec;

/* Operations */

#define gVuRefresh(v)		METH0((v), ViewClass, viewClass.ref_win)
#define gVuUpdAll(v)		METH0((v), ViewClass, viewClass.upd_all)
#define gVuUpdLine(v)		METH0((v), ViewClass, viewClass.upd_line)
#define gVuLabel(v,s,g)		METH2((v), ViewClass, viewClass.label, (s), (g))
#define gVuMvLns(v,n)		METH1((v), ViewClass, viewClass.mv_lns, (n))
#define gVuLnUp(v)			METH0((v), ViewClass, viewClass.ln_up)
#define gVuLnDn(v)			METH0((v), ViewClass, viewClass.ln_dn)
#define gVuPgUp(v)			METH0((v), ViewClass, viewClass.pg_up)
#define gVuPgDn(v)			METH0((v), ViewClass, viewClass.pg_dn)
#define gVuFirst(v)			METH0((v), ViewClass, viewClass.first)
#define gVuLast(v)			METH0((v), ViewClass, viewClass.last)
#define gVuSet(v, t)		METH1((v), ViewClass, viewClass.set, (t))
#define gVuReset(v)			METH0((v), ViewClass, viewClass.reset)


/*********************************************************************\
**
** Views.  === Eventually these ought to go into a separate file. ===
**
\*********************************************************************/

global TreeView	oDirView;				/* Idris Directory list */
global TreeView oDosView;				/* DOS Directory list */
global ViewClass clDirView;

global TreeView	oFileView;				/* Idris File list */
global TreeView	oDosFileView;			/* Dos File list */
global ViewClass clFileView;

global FileView	oTextView;				/* File text */
global ViewClass clTextView;

global FileView oBinaryView;			/* File contents in hex */
global ViewClass clBinaryView;

global HelpView	oHelpView;				/* Help Text */
global HelpView	oInitView;				/* Initial Text */
global ViewClass clHelpView;

global View	oMenuView;				/* Menus */
global ViewClass clMenuView;


/*********************************************************************\
**
** Functions
**
\*********************************************************************/


global void viewInit();				/* Initialize view module		*/



