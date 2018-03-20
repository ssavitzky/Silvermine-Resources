/*********************************************************************\
**
** Viewer.c	--	Traverse data structures for viewing
**
**	880819 SS	split View and Viewer structures
**	880422 SS	major changes:  view data structure
**	880111 SS	create PC version from DD
**
\*********************************************************************/

#include <stdio.h>
#include "../lib/ibmchars.h"

#include "coops.h"
#include "trees.h"
#include "dirs.h"

#undef  global
#define global

#include "viewer.h"


extern Object objInit(), objClone(), objKill(), objDoesNotImplement();
extern String objName();

/*
** Things useful for formatting
*/
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
		 NOTNULL(gUp(v -> treeVr.root)); 
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

static void vIndentDir(str, node)
	String str;
	Tree node;
{
	Tree p;

	if (ISNULL(node)) return;
	if (NOTNULL(p = gUp(node))) vIndentDir(str, p);
	if (NOTNULL(nextSib(node))) strcat(str, "\263 ");
	else           		   		strcat(str, "  ");
}

static String vrTreeString(v, w)
	TreeVr v;
	int w;
{
	register Tree p;
	register Tree node = v -> treeVr.cur;

	buf[0] = 0;
	if (ISNULL(node)) return ("<Nothing read>");
	vIndentDir(buf, (p = gUp(node)));

	/*
	** Decide on L vs. T for each end of the branch line
	*/
	if (NOTNULL(nextSib(node)))	strcat(buf, "\303\304");
	else if (NOTNULL(p))		strcat(buf, "\300\304");
	else						strcat(buf, "  ");
	if (NOTNULL(nextKid(node)))	strcat(buf, "\302");
	else						strcat(buf, "\304");

	if (NOTNULL(node)) {
		gHeader(node, buf + strlen(buf), (Cardinal) w - 1);
	}
	return (buf);
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
static void hexStr(b, p, lim, adr)
	char *b;			 	/* destination buffer */
	char huge *p;			/* source data */
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
static int textCol;
static char huge *textStr(b, p, lim, c, maxx)
	char *b;			 	/* destination buffer */
	char huge *p;			/* source text */
	char huge *lim;			/* limit (0 for null-terminated) */
	int	  c;				/* starting column */
	int   maxx;				/* max. column */
{
	int d;

	if (!p) { *b = 0; return (p); }
	for ( ; *p != '\n' && ((!lim && *p) || p < lim); ++p, ++c) {
		if (*p == '\t') c = (c + 8) & ~7;
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
	char huge *x;
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
	register char huge *b = v -> textVr.buf;

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
	char huge *b = v -> textVr.buf;

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
** Buffer Pool for file contents viewing.
**
**		The problem with file viewers is that we need to be able to
**		traverse the file quickly in both directions.  
**
**		We do this by keeping a fixed-size pool of buffers (2K seems
**		like a reasonable size) which we allocate to files and locations 
**		on an as-needed basis.
**
**		Eventually this may be a separate module and support editing 
**		and other frills.
**
** THIS CODE ASSUMES THAT WE WILL NEVER BE USING ALL THE BUFFERS AT ONCE.
**
**		Since there are 30 buffers and only one file view with
**		four active viewers, this seems a reasonable assumption.
**
**		If it becomes possible for active buffers to get thrown out,
**		FileViewer operations will have to check for a valid buffer
**		before every operation.
**
\*********************************************************************/

#ifdef FILE_VIEWER

#define BUFSIZE	2048
#define NBUFS	30

typedef struct _Buffer {
	short		users;			/* use count for the buffer */
	Bool		lru:1,			/* LRU state bit. */
				dirty:1;		/* dirty bit (currently unused) */
	Dir			d;				/* the Dir node for the file */
	long		org;			/* where in the file the buffer starts */
	long		size;			/* actual size of data in the buffer */
	char		data[BUFSIZE];
} Buffer;

Buffer  pool[NBUFS];		 	/* The buffer pool itself */
int		clock = 0;				/* The LRU clock */

/* 
** Increase a buffer's use count
*/
int bufEnter(n)
	int n;
{
	++pool[n].users;
#ifdef DEBUG_FV
errorPrintf("enter buffer %d, %d users", n, pool[n].users);
#endif
	pool[n].lru = 1;
	return (n);
}

/*
** Decrease a buffer's use count
*/
int bufLeave(n)
	int n;
{
#ifdef DEBUG_FV
errorPrintf("leave buffer %d, %d users", n, pool[n].users);
#endif
	--pool[n].users;
	return (n);
}

/*
** Find a suitable buffer to free up.
**		NOTE: we assume that one exists!  See section head.
*/
int bufFree()
{
	int n;

#ifdef DEBUG_FV
errorPrintf("Freeing a buffer");
#endif
	for (n = NBUFS * 2; n--; clock = (clock + 1) % NBUFS)
		if (pool[clock].users) continue;
		else if (pool[clock].lru) pool[clock].lru = 0;
		else return (clock);

	errorPrintf("Not enough buffers to view file; internal bug");
	return (-1);
}

/*
** Find a buffer.  Read from the file if necessary.
**		We assume that the file is already open.
*/
int bufFind(d, org)
	Dir		d;
	long	org;
{
	int i;
	int free = -1;
	Buffer *b;

#ifdef DEBUG_FV
errorPrintf("Finding a buffer");
#endif
	for (i = 0; i < NBUFS; ++i) 
		if (pool[i].d == d && pool[i].org == org) return(bufEnter(i));
		else if (!pool[i].d) free = i;
	/* 
	** Need to read.  Find a suitable free buffer,
	** and fill it from the file
	*/
	if (free == -1) free = bufFree();
	if (free == -1) return(free);

#ifdef DEBUG_FV
errorPrintf("Reading into buffer %d", free);
#endif
	b = &pool[free];
	b -> dirty = 0;
	b -> d     = d;
	b -> org   = org;
	gSeek(d, org);
	b -> size  = gReadFile(d, b -> data, BUFSIZE);

	return (bufEnter(free));
}

/*
** Invalidate buffers belonging to a given Dir node
*/
void bufFlush(d)
	Dir 	d;
{
	int i;

	for (i = 0; i < NBUFS; ++i) 
		if (pool[i].d == d) pool[i].d = (Dir)0;
}


/*********************************************************************\
**
** File Contents Viewer
**
**		An extension of TextVr to go to a file for multiple
**		buffers.  Uses the buffer pool operations above.
**		THE HEX STUFF ASSUMES THE BUFFER SIZE IS A MULTIPLE OF 16.
**
** WARNING!
**		The way the buffer use count stuff operates, it is ESSENTIAL
**		to clear viewers when you're done with them.
**
\*********************************************************************/

static Opaque vrFileGet(v)
	FileVr	v;
{
	return ((Opaque) v -> fileVr.d);
}

static Bool vrFileBuf(v, org)		/* set buffer for given origin */
	FileVr	v;
	long	org;
{
	Buffer *b;
	int bx;

#ifdef DEBUG_FV
errorPrintf("Setting buffer for org %ld", org);
#endif
	if (ISNULL(v -> fileVr.d) || org >= v -> fileVr.d -> dir.size) 
		return(FALSE);
	bx = bufFind(v -> fileVr.d, org);

#ifdef DEBUG_FV
errorPrintf(" --> buffer index %d", bx);
#endif

	if (bx == -1) {
		return (FALSE);
	}
	if (NOTNULL(v -> fileVr.d) && v -> fileVr.bx >= 0) 
		bufLeave(v -> fileVr.bx);
	v -> fileVr.org = org;
	v -> fileVr.bx  = bx;
	b = &pool[bx];
	v -> textVr.buf = b -> data;
	v -> textVr.cur = 0;
	v -> textVr.lim = b -> size;
	return (TRUE);
}

static void vrFileSet(v, d)
	FileVr	v;
	Dir 	d;
{
	Buffer *b;

	if (NOTNULL(v -> fileVr.d) && v -> fileVr.bx >= 0) 
		bufLeave(v -> fileVr.bx);
	v -> fileVr.d  = d;
	v -> fileVr.bx = -1;
	vrFileBuf(v, 0L);
}

static void vrFileCopy(v, o)
	FileVr v, o;
{
	if (NOTNULL(v -> fileVr.d) && v -> fileVr.bx >= 0) 
		bufLeave(v -> fileVr.bx);
	*v = *o;
	if (NOTNULL(v -> fileVr.d) && v -> fileVr.bx >= 0) 
		bufEnter(v -> fileVr.bx);
}

static void vrFileRewind(v)
	FileVr v;
{
	vrFileBuf(v, 0L);
}

/*
** Return current char. and advance to next; return -1 if EOF
*/
static int vrFileNc(v)
	FileVr v;
{
	register long lim = v -> textVr.lim;
	register long   i = v -> textVr.cur;
	register char   c;

	if (i >= lim) return (-1);
	c = v -> textVr.buf[i];
	if (++i >= lim) {
		if (vrFileBuf(v, v -> fileVr.org + lim)) {
			 i = 0;
		}
	}
	v -> textVr.cur = i;
	return (c);
}

/*
** Return current char. and advance to previous
*/
static int vrFilePc(v)
	FileVr v;
{
	register long   i = v -> textVr.cur;
	register char   c;

	c = v -> textVr.buf[i];
	if (i > 0) {
		v -> textVr.cur = i - 1;
	} else if (v -> fileVr.org > 0) {
		vrFileBuf(v, v -> fileVr.org - BUFSIZE);
		v -> textVr.cur = v -> textVr.lim - 1;
	}
	return (c);
}

static Bool vrFileNext(v)
	FileVr v;
{
	register long lim = v -> textVr.lim;
	register long   i = v -> textVr.cur;
	register char  *b = v -> textVr.buf;
	register char   c;
	static Bool vrFilePrev();

	if (!b) return (FALSE);

	if (v -> textVr.hex) {
		if (i + 16 < lim) {
			v -> textVr.cur += 16;
		} else if (v -> fileVr.org + lim >= v -> fileVr.d -> dir.size) {
			return (FALSE);
/*		} else if (!vrFileBuf(v, v -> fileVr.org + lim)) {
			return (FALSE);
*/		} else {
			vrFileBuf(v, v -> fileVr.org + lim);
		}
	} else if (i == lim) {
		return (FALSE);
	} else {
		for ( ; i < lim; ) {
			c = b[i];
			if (++i >= lim) {
				if (vrFileBuf(v, v -> fileVr.org + lim)) {
					i   = 0;
					lim = v -> textVr.lim;
					b   = v -> textVr.buf;
				} else {
					v -> textVr.cur = lim;
/*					vrFilePrev(v); */
					return (TRUE);
				}
			}
			if (c == '\n') {
				if (v -> fileVr.org + i < v -> fileVr.d -> dir.size) {
					v -> textVr.cur = i;
					return (TRUE);
				} else {
					v -> textVr.cur = lim;
/*					vrFilePrev(v);  */
					return (TRUE);
				}
			}
		}
		v -> textVr.cur = lim;
/*		vrFilePrev(v); */
		return (TRUE);
	}
	return (TRUE);
}
								  
static Bool vrFilePrev(v)
	FileVr v;
{
	register long i;
	char huge *b = v -> textVr.buf;

	if (!b) return (FALSE);

	if (v -> textVr.hex) {
		if (v -> textVr.cur >= 16) {
			v -> textVr.cur -= 16;
		} else if (v -> fileVr.org == 0 ||
				   !vrFileBuf(v, v -> fileVr.org - BUFSIZE)) {
			v -> textVr.cur = 0;
			return (FALSE);
		} else {
			v -> textVr.cur = (v -> textVr.lim - 16) & ~0xFL;
		}
	} else {
		for (i = v -> textVr.cur; i >= 0 && b[i] != '\n'; ) {
			if (--i < 0) {
				if (v -> fileVr.org > 0 &&
					vrFileBuf(v, v -> fileVr.org - BUFSIZE)) {
					b   = v -> textVr.buf;
					i   = v -> textVr.lim - 1;
				} else {
					v -> textVr.cur = 0;
					return (FALSE);
				}
			}
		}
		do {
			if (--i < 0) {
				if (v -> fileVr.org > 0 &&
					vrFileBuf(v, v -> fileVr.org - BUFSIZE)) {
					b   = v -> textVr.buf;
					i   = v -> textVr.lim - 1;
				} else {
					v -> textVr.cur = 0;
					return (TRUE);
				}
			}
		} while (i >= 0 && b[i] != '\n');
		if (i >= v -> textVr.lim) {
			vrFileBuf(v, v -> fileVr.org + v -> textVr.lim);
		} else {
			v -> textVr.cur = i + 1;
		}
	}
	return (TRUE);
}

static String vrFileString(v, w)
	FileVr v;
	int w;
{
	long lim = v -> textVr.lim;
	long off = v -> textVr.cur;
	register char  *p = v -> textVr.buf;

	buf[0] = 0;
	if (!p || lim == 0L || off >= lim) {
		; /* Nothing to do */
	} if (v -> textVr.hex) {
		hexStr(buf, p + off, lim - off, (long)off + v -> fileVr.org);
	} else if (lim) {
		p = textStr(buf, p + off, p + lim, 0, w);
		if (p == v -> textVr.buf + lim && lim == BUFSIZE &&
			vrFileBuf(v, v -> fileVr.org + BUFSIZE)) {
			p = v -> textVr.buf; lim = v -> textVr.lim;
			textStr(buf + strlen(buf), p, p + lim, textCol, w);
			vrFileBuf(v, v -> fileVr.org - BUFSIZE);
			v -> textVr.cur = off;
		}
	}
	return (buf);
}


VrClassRec crFileVr = {
	&crClass,				/* class */				/* Object Part */
   {												/* ObjectClass Part */
	sizeof (FileVrRec),		/* instance size */
	"FileViewer",			/* class name */
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
	vrFileGet,				/* (v) -> current object */
	vrFileSet,				/* (v, ...) re-direct */
	vrFileCopy,				/* (v, src) -> v   copy src into v */
	vrFileRewind,			/* (v) go back to beginning */
	vrFileNext,				/* (v) -> success  advance one line */
	vrFilePrev,				/* (v) -> success  retreat one line */
	vrFileString,			/* (v, width) -> string to display */
   },
};
global VrClass clFileVr = &crFileVr;

#endif

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


