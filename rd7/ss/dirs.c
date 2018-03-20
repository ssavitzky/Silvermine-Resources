/***/static char *moduleID="dirs 1.0 (c)1988 S. Savitzky";/***/

/*********************************************************************\
**
**	D I R E C T O R Y   T R E E S
**
**		Here is where we read, write, and maintain the in-core
**		representation of the directory tree.
**
**		For each file, we keep its
**			name
**			size
**			starting block number
**			and the usual tree links.
**
**	880111 SS	create PC version from DD
**	880419 SS	rename from data to dirs, add coops class stuff
**	880430 SS	add DOS stuff.
**	880509 SS	move Idris stuff to convert.c
**
\*********************************************************************/

#include "rd7.h"
#include "coops.h"
#include "trees.h"
#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <malloc.h>
#include <dos.h>
#include "../lib/dostypes.h"		/* from Microsoft library sources */

#undef  global
#define global
#include "dirs.h"

extern void errorSet(), errorClear();
extern int errorCheck();


extern Object objInit(), objClone(), objKill(), objNew();
extern Object objDoesNotImplement();
extern Tree   treeNext(), treePrev(), treeDown(), treeUp(),
			  treeSucc(), treePred(), treeFirst(), treeLast(),
			  treeAfter(), treeBefore(), treeFront(), treeBack(),
			  treeCut(), treeFind();
extern Object treeKill();
extern String   treePath(), treeHeader();

extern TreeClassRec crTree;

/*********************************************************************\
**
** Local allocation and deallocation routines
**
**		These use a free list for speed.
**		They also try to scavenge trees that aren't being used.
**		The whole thing is pretty ugly, but we're on a PC and
**		memory is scarce.
**
\*********************************************************************/

int freeSomeTree(cl)
	Class cl;
{
	register int i;
	Dir d;
	ushort biggest;
	int j;

	/*
	** This is a kludge.  It tries to find a suitable tree to free.
	** The most "suitable" is the largest tree that isn't in use by
	** some view and isn't on drive A or B.  
	** Well, it's better than giving up.
	*/
	for (d = NIL, biggest = 0, i = MAXDRIVES; i >= 2; --i) {
		if (NOTNULL(dDrives[i])
		 && fClass(dDrives[i]) == cl
		 && ! inUse[i]
		 && dDrives[i] -> dir.fcount > biggest) { 
			d = (Dir) dDrives[i];
			j = i;
			biggest = d -> dir.fcount;
		}
	}
	if (d) {
		dDrives[j] = (Dir) gKill(d);
		return (TRUE);
	} else
		return (FALSE);
}


Dir dirCalloc(cl, extra)
	Class cl;
	ushort extra;
{
	Dir d;

	if (NOTNULL(dirFreelist(cl)) || freeSomeTree(cl)) {
												/* Allocate from freelist */
		d = dirFreelist(cl);
		dirFreelist(cl) = (Dir) d -> tree.next;
		memset(d, 0, fClassIsize(cl));			/* Clear it */
		fClass(d) = cl;
	} else {									/* Allocate from heap */
		/* === ought to grab a bunch while we're here === */
		d = (Dir) objCalloc(cl, 0);
	}
	return (d);
}

Object dirFree(d)
	Dir d;
{
	d -> tree.next = (Tree) dirFreelist(fClass(d));
	dirFreelist(fClass(d)) = d;
	return (NIL);
}


/*********************************************************************\
**
** Class-callable operations
**
\*********************************************************************/

String dirName(d)
	Dir d;
{
	return (d -> dir.name);
}

Object dirKill(d)
	Dir d;
{
	Tree p, q;

	for (p = (Tree)d -> tree.down; NOTNULL(p); p = q) {
		q = p -> tree.next;
		gKill(p);
	}
	return (dirFree(d));
}

