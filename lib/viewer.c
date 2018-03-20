/*********************************************************************\
**
** Viewer.c	--	Traverse data structures for viewing
**
**	890819 SS	split View and Viewer structures
**	880422 SS	major changes:  view data structure
**	880111 SS	create PC version from DD
**
\*********************************************************************/

#include <stdio.h>
#include "ibmchars.h"

#include "coops.h"
#include "tree.h"
#include "dir.h"

#undef  global
#define global

#include "viewer.h"


extern Object objInit(), objClone(), objKill(), objDoesNotImplement();
extern String objName();

/*
** Things useful for formatting
*/
int vrTabStop = 8;

static char buf[81];


/*********************************************************************\
**
** V I E W E R S
**
\*********************************************************************/

/*
** Generic maker.  Works if Set is sufficient for setup.
*/
global Object vrNew(cl, x)
	VrClass cl;
	Opaque x;
{
	Viewer v;

	v = (Viewer) objCalloc(cl, 0);
	gVrSet(v, x);
}


/*********************************************************************\
**
** Tree Viewer
**
**		Displays an indented directory tree.
**
\*********************************************************************/

static Opaque vrTreeGet(v)
	TreeVr v;
{
	return ((Opaque) v -> treeVr.root);
}

static void vrTreeSet(v, x)
	TreeVr v;
	Tree x;
{
	for (v -> treeVr.root = x; 
		 NOTNULL(x) && NOTNULL(gUp(v -> treeVr.root)); 
		 v -> treeVr.root = gUp(v -> treeVr.root)) ;
/*
	if (NOTNULL(x)) x = gDown(x);
	if (v -> treeVr.no_leaves)
		for ( ; NOTNULL(x) && !ISDIR(x); x = gSucc(x)) ;
*/
	v -> treeVr.cur = x;
}

static void vrTreeCopy(v, src)
	TreeVr v;
	TreeVr src;
{
	*v = *src;
}

static void vrTreeRewind(v)
	TreeVr v;
{
	gVrSet(v, v -> treeVr.root);
}

static Bool vrTreeNext(v)
	TreeVr v;
{
	Tree d = v -> treeVr.cur;

	if (!d) return (FALSE);
	d = gSucc(d);
	if (v -> treeVr.no_leaves)
		for ( ; NOTNULL(d) && !ISDIR(d); d = gSucc(d)) ;

	if (!d) return (FALSE);
	v -> treeVr.cur = d;
	return (TRUE);
}
								  
static Bool vrTreePrev(v)
	TreeVr v;
{
	Tree d = v -> treeVr.cur;

	if (!d) return (FALSE);
	d = gPred(d);
	if (v -> treeVr.no_leaves)
		for ( ; NOTNULL(d) && !ISDIR(d); d = gPred(d)) ;

	if (!d) return (FALSE);
	v -> treeVr.cur = d;
	return (TRUE);
}

/*
** We need a bunch of random utilities for indenting trees
*/
static Tree nextSib(d, v)
	Tree d;
	TreeVr v;
{
	d = gNext(d);
	if (!v -> treeVr.no_leaves) return (d);
	for ( ; NOTNULL(d) && !ISDIR(d); d = gNext(d)) ;
	return (d);
}

static Tree nextKid(d, v)
	Tree d;
	TreeVr v;
{
	d = gDown(d);
	if (!v -> treeVr.no_leaves) return (d);
	for ( ; NOTNULL(d) && !ISDIR(d); d = gNext(d)) ;
	return (d);
}

static void vIndentDir(str, node, v)
	String str;
	Tree node;
	TreeVr v;
{
	Tree p;

	if (ISNULL(node)) return;
	if (NOTNULL(p = gUp(node)))    vIndentDir(str, p);
	if (NOTNULL(nextSib(node, v))) strcat(str, "\263 ");
	else           		   		   strcat(str, "  ");
}

