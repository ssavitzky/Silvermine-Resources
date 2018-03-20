/*** headerID = "viewer.h 1.0 (c) 1989 S. Savitzky ***/

/*********************************************************************\
**
**	Viewer -- 	Header file for Data Viewers
**
**	890814 SS	more major changes: view/viewer split
**	880422 SS	major changes:  view data structure
**	880111 SS	create PC version from DD
**
\*********************************************************************/

/*********************************************************************\
**
** Viewer Data Structure	sequences through a data structure for 
**							viewing.  Formats the data appropriately.
**
**		Prefix vr
**
**		Viewer is an abstract class; all of the interesting things
**		happen in the specialized subclasses.
**
\*********************************************************************/

typedef Object Viewer;

/*********************************************************************\
**
** Specialized viewers
**
\*********************************************************************/

typedef struct _TreeVrPart {
	Tree		root;			/* relative root */
	Tree		cur;			/* current node */
	Bool		no_leaves:1;	/* skip leaf nodes (TreeViewer) */
	Bool		no_branches:1;	/* skip branch nodes (LeafViewer) */
} TreeVrPart;

typedef struct _TextVrPart {
	char  huge *buf;			/* the buffer */
	long		cur;			/* index of start of current line */
	long		lim;			/* size of text */
	Bool		hex:1;			/* display buffer in hex */
} TextVrPart;

typedef struct _HelpVrPart {
	char	  **vec;			/* start of vector of text pointers */
	int			cur;			/* index of current line */
	int			lim;			/* index of last line */
} HelpVrPart;

typedef struct _TreeVrRec {
	ObjectPart 	object;
	TreeVrPart 	treeVr;
} TreeVrRec, *TreeVr;

typedef struct _TextVrRec {
	ObjectPart 	object;
	TextVrPart 	textVr;
} TextVrRec, *TextVr;

typedef struct _HelpVrRec {
	ObjectPart 	object;
	HelpVrPart 	helpVr;
} HelpVrRec, *HelpVr;


/*********************************************************************\
**
** Viewer Class	(VrClass) and operations
**
\*********************************************************************/

typedef struct _VrClassPart {
	OpaqueFunc	m_get;			/* (v) -> current object */
	VoidFunc	m_set;			/* (v, ...) re-direct */
	VoidFunc	m_copy;			/* (v, src)   copy src into v */
	VoidFunc	m_rewind;		/* (v) go back to beginning */
	BoolFunc	m_next;			/* (v) -> success  advance one line */
	BoolFunc	m_prev;			/* (v) -> success  retreat one line */
	StringFunc	m_string;		/* (v, width) -> string to display */
} VrClassPart;

typedef struct _VrClassRec {
	ObjectPart		object;
	ObjectClassPart	objectClass;
	VrClassPart		vrClass;
} VrClassRec, *VrClass;

#define gVrGet(v)			METH0((v), VrClass, vrClass.m_get)
#define gVrSet(v,x)			METH1((v), VrClass, vrClass.m_set, (x))
#define gVrCopy(v,s)		METH1((v), VrClass, vrClass.m_copy, (s))
#define gVrRewind(v)		METH0((v), VrClass, vrClass.m_rewind)
#define gVrNext(v)			METH0((v), VrClass, vrClass.m_next)
#define gVrPrev(v)			METH0((v), VrClass, vrClass.m_prev)
#define gVrString(v,w)		METH1((v), VrClass, vrClass.m_string, (w))


/*********************************************************************\
**
** Viewer Classes
**
\*********************************************************************/

global VrClass clTreeViewer;	/* tree of Dir nodes */
global VrClass clLeafViewer;	/* list of non-directory Dir nodes */
global VrClass clFileViewer;	/* contents of file */
global VrClass clTextViewer;	/* simple block of text */
global VrClass clHelpViewer;	/* menu help */

global int vrTabStop;			/* standard tab stop interval */


