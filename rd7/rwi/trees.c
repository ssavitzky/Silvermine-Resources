/***/static char *moduleID="trees 1.0 (c)1988 S. Savitzky";/***/

/*********************************************************************\
**
**	COOPS Tree Class
**
**		Entities in this file are for the most part global, to
**		facilitate static initialization of class objects.  This
**		may eventually change.
**
**	880224 SS	create
**
\*********************************************************************/

#include <stdio.h>
#include <string.h>
#include "coops.h"
#include "trees.h"

#undef  global
#define global

extern int errorPrintf();
extern Object objDoesNotImplement();
extern Object objInit(), objClone(), objKill(), objNew();


/*********************************************************************\
**
** Methods for Simple Trees
**
**	NOTE:  these are written assuming that the links fields are valid.
**		   This is not always true for all subclasses of Tree,
**		   i.e. those that calculate their links.
**
\*********************************************************************/

Object treeKill (t)			/* Kill */
	Tree t;
{
	Tree p, q;

	/* === Maybe need to cut first.  Assume it's a root. === */
	for (p = t -> tree.down; NOTNULL(p); p = q) {
		q = p -> tree.next;
		gKill(p);
	}
	return (objKill(t));
}

Tree treeNext (t)			/* Next */
	Tree t;
{
	return (t -> tree.next);
}

Tree treePrev (t)			/* Prev */
	Tree t;
{
	return (t -> tree.prev);
}

Tree treeDown (t)			/* Down */
	Tree t;
{
	return (t -> tree.down);
}

Tree treeUp (t)				/* Up */
	Tree t;
{
	return (t -> tree.up);
}


Tree treeFirst (t)			/* first sibling */
	Tree t;
{
	for ( ; t -> tree.prev; t = t -> tree.prev)	;
	return (t);
}

Tree treeLast (t)			/* last sibling */
	Tree t;
{
	for ( ; t -> tree.next; t = t -> tree.next)	;
	return (t);
}

Tree treeSucc (t)			/* pre-order successor */
	Tree t;
{
	Tree p;

	if (ISNULL(t)) return(t);			/* nil has no successor */
	if (p = t -> tree.down) {			/* go down if possible */
		return(p);
	} else if (!ISNULL(p = t -> tree.next)) {	/* else go to next sib, if any */
		return(p);
	} else {							/* else try parent */
		for (p = t -> tree.up; 
			 NOTNULL(p) && ISNULL(p -> tree.next); 
			 p = p -> tree.up) ;
		return (ISNULL(p) ? p : p -> tree.next);
	}
}

Tree treePred (t)			/* pre-order predecessor */
	Tree t;
{
	Tree p;

	if (ISNULL(t)) return (t);			/* nil has no predecessors */
	if (!ISNULL(p = t -> tree.prev)) {	/* visit predecessor if any */
		for ( ;							/* and find its last descendent */ 
			 !ISNULL(p -> tree.down); 
			 p = treeLast(p -> tree.down)) ;
		return(p);
	} else {							/* else parent if any */
		return(t -> tree.up);
	}
}

Tree treeAfter (t, n)		/* insert n after t */
	Tree t, n;
{
	if (NOTNULL(n -> tree.next = t -> tree.next)) 
		n -> tree.next -> tree.prev = n;
	t -> tree.next = n;
	n -> tree.prev = t;
	n -> tree.up   = t -> tree.up;
	return (n);
}

Tree treeBefore (t, n)		/* insert n before t */
	Tree t, n;
{
	if (ISNULL(n -> tree.prev = t -> tree.prev))
		t -> tree.up -> tree.down = n;
	t -> tree.prev = n;
	n -> tree.next = t;
	n -> tree.up   = t -> tree.up;
	return (n);
}

Tree treeFront (t, n)		/* insert n at front of t's subs */
	Tree t, n;
{
	if (!ISNULL(n -> tree.next = t -> tree.down))
		t -> tree.down -> tree.prev = n;
	n -> tree.prev = (Tree) NIL;
	n -> tree.up = t;
	t -> tree.down = n;
	return (n);
}

Tree treeBack (t, n)			/* insert n at back of t's subs */
	Tree t, n;
{
	if (ISNULL(t -> tree.down)) return (treeFront(t, n));
	else return (treeAfter(treeLast(t -> tree.down), n));
}

Tree treeCut (t)				/* cut t from its tree */
	Tree t;
{
	if (t -> tree.prev)
		t -> tree.prev -> tree.next = t -> tree.next;
	else
		t -> tree.up -> tree.down = t -> tree.next;
	if (t -> tree.next)
		t -> tree.next -> tree.prev = t -> tree.prev;
	t -> tree.up = t -> tree.next = t -> tree.prev = (Tree) NIL;

	return (t);
}

String treeName (t)
	Tree t;
{
	return ("aTree");
}

String treeHeader (t, buf, len)
	Tree t;
	String buf;
	Card16 len;
{
	strncpy (buf, gName (t), len);
	return (buf);
}


String treePath (t, r)
	Tree t;
	Tree r;
{
	register char *p;
	static char buf[1024];

	if (r == t -> tree.up) {
		strcpy(buf, gName(t));
	} else {
		/* This code relies on knowing that buf is static! */
		(void) treePath(t -> tree.up, r);
		p = buf + strlen(buf);
		*p++ = gPathSep(t);
		strcpy(p, gName(t));
	}
	return(buf);
}

Tree treeFind (t, s)
	Tree t;
	String s;
{
	register char *p;
	register int n;
	register char sep = gPathSep(t);

	if (*s == sep) {
		++s;
	}
	/* === ought to open t if necessary.  Ought to recurse. === */
	/* === strictly speaking, we could check t's class   	=== */
	/* === and recurse only if it's different from original	=== */
	for (t = gDown(t); NOTNULL(t); t = gDown(t), s = p + 1) {
		p = strchr(s, sep);
		n = (p) ? p - s : strlen(s);
		for ( ; NOTNULL(t) && strncmp(s, gName(t), n); t = gNext(t)) ;
		if (!p || !p[1]) return (t);
	}
	return (t);
}


/*********************************************************************\
**
** Methods for Generic Trees
**
**	NOTE:  these are written to always use NEXT, etc.
**		   This makes them slower, but more general.
**
\*********************************************************************/


/*********************************************************************\
**
** Classes
**
\*********************************************************************/


global TreeClassRec crTree = {
	&crClass,				/* class */				/* Object Part */
   {												/* Class Part */
	sizeof (TreeRec),		/* instance size */
	"Tree",					/* class name */
	&crObject,				/* super */
	NIL,					/* method dict */
	objInit,				/* initialize */
	objNew,					/* new object */
	objClone,				/* clone self */
	objKill,				/* kill self */
	objDoesNotImplement,	/* open */
	objDoesNotImplement,	/* close */
	treeName,				/* name */
   }, {												/* Tree Class Part */
    '/',					/* path sep */
	treeNext,				/* next */
	treePrev,				/* prev */
	treeDown,				/* down */
	treeUp,					/* up */
	treeSucc,				/* succ */
	treePred,				/* pred */
	treeFirst,				/* first */
	treeLast,				/* last */
	treeAfter,				/* after */
	treeBefore,				/* before */
	treeFront,				/* front */
	treeBack,				/* back */
	treeCut,				/* cut */
	treeHeader,				/* header */
	treePath,				/* get path name */
	treeFind,				/* find by path */
   },
};

Class clTree   = (Class)&crTree;


/*********************************************************************\
**
** Class Initialization
**
\*********************************************************************/




