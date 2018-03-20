/***/static char *moduleID="dir 1.0 (c)1988 S. Savitzky";/***/

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

#include "coops.h"
#include "tree.h"
#include <io.h>
#include <stdio.h>
#include <time.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/utime.h>
#include <malloc.h>
#include <dos.h>
#include "dostypes.h"		/* from Microsoft library sources */

#undef  global
#define global
#include "dir.h"

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
** Globals: Hidden parameters to file creation ops.
**
\*********************************************************************/

global char *dFileExt = (char *)0;	/* extension for new file */
global int   dFileVer = 1;			/* clobber existing file? */
									/* 1 -- no, rename new    */
									/*-1 -- no, rename old	  */
									/* 0 -- yes.			  */

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
** Filename Mapping
**
\*********************************************************************/

/*
** Map lowercase to uppercase, [] to (), everything else to ~.
*/

static char fixTbl[128] = {
/*	   0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F */
/*0*/  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
/*1*/  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
/*2*/  0, '!', 0, '#','$','%','&', 39,'(',')', 0,  0,  0, '-', 0,  0, 
/*3*/ '0','1','2','3','4','5','6','7','8','9', 0,  0,  0,  0,  0,  0, 
/*4*/ '@','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O',
/*5*/ 'P','Q','R','S','T','U','V','W','X','Y','Z','(', 0, ')','^','_',
/*6*/ '`','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O',
/*7*/ 'P','Q','R','S','T','U','V','W','X','Y','Z','{', 0, '}','~', 0, 
};

/*
** fixcpy(dst, src, len)
**
**		copy at most len bytes of src to dst,
**		fixing characters in the process.
*/
static void fixcpy(dst, src, len)
	char *dst;
	char *src;
	int   len;
{
	for ( ; *src && len; ++dst, ++src, --len) 
		*dst = fixTbl[*src] ? fixTbl[*src] : '~';
	*dst = 0;
}

/*
** shorten(dst, dstlen, src, srclen)
**
**		shorten a string by truncating at the right,
**		uppercasing and fixing characters in the process.
*/
static void shorten(dst, dstlen, src, srclen)
	char *dst;
	int   dstlen;
	char *src;
	int   srclen;
{
	/*
	** First see if it fits.
	**		if it does, copy, substituting characters as needed
	**		if not, truncate it
	*/
	if (srclen <= dstlen) {
		fixcpy(dst, src, srclen);
	} else {
		fixcpy(dst, src, dstlen);
	}
}

/*
** appendCount (dst, clobber)
*/
void appendCount (dst, clobber, maxl)
	char *dst;
	int   clobber;
	int   maxl;
{
	int len = strlen(dst);
	char nn[6];

	sprintf(nn, "%d", clobber);
	if (strlen(dst) + strlen(nn) <= maxl)
		strcat(dst, nn);
	else {
		strcpy(dst + maxl - strlen(nn), nn);
	}
}
/*
** dFixName (dst, src, ext, clobber)
**
**		Fix a filename, putting it in MS-DOS format.
**		If clobber > 0, it is a count of the number of conflicting
**		files found so far.  It is appended to the name.
*/
void dFixName(dst, src, ext, clobber)
	char *dst;
	char *src;
	char *ext;
	int clobber;
{
	char *extDot = strrchr(src, '.');

	if (extDot) {
		shorten(dst, 8, src, extDot - src);
		if (clobber) appendCount(dst, clobber, 8);
		strcat(dst, ".");
		if (ext != NULL)
			strcat(dst, ext);
		else
			shorten(dst + strlen(dst), 3, extDot + 1, strlen(extDot + 1));
	} else {
		shorten(dst, 8, src, strlen(src));
		if (ext != NULL) strcat(dst, ext);
		if (clobber) appendCount(dst, clobber, 8);
	}
}