static String vrTreeString(v, w)
	TreeVr v;
	int w;
{
	register Tree p;
	register Tree node = v -> treeVr.cur;

	buf[0] = 0;
	if (ISNULL(node)) return ("<Nothing read>");
	vIndentDir(buf, (p = gUp(node)), v);

	/*
	** Decide on L vs. T for each end of the branch line
	*/
	if (NOTNULL(nextSib(node, v)))	strcat(buf, "\303\304");
	else if (NOTNULL(p))			strcat(buf, "\300\304");
	else							strcat(buf, "  ");
	if (NOTNULL(nextKid(node, v)))	strcat(buf, "\302");
	else							strcat(buf, "\304");

	if (NOTNULL(node)) {
		gHeader(node, buf + strlen(buf), (Cardinal) w - 1);
	}
	return (buf + 1);		/* +1 compensates for extra space on left */
}


VrClassRec crTreeVr = {
	&crClass,				/* class */				/* Object Part */
   {												/* ObjectClass Part */
	sizeof (TreeVrRec),		/* instance size */
	"TreeViewer",			/* class name */
	&crObject,				/* super */
	NIL,					/* method dict */
	objInit,				/* initialize */
	vrNew,					/* new object */
	objClone,				/* clone self */
	objKill,				/* kill self */
	objDoesNotImplement,	/* open */
	objDoesNotImplement,	/* close */
	objName,					/* name */
   }, {												/* ViewClassPart */
	vrTreeGet,				/* (v) -> current object */
	vrTreeSet,				/* (v, ...) re-direct */
	vrTreeCopy,				/* (v, src) -> v   copy src into v */
	vrTreeRewind,			/* (v) go back to beginning */
	vrTreeNext,				/* (v) -> success  advance one line */
	vrTreePrev,				/* (v) -> success  retreat one line */
	vrTreeString,			/* (v, width) -> string to display */
   },
};
global VrClass clTreeVr = &crTreeVr;


/*********************************************************************\
**
** Leaf Viewer
**
**		This shows a single, unindented level in a tree.
**		The root that we set is the parent of the level.
**
\*********************************************************************/

static void vrLeafSet(v, x)
	TreeVr v;
	Tree x;
{
	v -> treeVr.root = x;
	if (NOTNULL(x)) x = gDown(x);
	if (v -> treeVr.no_branches)
		for ( ; NOTNULL(x) && ISDIR(x); x = gNext(x)) ;
	v -> treeVr.cur = x;
}

static Bool vrLeafNext(v)
	TreeVr v;
{
	Tree d = v -> treeVr.cur;

	if (!d) return (FALSE);
	d = gNext(d);
	if (v -> treeVr.no_branches)
		for ( ; NOTNULL(d) && ISDIR(d); d = gNext(d)) ;

	if (!d) return (FALSE);
	v -> treeVr.cur = d;
	return (TRUE);
}
								  
static Bool vrLeafPrev(v)
	TreeVr v;
{
	Tree d = v -> treeVr.cur;

	if (!d) return (FALSE);
	d = gPrev(d);
	if (v -> treeVr.no_branches)
		for ( ; NOTNULL(d) && ISDIR(d); d = gPrev(d)) ;

	if (!d) return (FALSE);
	v -> treeVr.cur = d;
	return (TRUE);
}

static String vrLeafString(v, w)
	TreeVr v;
	int w;
{
	register Tree node = v -> treeVr.cur;

	buf[0] = 0;
	if (ISNULL(node)) return ("<Empty>");
	gHeader(node, buf, (Cardinal) w - 1);
	return (buf);
}


VrClassRec crLeafVr = {
	&crClass,				/* class */				/* Object Part */
   {												/* ObjectClass Part */
	sizeof (TreeVrRec),		/* instance size */
	"LeafViewer",			/* class name */
	&crObject,				/* super */
	NIL,					/* method dict */
	objInit,				/* initialize */
	vrNew,					/* new object */
	objClone,				/* clone self */
	objKill,				/* kill self */
	objDoesNotImplement,	/* open */
	objDoesNotImplement,	/* close */
	objName,					/* name */
   }, {												/* ViewClassPart */
	vrTreeGet,				/* (v) -> current object */
	vrLeafSet,				/* (v, ...) re-direct */
	vrTreeCopy,				/* (v, src) -> v   copy src into v */
	vrTreeRewind,			/* (v) go back to beginning */
	vrLeafNext,				/* (v) -> success  advance one line */
	vrLeafPrev,				/* (v) -> success  retreat one line */
	vrLeafString,			/* (v, width) -> string to display */
   },
};
global VrClass clLeafVr = &crLeafVr;


