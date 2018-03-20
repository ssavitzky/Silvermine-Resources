/*** headerID = "trees.h 1.0 (c) 1988 S. Savitzky" ***/

/*********************************************************************\
**
**	Trees -- 	Header file for COOPS tree representation
**
**		The following prefixes are used:
**
**			tree		simple tree operations
**			treg		generic tree operations === later ===
**			walk		tree walker operations  === later ===
**
**	880224	SS	create
**
\*********************************************************************/



/*********************************************************************\
**
** Simple Trees
**
**		This is the run-of-the-mill internal representation for trees.
**
**		Though simple, it is not especially compact.
**
\*********************************************************************/

typedef struct _TreeRec *Tree;
typedef Tree	(*TreeFunc)();


typedef struct _TreePart {
	Tree	next, prev, up, down;	
} TreePart;

typedef struct _TreeRec {
	ObjectPart	object;
	TreePart	tree;
} TreeRec;



/*********************************************************************\
**
** Tree Class
**
\*********************************************************************/

typedef struct _TreeClassPart {
	char		path_sep;	/* pathname separator character */
	TreeFunc	m_next;		/* (self)		next sibling */
	TreeFunc	m_prev;		/* (self)		prev. sibling */
	TreeFunc	m_down;		/* (self)		first kid */
	TreeFunc	m_up;		/* (self)		parent */
	TreeFunc	m_succ;		/* (self)		pre-order successor */
	TreeFunc	m_pred;		/* (self)		pre-order predecessor */
	TreeFunc	m_first;	/* (self)		first sib */
	TreeFunc	m_last;		/* (self)		last sib */

	TreeFunc	m_after;	/* (self, new)	insert new sib after */
	TreeFunc	m_before;	/* (self, new)	insert new sib before */
	TreeFunc	m_front;	/* (self, new)	insert new sub at front */
	TreeFunc	m_back;		/* (self, new)	insert new sub at back */
	TreeFunc	m_cut;		/* (self)		cut from tree */

	StringFunc	m_header;	/* (self, buf, len)	header line */
	StringFunc	m_path;		/* (self, root)	pathname rel. to root */
	TreeFunc	m_find;		/* (self, name) find by pathname */
} TreeClassPart;

typedef struct _TreeClassRec {
	ObjectPart		obj;
	ObjectClassPart	objClass;
	TreeClassPart	treeClass;	
} TreeClassRec, *TreeClass;

global Class  	clTree;			/* -> Tree's class */


/*********************************************************************\
**
** Generic Tree Operations
**
**		These are defined as macros that go direct to fields
**		in the class structure.
**
\*********************************************************************/

#define gNext(o)		METH0((o), TreeClass, treeClass.m_next)
#define gPrev(o)		METH0((o), TreeClass, treeClass.m_prev)
#define gUp(o)			METH0((o), TreeClass, treeClass.m_up)
#define gDown(o)		METH0((o), TreeClass, treeClass.m_down)
#define gSucc(o)		METH0((o), TreeClass, treeClass.m_succ)
#define gPred(o)		METH0((o), TreeClass, treeClass.m_pred)
#define gFirst(o) 		METH0((o), TreeClass, treeClass.m_first)
#define gLast(o)		METH0((o), TreeClass, treeClass.m_last)

#define gAfter(o, n) 	METH1((o), TreeClass, treeClass.m_after, (n))
#define gBefore(o, n)	METH1((o), TreeClass, treeClass.m_before, (n))
#define gFront(o, n)	METH1((o), TreeClass, treeClass.m_front, (n))
#define gBack(o, n)		METH1((o), TreeClass, treeClass.m_back, (n))
#define gCut(o)			METH0((o), TreeClass, treeClass.m_cut)

#define gHeader(o,b,l)	METH2((o), TreeClass, treeClass.m_header, (b), (l))
#define gPath(o,r)		METH1((o), TreeClass, treeClass.m_path, (r))
#define gFind(o,s)		METH1((o), TreeClass, treeClass.m_find, (s))

#define gPathSep(t)		(((TreeClass) (t -> object.class)) -> treeClass.path_sep)

#define GetPath(t,buf)	strcpy(buf, gPath(t, NIL))

/*********************************************************************\
**
** Tree Walker
**
**		This is an object that does the bookkeeping for operating on
**		trees indirectly.
**
\*********************************************************************/


/*********************************************************************\
**
** Subclasses of Tree:  Item and Menu
**
**		An Item is a simple tree with a name.
**		A Menu is an Item with an attached action.
**
\*********************************************************************/