String dirHeader(d, buf, len)
	Dir  	d;
	String	buf;
	Cardinal len;
{
	register int i;
	struct tm *tm;

	if (len < 38) 
		treeHeader(d, buf, len);
	else {
		sprintf(buf, "%s%c", gName(d), d ->dir.tcount? '*' : ' ');
									/* and other information */
		for (i = strlen(buf); i < 19; ++i) buf[i] = ' ';
		switch (d -> dir.mode) {
		 case unknown:	strcpy(buf + 19, "?");	break;
		 case ascii:	strcpy(buf + 19, "A");	break;
		 case binary:	strcpy(buf + 19, "B");	break;
		 case css:		strcpy(buf + 19, "C");	break;
		 case jcamp:	strcpy(buf + 19, "J");	break;
		 case spectrum:	strcpy(buf + 19, "S");	break;
		}
		tm = localtime((time_t*) &d -> dir.time);
		sprintf(buf + strlen(buf), " %02d/%02d/%02d %8ld", 
				tm -> tm_year, tm -> tm_mon + 1, tm -> tm_mday,
				(unsigned long)(d -> dir.size));
	}
}

/*********************************************************************\
**
** Node Creation
**
**		Uses a dumb insertion sort to keep the list alphabetical.
**		We can get away with this because the average directory isn't
**		very big.
**
**		All of the recursively-defined totals are cached to make
**		screen update snappy.
**
\*********************************************************************/

global Object dirNew(cl, parent, name, isDir, size)
	Class cl;
	Dir  parent;
	char *name;
	char isDir;
	ulong size;
{
	Dir n, m, p, *h;
	ulong s;
	int c;

	if (!parent) { 						/* handle root specially */
		n = dirCalloc(cl, 0); 
		if (!n) return ((Object)n);
		goto fillin;
	}

	h = (Dir *) (&parent -> tree.down);
	p = *h;
	if (ISNULL(p)) {
		n = dirCalloc(cl, 0);
		if (!n) return ((Object)n);
		n -> tree.up   = (Tree)parent;
		*h = n;	
	} else if ((c = strncmp(name, p -> dir.name, NAMELEN)) > 0) {
		for ( ; 
			 (m = (Dir)p -> tree.next, NOTNULL(m)) 
			   && (c = strncmp(name, m -> dir.name, NAMELEN)) > 0;
			 p = m) ;
		if (c == 0) goto Existing;
		n = dirCalloc(cl, 0);
		if (!n) return ((Object) n);
		treeAfter(p, n);
	} else if (c < 0) {
		n = dirCalloc(cl, 0);
		if (!n) return ((Object) n);
		n -> tree.next = (Tree) p;
		p -> tree.prev = (Tree) n;
		n -> tree.up   = (Tree)parent;
		*h = n;
	} else {
  Existing:
		n = p;
		/* It's already there, so just correct the size */
		/* === worry about dir/non-dir later === */
		/* === worry about the size later, too === */
		return ((Object) n);
	}

fillin:
	strncpy(n -> dir.name, name, sizeof(n -> dir.name));
	n -> dir.isDir = isDir;
	n -> dir.size  = size;

	if (isDir) {
		n -> dir.fcount = 0;
		n -> dir.dcount = 1;
		n -> dir.fsize  = s = 0;
	} else {
		n -> dir.fcount = 1;
		n -> dir.dcount = 0;
		n -> dir.fsize  = s = size;
	}
	for (p = (Dir)n -> tree.up; p; p = (Dir)p -> tree.up) {
		p -> dir.fcount += n -> dir.fcount;
		p -> dir.dcount += n -> dir.dcount;
		p -> dir.fsize  += s;
	}
	return ((Object)n);
}


/*********************************************************************\
**
** Open (read in) DOS directory 
**
**		Note the use of stat to get the last-modified time.
**		This is inefficient, but it beats trying to decode the DOS time.
**
\*********************************************************************/

static struct find_t find_buf;