/*
** dFixXName (dst, src, ext, clobber, len)
**
**		Fix a filename, putting it in Unix format.
**		If clobber > 0, it is a count of the number of conflicting
**		files found so far.  It is appended to the name.
**		=== fixtable bogus ===
*/
void dFixXName(dst, src, ext, clobber, maxl)
	char *dst;
	char *src;
	char *ext;
	int clobber;
	int maxl;
{
	char *extDot = strrchr(src, '.');

	if (extDot) {
		shorten(dst, maxl, src, extDot - src);
		if (clobber) appendCount(dst, clobber, maxl);
		strcat(dst, ".");
		if (ext != NULL)
			strcat(dst, ext);
		else
			shorten(dst + strlen(dst), 3, extDot + 1, strlen(extDot + 1));
	} else if (ext) {
		shorten(dst, maxl - strlen(ext) - 1, src, strlen(src));
		strcat(dst, "."); strcat(dst, ext);
		if (clobber) appendCount(dst, clobber, maxl - strlen(ext) - 1);
	} else {
		shorten(dst, maxl, src, strlen(src));
		if (clobber) appendCount(dst, clobber, maxl);
	}
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

	if (!d -> dir.isDir && len < 38) 
		treeHeader(d, buf, len);
	else if (d -> dir.isDir) {
		sprintf(buf, "%s%c", gName(d), d ->dir.tcount? '*' : ' ');
		for (i = strlen(buf); i < 16; ++i) buf[i] = ' ';
		sprintf(buf + 16, " %d(%ldK)",
			    d -> dir.fcount, (d -> dir.fsize + 1023)/1024);
	} else {
		sprintf(buf, "%s%c", gName(d), d ->dir.tcount? '*' : ' ');
		for (i = strlen(buf); i < 19; ++i) buf[i] = ' ';
		if (d -> dir.time) {
			tm = localtime((time_t*) &d -> dir.time);
			sprintf(buf + 19, "%c %8ld %02d/%02d/%02d",
					*dFtypeName[d -> dir.ftype],
					(unsigned long)(d -> dir.size),
					tm -> tm_year, tm -> tm_mon + 1, tm -> tm_mday);
		} else {
			sprintf(buf+19, "%c %8ld un-dated",
					*dFtypeName[d -> dir.ftype],
					(unsigned long)(d -> dir.size) );
		}
	}
}

/*
** Close -- make the node's kids go away.
*/
Object dirClose(d)
	Dir d;
{
	Tree p, q;

	for (p = (Tree)d -> tree.down; NOTNULL(p); p = q) {
		q = p -> tree.next;
		gKill(p);
	}
	d -> dir.isOpen = FALSE;
	d -> tree.down = (Tree) NIL;
	return ((Object)d);
}


/*
** Simple 1-level find routine: 
**		search for a name among the	children of a node.
*/
Dir dirFind(d, s)
	Tree d;
	String s;
{
	for (d = gDown(d); d && strncmp(gName(d), s, NAMELEN); d = gNext(d)) ;
	return ((Dir) d);
}

/*********************************************************************\
**
** Node Creation
**
**		Uses a dumb insertion sort to keep the list alphabetical.
**		We can get away with this because the average directory isn't
**		very big.  (Loses on big directories.)
**
**		All of the recursively-defined totals are cached to make
**		screen update snappy.
**
\*********************************************************************/

global Object dirNew(cl, parent, name, isDir, size)
	DirClass cl;
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
	strncpy(n -> dir.name, name, cl -> dirClass.name_size);
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
	if (!isDir) {		/* find extension in table */
		ExtList		ex;

		for (ex = cl -> dirClass.exts; ex ; ) {
			if (ex -> ftype == 0xffff) {
				ex = (ExtList) ex -> ext;
				continue;
			} else if (!ex -> ext) {
				if (!n -> dir.ftype) n -> dir.ftype = ex -> ftype;
				break;
			} else if (!strcmp(ex -> ext, 
							   name + strlen(name) - strlen(ex -> ext))) {
				n -> dir.ftype = ex -> ftype;
				break;
			} else {
				++ex;
			}
		}
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
		/* === if n is tagged, tag dp too === */
	} while (! _dos_findnext(&find_buf));

	return ((Object) n);
}

/*********************************************************************\
**
** dReadDirTree(dir)
**
**		(re)read the directory tree for dir
**
\*********************************************************************/

global Dir dReadDirTree(n)
	Dir n;
{
	/*
	** The strategy at this point is to read a directory using rdDir,
	** then recursively read each of its subtrees.
	*/
	gOpen(n);
	if (dEachNewDir) (*dEachNewDir)(n);
	for (n = (Dir)gDown(n); NOTNULL(n); n = (Dir)gNext(n)) 
		if (n -> dir.isDir) dReadDirTree(n);
	return (n);
}