/*********************************************************************\
**
** Formatters for hex and ascii
**
\*********************************************************************/

/* 
** Internal routine to format a line of hex.
*/
global void hexStr(b, p, lim, adr)
	char *b;			 	/* destination buffer */
	char *p;			/* source data */
	long  lim;				/* max # of bytes to do */
	long  adr;				/* starting address of row */
{
	register int c;
	static char hexdigits[16] = "0123456789abcdef";

	sprintf(b, "%8lx: ", adr);
	b += 10;
	for (c = 0; c < 16; ++c) {
		register ushort d = p[c] & 255;
		if (c < lim) {
			*b++ = hexdigits[d >> 4];
			*b++ = hexdigits[d & 15];
		} else {
			*b++ = ' ';
			*b++ = ' ';
		}
		*b++ = ' ';
	}
	*b++ = ' ';
	for (c = 0; c < 16; ++c) {
		register int cc = p[c] & 255;
		if (c < lim) {
			*b++ = cc >= ' ' && cc < 0x7f ? cc : '.';
		} else {
			*b++ = ' ';
		}
	}
	*b = 0;
}

/* 
** Internal routine to format a line of text (with tabs).
**		designed so it can be used to continue a partial line
**		that is split across more than one buffer.
**
**		Returns the next source char; next column in textCol.
**		Counts columns for tabs, but lets waddch expand them.
*/
global int textCol;
global char *textStr(b, p, lim, c, maxx)
	char *b;			 	/* destination buffer */
	char *p;				/* source text */
	char *lim;				/* limit (0 for null-terminated) */
	int	  c;				/* starting column */
	int   maxx;				/* max. column */
{
	int d;

	if (!p) { *b = 0; return (p); }
	for ( ; *p != '\n' && *p != '\r' && ((!lim && *p) || p < lim); ++p, ++c) {
		if (*p == '\t') c = (c + vrTabStop) % vrTabStop;
		if (c < maxx && *p != '\r') *b++ = *p; 
	}
	*b = 0;
	textCol = c;
	return (p);
}


/*********************************************************************\
**
** Text Buffer Viewer
**
**		Simple viewer for a purely internal buffer.
**		Is the superclass for the file viewer.
**		Includes hex formatting as an option.
**
\*********************************************************************/

static Opaque vrTextGet(v)
	TextVr v;
{
	return ((Opaque) v -> textVr.buf);
}

static void vrTextSet(v, x)
	TextVr v;
	char *x;
{
	v -> textVr.buf = x;
	v -> textVr.cur = 0;
	v -> textVr.lim = 0;
	/* The size will get fixed up by the caller if necessary */
	/* Zero means the buffer is null-terminated */
}

static void vrTextCopy(v, o)
	TextVr v, o;
{
	*v = *o;
}

static void vrTextRewind(v)
	TextVr v;
{
	v -> textVr.cur = 0;
}

static Bool vrTextNext(v)
	TextVr v;
{
	register long lim = v -> textVr.lim;
	register long   i = v -> textVr.cur;
	register char  *b = v -> textVr.buf;

	if (!b) return (FALSE);
	if (lim == 0) lim = strlen(b);

	if (v -> textVr.hex) {
		if (i + 16 < lim) {
			v -> textVr.cur += 16;
		} else {
			return (FALSE);
		}
	} else {
		for ( ; i < lim; ++i) {
			if (b[i] == '\n') {
				v -> textVr.cur = i + 1;
				return (TRUE);
			}
		}
		return (FALSE);
	}
	return (TRUE);
}
								  
static Bool vrTextPrev(v)
	TextVr v;
{
	register long i = v -> textVr.cur;
	char *b = v -> textVr.buf;

	if (!b) return (FALSE);

	if (v -> textVr.hex) {
		if (i >= 16) {
			v -> textVr.cur -= 16;
		} else {
			v -> textVr.cur = 0;
			return (FALSE);
		}
	} else {
		for ( ; i >= 0 && b[i] != '\n'; --i) ;
		if (i < 0) {
			v -> textVr.cur = 0;
			return (FALSE);
		}
		for (--i; i >= 0 && b[i] != '\n'; --i) 
		v -> textVr.cur = i /* + 1 */;
	}
	return (TRUE);
}