Object dirOpen(n)
	Dir n;
{
	register Dir dp;
	char path[256];
	struct stat stat_buf;

	errorClear();
	GetPath(n, path);
	strcat(path, "\\*.*");
	if (_dos_findfirst(path, _A_SUBDIR, &find_buf)) {
		errorCheck();
		return (NIL);
	}
	if (errorCheck()) return (NIL);
	do {
		if (!strcmp(find_buf.name, ".") || !strcmp(find_buf.name, ".."))
			continue;
		dp = (Dir) cfNew(clDir)(clDir, n, find_buf.name,
							    0 != (find_buf.attrib & _A_SUBDIR),
						  		find_buf.size);
		if (!dp) {
			errorPrintf(
"Out of memory reading directory \"%s\"; tree will be incomplete",
					   find_buf.name);
			break;
		}
		dp -> dir.time = XTIME(find_buf.wr_date,find_buf.wr_time);
/*		stat(gPath(dp, NIL), &stat_buf);	*/
/*		dp -> dir.time = stat_buf.st_mtime;	*/
		if (dEachNewDir && ISDIR(dp)) (*dEachNewDir)(dp);
	} while (! _dos_findnext(&find_buf));

	return ((Object) n);
}

/*********************************************************************\
**
** dReadDosDir(dir)
**
**		(re)read the directory tree for dir
**
\*********************************************************************/

global Dir dReadDosDir(n)
	Dir n;
{
	/*
	** The strategy at this point is to read a directory using rdDir,
	** then recursively read each of its subtrees.
	*/
	gOpen(n);
	for (n = (Dir)gDown(n); NOTNULL(n); n = (Dir)gNext(n)) 
		if (n -> dir.isDir) dReadDosDir(n);
	return (n);
}


/*********************************************************************\
**
** void dReadDosTree(Drive)
**
**		Read the directory tree for drive (actually any path will do).
**
\*********************************************************************/

global Dir dReadDosTree(drive)
	String drive;
{
	Dir n;
	int d;

	d = dDriveNum(drive);
	inUse[d] = TRUE;
	if (NOTNULL(dDrives[d])) gKill(dDrives[d]);
	n = (Dir) cfNew(clDir)(clDir, NIL, drive, TRUE, 0L);
	dDrives[d] = n;
	if (!n) {
		errorPrintf(
"Out of memory creating root; tree will be empty."
				   );
		return (n);
	}
	if (dEachNewDir) (*dEachNewDir)(n);
	dReadDosDir(n);
	return (n);
}

/*********************************************************************\
**
** Setting Working Directory
**
**	=== unless we stop using "." as a root, this is useless ===
**
\*********************************************************************/

global Bool dSetWorkingDir(d)
	Dir d;
{
	char *p;
	unsigned drive, ndrives;
	/* 
	** make this directory the current one for writes 
	** (and everything else!)
	*/
	if (!d) return (FALSE);
	p = gPath(d, NIL);
	if (p[1] == ':') {
		/*
		** Due to DOS braindamage, we have to set the drive specially!
		*/
		drive = *p - 'A' + 1;
		_dos_setdrive(drive, &ndrives);
		if (drive >= ndrives) return (FALSE);
		if (!p[2]) {
			/* Special hack for roots */
			if (chdir("\\")) return (FALSE);
			strcpy(dWorkingPath, p);
			strcat(dWorkingPath, "\\");
			dWorkingDrive = drive;
			return(TRUE);
		}
	}
	if (chdir(p)) return (FALSE);
	strcpy(dWorkingPath, p);
	_dos_getdrive(&dWorkingDrive);
	return(TRUE);
}


/*********************************************************************\
**
** Tagging and untagging
**
**		When tagging a directory, recursively descend and tag
**		all of its descendents.  Report the total size.
**
\*********************************************************************/

global long dTaggedSize(n)
	Dir n;
{
	register long s;
	register Dir p;

	return (n -> dir.tsize);
}