/*********************************************************************\
**
** void dReadDosTree(Drive)
**
**		Read the directory tree for drive (actually any path will do).
**
\*********************************************************************/

global Dir dReadDosTree(cl, d)
	Class  cl;
	int d;
{
	Dir n;
	char dname[3];

	sprintf(dname, "%c:", d + 'A');

	++inUse[d];
	if (NOTNULL(dDrives[d])) gKill(dDrives[d]);
	n = (Dir) cfNew(cl)(cl, NIL, dname, TRUE, 0L);
	dDrives[d] = n;
	if (!n) {
		errorPrintf(
"Out of memory creating root; tree will be empty."
				   );
		return (n);
	}
	dReadDirTree(n);
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

	if (ISNULL(n)) return(0);
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

	if (ISNULL(n)) return(0);
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
** Create a file or directory.
**
**		The name is fixed up if necessary.
**		Directories are created immediately on disk,
**		files have a Dir node created but nothing else is done;
**		the file will actually be created when it is opened.
**
**		if global dFileExt is non-null, it is used as an extension
**		for the new filename.
**
\*********************************************************************/

Dir dCreateDosFile(d, s, isDir)
	Dir d;
	String s;
	Bool isDir;
{
	int  n;
	Dir  f = (Dir)NIL;
	static char path[256];
	char *name;

	/* 
	 * truncate name intelligently 
	 */
	strcpy(path, gPath(d, NIL));
	name = path + strlen(path);
	*name++ = gPathSep(d);   *name = 0;
	dFixName(name, s, dFileExt, 0);

	/* 
	 * Check for file already existing
	 * === should ask (skip, write, rename), but instead we just bump count
	 */
	if (!isDir) 
		for (n = 1; n < 1000 && NOTNULL(dirFind(d, name)); ++n) 
			dFixName(name, s, dFileExt, n);
	else 
		for (n = 1; 
			 n < 1000 && NOTNULL(f = dirFind(d, name)) && ! f -> dir.isDir; 
			 ++n) 
			dFixName(name, s, (char *)0, n);

	dFileExt = (char *)0;
	if (n == 1000) {
		errorPrintf("Unable to create %s '%s'", 
					isDir? "directory" : "file",
					name);
		return ((Dir) NIL);
	}

	/*
	 * Create directory node (if not a pre-existing directory) and return it
	 * if isDir, create it on disk immediately
	 */
	if (isDir && ISNULL(f) && mkdir(path)) {
		errorPrintf("Unable to create directory '%s'", path);
		return ((Dir) NIL);
	}
	
	if (ISNULL(f)) f = (Dir) cfNew(clDir)(clDir, d, name, isDir, 0L);
	if (ISNULL(f)) {
		errorPrintf(
"Out of memory creating \"%s\"; tree will be incomplete",
				   name);
		return ((Dir) NIL);
	}
	f -> dir.isChanged = !isDir;
	return (f);
}

/*********************************************************************\
**
** DOS file open/close
**
**		There is a bit of wierdness about times.  You can set the
**		file's last-modified time by setting it's dir.time field
**		to a non-zero value while the file is open for writing.
**		This feature is used to preserve time when copying a file.
**
\*********************************************************************/

Bool dOpenDosFile(f, s)
	Dir f;
	String s;
{
	register int m = O_BINARY;
	char *p;

	for (p = s; *p; ++p)
		switch (*p) {
		 case 'r': case 'R': 
		 	m |= O_RDONLY; 
			break;
		 case 'w': case 'W': 
		 	m |= O_RDWR | O_CREAT;
			f -> dir.time = 0L;
			f -> dir.isChanged = TRUE;
			break;
		 case 't': case 'T':
		 	m &= ~O_BINARY;
			m |= O_TEXT;
			break;
		}
	if (!f -> dir.file)	{
		f -> dir.file = (ulong) open(gPath(f, NIL), m, S_IWRITE | S_IREAD);
		if (! f -> dir.file) {
			errorPrintf("cannot open file '%s' in mode '%s'", 
						gPath(f, NIL), s);
			return (FALSE);
		}
	}
	return (TRUE);
}

Bool dCloseDosFile(f)
	Dir f;
{
	struct stat stat_buf;
	struct utimbuf times;
	Dir p;
	long diff;

	if (f -> dir.file)
		close((int)f -> dir.file);
	f -> dir.file = 0;
	if (f -> dir.isChanged)	{
		stat(gPath(f, NIL), &stat_buf);
		if (f -> dir.time == 0L) {
			/* file had no time originally; get from disk */
			f -> dir.time = stat_buf.st_mtime;
		} else {
			/* update time on disk from time in file */
			times.actime = times.modtime = f -> dir.time;
			utime(gPath(f, NIL), &times);
		}
		/* update size of file, and total size of all ancestors */
		diff = f -> dir.size;
		f -> dir.size = stat_buf.st_size;
		diff -= f -> dir.size;
		for (p = (Dir)f -> tree.up; p; p = (Dir)p -> tree.up) {
			p -> dir.fsize -= diff;
		}
		f -> dir.isChanged = FALSE;
	}
	return (TRUE);
}

/*********************************************************************\
**
** DOS file read/write
**
**		Read and Write open the file if necessary.
**		n == 0 closes the file.
**
\*********************************************************************/

int dReadDosFile(f, b, n)
	Dir f;
	char huge *b;
	int	n;
{
	if (n == 0) {
		dCloseDosFile(f);
		return (0);
	}
	if (!f -> dir.file && !dOpenDosFile(f, "rb")) {
		return (0);
	}
	return (read((int) f -> dir.file, b, n));
}

int dWriteDosFile(f, b, n)
	Dir f;
	char huge *b;
	int	n;
{
	if (n == 0) {
		dCloseDosFile(f);
		return (0);
	}
	if (!f -> dir.file && !dOpenDosFile(f, "wb")) {
		return (0);
	}
	return (write((int)f -> dir.file, b, n));
}


global unsigned dValidateDos(d)
	Dir d;
{
	struct stat buf;
	char path[255];
	struct diskfree_t diskspace;

	errorClear();
	if (ISNULL(d)) return (FALSE);
	GetPath(d, path);
	if (_dos_getdiskfree(dDriveNum(path) + 1, &diskspace)) 
		if (errorCheck()) return (FALSE);
		else {
			errorPrintf("DOS input drive invalid or inaccessible");
			return (FALSE);
		}
	if (errorCheck()) return (FALSE);
	if (strlen(path) == 2 && path[1] == ':') strcat(path, "\\");
	if (stat(path, &buf))
		if (errorCheck()) return (FALSE);
		else {
			errorPrintf("DOS input drive invalid or inaccessible");
			return (FALSE);
		}
	if (errorCheck()) return (FALSE);
	if ((buf.st_mode & S_IFDIR))
		return (d -> dir.isDir != 0);
	else
		return (d -> dir.isDir == 0);
}

global int dSeekDos(d, loc)
	Dir d;
	long loc;
{
	if (!d -> dir.file)	{return (FALSE);}
	if (loc >= 0) {
		loc = lseek((int)d -> dir.file, loc, SEEK_SET);
	} else {
		loc = lseek((int)d -> dir.file, (1L - loc), SEEK_END);
	}
	return(loc != -1L);
}

global long dTellDos(d)
	Dir d;
{
	if (!d -> dir.file)	{return (-1L);}
	return(tell((int)d -> dir.file));
}


/*********************************************************************\
**
** Classes
**
\*********************************************************************/

global ExtListRec dExts[] = {
	{1, ".TXT"},
	{1, ".NOT"},
	{1, ".C"},
	{1, ".H"},
	{1, ".BAT"},
	{2, ".OBJ"},
	{2, ".EXE"},
	{2, ".COM"},
	{2, ".DAT"},
	{0,	NULL}	
};

global DirClassRec crDir = {
	&crClass,				/* class */				/* Object Part */
   {												/* Class Part */
	sizeof (DirRec),		/* instance size */
	"DOS",					/* class name */
	(Class)&crTree,			/* super */
	NIL,					/* method dict */
	objInit,				/* initialize */
	dirNew,					/* new object */
	objClone,				/* clone self */
	dirKill,				/* kill self */
	dirOpen,				/* open */
	dirClose,				/* close */
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
   	12,						/* max name length */
   	0,
    dReadDosTree,
	dCreateDosFile,
	dOpenDosFile,
	dCloseDosFile,
	dReadDosFile,
	dWriteDosFile,
	dValidateDos,
	dSeekDos,
	dTellDos,
	0, /* === rename === */
	0, /* === unlink === */
	dExts,
   },
};

Class clDir   = (Class)&crDir;


