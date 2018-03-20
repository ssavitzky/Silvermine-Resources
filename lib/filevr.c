/*********************************************************************\
**
** FileVr.c	--	viewer for disk files
**
**	891210 SS	split FileVr out of viewer.c
**
\*********************************************************************/

#include <stdio.h>

#include "coops.h"
#include "tree.h"
#include "dir.h"
#include "viewer.h"

#undef  global
#define global

#include "filevr.h"

extern Object objInit(), objClone(), objKill(), objDoesNotImplement();
extern String objName();

extern Object vrNew();
extern void hexStr();
extern char *textStr();
extern int textCol;

global char fileVrEOL = '\n';

#define EOL fileVrEOL

/*
** Things useful for formatting
*/
static char buf[81];

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
	if (gSeek(d, org) == -1) {
		b -> size = 0;
	} else {
		b -> size  = gReadFile(d, b -> data, BUFSIZE);
	}
	if (b -> size == 0) {
		strcpy(b -> data, "*READ ERROR*");
		b -> size = strlen(b -> data);
	}
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

/* Private Routines */

static Bool vrFileBuf(v, org)		/* set buffer for given origin */
	FileVr	v;
	long	org;
{
	Buffer *b;
	int bx;

#ifdef DEBUG_FV
errorPrintf("Setting buffer for org %ld", org);
#endif
	if (ISNULL(v -> fileVr.d) || org >= v -> fileVr.d -> dir.size) {
		if (org == 0L) {
			v -> fileVr.org = org;
			v -> fileVr.bx = -1;
			v -> textVr.buf = (char *)0;
			v -> textVr.cur = 0;
			v -> textVr.lim = 0;
		}
		return(FALSE);
	}
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

/*
** Return true if file appears to be non-ascii
**		Non-ascii is indicated by the presence of nulls or
**		long (>128 chars) lines.
*/
static Bool vrFileNonAscii(v)
	FileVr	v;
{
	register char *p = v -> textVr.buf;
	register int   i = v -> textVr.lim;
	register int  ll = 0;

	if (ISNULL(v -> fileVr.d)) return(FALSE);
	for ( ; i; ++p, --i, ++ll) {
		if (*p == 0) return (TRUE);
		if (*p == EOL) ll = 0;
		if (ll > 128) return (TRUE);
	}
	return (FALSE);
}

/* Public routines */

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
	v -> textVr.hex = vrFileNonAscii(v);
}

static Opaque vrFileGet(v)
	FileVr	v;
{
	return ((Opaque) v -> fileVr.d);
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
			if (c == EOL) {
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
	char *b = v -> textVr.buf;

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
		i = v -> textVr.cur;
		do {
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
		} while (i >= 0 && b[i] != EOL);
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
		} while (i >= 0 && b[i] != EOL);
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