static String vrTextString(v, w)
	TextVr v;
	int w;
{
	long lim = v -> textVr.lim;
	long off = v -> textVr.cur;
	register char  *p = v -> textVr.buf;

	buf[0] = 0;
	if (!p || lim && off >= lim) {
		; /* Nothing to do */
	} if (v -> textVr.hex) {
		hexStr(buf, p + off, lim - off, (long)off);
	} else {
		textStr(buf, p + off, (lim)? p + lim : (char*)NULL, 0, w);
	}
	return (buf);
}


VrClassRec crTextVr = {
	&crClass,				/* class */				/* Object Part */
   {												/* ObjectClass Part */
	sizeof (TextVrRec),		/* instance size */
	"TextViewer",			/* class name */
	&crObject,				/* super */
	NIL,					/* method dict */
	objInit,				/* initialize */
	vrNew,					/* new object */
	objClone,				/* clone self */
	objKill,				/* kill self */
	objDoesNotImplement,	/* open */
	objDoesNotImplement,	/* close */
	objName,					/* name */
   }, {												/* ViewClassPart */
	vrTextGet,				/* (v) -> current object */
	vrTextSet,				/* (v, ...) re-direct */
	vrTextCopy,				/* (v, src) -> v   copy src into v */
	vrTextRewind,			/* (v) go back to beginning */
	vrTextNext,				/* (v) -> success  advance one line */
	vrTextPrev,				/* (v) -> success  retreat one line */
	vrTextString,			/* (v, width) -> string to display */
   },
};
global VrClass clTextVr = &crTextVr;


/*********************************************************************\
**
** Help	Viewer
**
\*********************************************************************/

static Opaque vrHelpGet(v)
	HelpVr v;
{
	return ((Opaque) v -> helpVr.vec);
}

static void vrHelpSet(v, x)
	HelpVr v;
	String *x;
{
	register int i;

	v -> helpVr.vec = x;
	for (i = 0; x[i]; ++i) ;
	v -> helpVr.lim = i;
	v -> helpVr.cur = 0;
}

static void vrHelpCopy(v, o)
	HelpVr v, o;
{
	*v = *o;
}

static void vrHelpRewind(v)
	HelpVr v;
{
	v -> helpVr.cur = 0;
}

static Bool vrHelpNext(v)
	HelpVr v;
{
	if (v -> helpVr.cur >= v -> helpVr.lim)	return (FALSE);
	++ v -> helpVr.cur;
	return (TRUE);
}
								  
static Bool vrHelpPrev(v)
	HelpVr v;
{
	if (v -> helpVr.cur <= 0)	return (FALSE);
	-- v -> helpVr.cur;
	return (TRUE);
}

static String vrHelpString(v, w)
	HelpVr v;
	int w;
{
	textStr(buf, v -> helpVr.vec[v -> helpVr.cur], (char*)NULL, 0, w);
	return (buf);
}


VrClassRec crHelpVr = {
	&crClass,				/* class */				/* Object Part */
   {												/* ObjectClass Part */
	sizeof (HelpVrRec),		/* instance size */
	"HelpViewer",			/* class name */
	&crObject,				/* super */
	NIL,					/* method dict */
	objInit,				/* initialize */
	vrNew,					/* new object */
	objClone,				/* clone self */
	objKill,				/* kill self */
	objDoesNotImplement,	/* open */
	objDoesNotImplement,	/* close */
	objName,					/* name */
   }, {												/* ViewClassPart */
	vrHelpGet,				/* (v) -> current object */
	vrHelpSet,				/* (v, ...) re-direct */
	vrHelpCopy,				/* (v, src) -> v   copy src into v */
	vrHelpRewind,			/* (v) go back to beginning */
	vrHelpNext,				/* (v) -> success  advance one line */
	vrHelpPrev,				/* (v) -> success  retreat one line */
	vrHelpString,			/* (v, width) -> string to display */
   },
};
global VrClass clHelpVr = &crHelpVr;