global long	  dTag(n)
	Dir n;
{
	Dir p;

	if (n -> dir.isDir) {
		for (p = (Dir) gDown(n); p; p = (Dir) gNext(p)) dTag(p);
	} else if (n -> dir.tcount == 0) {
		n -> dir.tcount = 1;
		n -> dir.tsize  = n -> dir.size;
		for (p = (Dir)n -> tree.up; p; p = (Dir)p -> tree.up) {
			p -> dir.tcount += 1;
			p -> dir.tsize  += n -> dir.size;
		}
	}
	return (n -> dir.tsize);
}

global long	  dUntag(n)
	Dir n;
{
	Dir p;

	if (n -> dir.isDir) {
		for (p = (Dir) gDown(n); p; p = (Dir) gNext(p)) dUntag(p);
	} else if (n -> dir.tcount != 0) {
		n -> dir.tcount = 0;
		n -> dir.tsize  = 0;
		for (p = (Dir)n -> tree.up; p; p = (Dir)p -> tree.up) {
			p -> dir.tcount -= 1;
			p -> dir.tsize  -= n -> dir.size;
		}
	}
	return (0);
}

/*********************************************************************\
**
** char huge *dReadDosFile(f)
**
**		Suck the whole file into a buffer, or as much as will fit.
**
**		The dReadDosBuffer crock is non-reentrant and assumes that
**		we're not reading any other files with dReadDosFile in the
**		meantime (a good assumption so far).  We close after each
**		chunk in case we give up after the first (e.g. for viewing)
**
\*********************************************************************/

static ulong sizeRemaining;

char huge *dReadDosFile(f)
	Dir f;
{
	register char huge *p;
	register int   c;
	register FILE *ff;
	ulong size;
	int sz;
	char huge *buf;

	if (ISNULL(f)) return (0);

	size = f -> dir.size;
	if (size > STATICBUFMAX) size = STATICBUFMAX;
	sizeRemaining = f -> dir.size - size;
	if (!(buf = BufAlloc(size, 1))) {
		return (buf); 
	}

	ff = fopen(gPath(f, NIL), "rb");
#if 1
	for (p = buf; size && (sz = fread(p, 1, 1024, ff)); p += sz, size -= sz) ;
#else
	for (p = buf; size-- && (c = getc(ff)) != -1; ++p) *p = c;
#endif
	fclose(ff);

	return (buf);
}

char huge *dReadDosBuffer(f, b)
	Dir f;
	char huge *b;
{
	register char huge *p;
	register int   c;
	register FILE *ff;
	ulong size;
	int sz;
	char huge *buf;

	size = sizeRemaining;
	if (size > STATICBUFMAX) size = STATICBUFMAX;
	buf = b;

	ff = fopen(gPath(f, NIL), "rb");
	fseek(ff, sizeRemaining, SEEK_SET);
	sizeRemaining -= size;

	for (p = buf; size && (sz = fread(p, 1, 1024, ff)); p += sz, size -= sz) ;

	fclose(ff);
	return (buf);
}

/*********************************************************************\
**
** Classes
**
\*********************************************************************/

global DirClassRec crDir = {
	(Class)&crTree,			/* class */				/* Object Part */
   {												/* Class Part */
	sizeof (DirRec),		/* instance size */
	"Dir",					/* class name */
	&crObject,				/* super */
	NIL,					/* method dict */
	objInit,				/* initialize */
	dirNew,					/* new object */
	objClone,				/* clone self */
	dirKill,				/* kill self */
	dirOpen,				/* open */
	objDoesNotImplement,	/* close */
	dirName,				/* name */
   }, {
    '\\',					/* path sep */
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
	dirHeader,				/* header */
	treePath,				/* pathname */
	treeFind,				/* find by name */
   }, {
   	0
   },
};

Class clDir   = (Class)&crDir;


