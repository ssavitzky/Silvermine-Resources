/***/static char *moduleID="petosdir 1.0 (c)1988 S. Savitzky";/***/

/*********************************************************************\
**
** petosdir.c -- D I R E C T O R Y   T R E E S
**
**	900114 SS	create from irmx.c
**
\*********************************************************************/


#include "coops.h"
#include "tree.h"
#include "dir.h"
#include "viewer.h"

#include <dos.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include <malloc.h>
#include <time.h>
#include "../lib/curse.h"

#undef  global
#define global
#include "disk.h"
#include "petos.h"

/* #define WDEBUG 1 */
/* #define RDEBUG 1 */
/* #define DDEBUG 1 */
#if (!defined(DDEBUG) && (defined(WDEBUG) || defined(RDEBUG)))
#define DDEBUG 1
#endif

extern Object objInit(), objClone(), objKill(), objNew();
extern Object objDoesNotImplement();
extern String objName();
extern Tree   treeNext(), treePrev(), treeDown(), treeUp(),
			  treeSucc(), treePred(), treeFirst(), treeLast(),
			  treeAfter(), treeBefore(), treeFront(), treeBack(),
			  treeCut(), treeFind();
extern Object treeKill();
extern String   treePath(), treeHeader();

extern DirClassRec crDir;
extern String dirHeader(), dirName();
extern Object dirNew(), dirKill(), dirOpen();

extern Object vrNew();

extern uchar VolInfoBuf[];

ExtListRec dXExts[];
#define defaultConv	(dXExts[0].ftype)

/*********************************************************************\
**
** ulong petosTime(p)		convert yymmdd to seconds from 1970
**
\*********************************************************************/
static short a2i2(d1, d2)
	char d1, d2;
{
	if (d1 < '0' || d1 > '9') return (-1);
	if (d2 < '0' || d2 > '9') return (-1);
	return (d2 - '0' + 10 * (d1 - '0'));
}

ulong petosTime(p)
	char *p;
{
	struct tm tm;

#define a2i(n)	a2i2(p[n], p[n+1])

	tm.tm_sec = 0;
	tm.tm_min = 0;
	tm.tm_hour= 0;
	tm.tm_mday= a2i(4);
	tm.tm_mon = a2i(2);
	tm.tm_year= a2i(0);
	tm.tm_wday= 0;
	tm.tm_yday= 0;
	tm.tm_isdst=0;
	if (tm.tm_mday <= 0 || tm.tm_mon <= 0 || tm.tm_year <= 0)
		return(0L);
	return (mktime(&tm));

#undef a2i
}

/*********************************************************************\
**
** dOpenXDir(Dir n)			read a directory file
**
**		dOpenXDir takes a Dir node that refers to a directory, and
**		reads that directory, constructing nodes for its files.
**		(In PETOS there's only one directory, and it really ought to
**		 be read in dOpenDisk.)
**
\*********************************************************************/

Object dOpenXDir(n)
	XDir n;
{
	register XDir dp;
	int c;
	int i, j;
	extern DirEntry DirBuf[DIRLEN];
	static DirEntry ent;
	static RIB buf;
	static char name[16];

	/* 
	** There's only one directory per disk, and it's in DirBuf,
	** having been put there by dOpenDisk.  So iterate through it.
	*/
    for (i = 0; i < DIRLEN; ++i) {
		ent = DirBuf[i];
		/* 
		** Skip deleted entries
		*/
		if (!(c = 255 & ent.fname[0]))
			break;
		if (c == 0xFF)
			continue;
		if (!X_INT(ent.rib_loc))
			continue;

		if (xstatN(X_INT(ent.rib_loc), &buf)) continue;
		sprintf(name, "%.5s.%.2s", ent.fname, ent.ftype);
		dp = (XDir) cfNew(clXDir)(clXDir, n, 
								name,
								FALSE,				/* flat--no subdirs */
							    FILE_SIZE(&buf)
							   );
		if (!dp) {
			errorPrintf(
"Out of memory reading directory \"%s\"; tree will be incomplete.",
					   name);
			break;
		}
		dp -> x_dir.loc = X_INT(ent.rib_loc);
		dp -> dir.time = petosTime(buf.mdate);
		for (j = 0; j < sizeof buf.mdate; ++j) 
			dp -> x_dir.mdate[j] = buf.mdate[j];
		if (!strncmp(ent.ftype, "SP", 2)) 
			dp -> dir.ftype = defaultConv;
		else if (ent.reclen == '\r') dp -> dir.ftype = ascii;
		else    				     dp -> dir.ftype = binary;
		dp -> x_dir.dirent = ent;
	}
	return ((Object) n);
}


/*********************************************************************\
**
** R E A D   F I L E   T R E E
**
\*********************************************************************/

Dir dReadXDisk (cl, drive)
	Class cl;
	int  drive;
{
	char driveName[16];
	XDir  n;
	static RIB buf;

	dDrive = drive;
	++inUse[dDrive];

	dErrors = 0;
	if (NOTNULL(dDrives[dDrive])) gKill(dDrives[dDrive]);
	dDrives[dDrive] = (Dir) NIL;
	if (! dOpenDisk(dDrive)) {
		return ((Dir) NIL);
	}
	dErrors = 0;

	/*
	** Construct Root Node
	*/
	sprintf(driveName, "%c: %.4s", drive + 'A', VolInfoBuf);
	xstatN(T0_DirRIB, &buf);
	n = (XDir) cfNew(cl)(cl, NIL, driveName, TRUE, FILE_SIZE(&buf));
	dDrives[dDrive] = (Dir) n;
	if (!n) {
		errorPrintf(
"Out of memory creating root; tree will be empty."
				   );
		return ((Dir) n);
	}
	n -> x_dir.loc = T0_DirRIB;
	n -> dir.time = petosTime(buf.mdate);
	/* 
	** traverse tree 
	*/
	dReadDirTree(n);
	
	return ((Dir) n);
}


/*********************************************************************\
**
** C R E A T E   F I L E
**
\*********************************************************************/

Dir dCreateXFile(d, s, isDir)
	Dir d;
	String s;
	Bool isDir;
{
	int  n;
	XDir  f = (Dir)NIL;
	char name[15];
	int fnn;
	DirEntry de;

#ifdef WDEBUG
	errorPrintf("dCreateXFile(d, '%s', %s)", s, isDir? "TRUE" : "FALSE");
#endif

#if 0 		/* ===  === */
	/* 
	 * Create a Dir node (or locate an existing one)
	 * truncate name intelligently 
	 */
	dFixXName(name, s, (char *)0, 0, 14);

	/* 
	 * Check for file already existing
	 * === should ask (skip, write, rename), but instead we just bump count
	 */
	if (!isDir) 
		for (n = 1; n < 1000 && NOTNULL(gFind(d, name)); ++n) 
			dFixXName(name, s, (char *)0, n, 14);
	else 
		for (n = 1; 
			 n < 1000 && NOTNULL(f = (XDir) gFind(d, name)) && ! f -> dir.isDir; 
			 ++n) 
			dFixXName(name, s, (char *)0, n, 14);

	if (n == 1000) {
		errorPrintf("Unable to create %s '%s'", 
					isDir? "directory" : "file",
					name);
		return ((Dir) NIL);
	}

	/*
	 * Create directory node (if not a pre-existing directory)
	 */

	if (ISNULL(f)) {
		f = (XDir) cfNew(clXDir)(clXDir, d, name, isDir, 0L);
		if (ISNULL(f)) {
			errorPrintf(
				"Out of memory creating \"%s\"; tree will be incomplete",
					   name);
			return ((Dir) NIL);
		}
		f -> dir.isChanged = TRUE;
		time((time_t *)&f -> dir.time);
		/*
		 * Allocate and initialize a RIB;
		 * Construct the directory entry.
		 */
		de.rib_loc = f -> x_dir.rib_loc = xcreatN(isDir);
		if (de.rib_loc == -1) return ((Dir) NIL);
#if 0 /* ===  === */
		strncpy(de.fname, name, 8);		/* === BOGUS === */

#ifdef WDEBUG
	errorPrintf("Making dir entry: fnode=%d, name='%s', dir size = %ld", 
				de.fnode, name, d -> dir.size);
#endif

#endif
		/*
		 * Open the directory and append the entry.
		 * Close the directory when we're done (inefficient but safe).
		 */
		if (! gOpenFile(d, "ub"))
			errorPrintf("Unable to open directory for update");
		if (-1L == gSeek(d, d -> dir.size))
			errorPrintf("Unable to seek to end of directory");
		if (gWriteFile(d, &de, sizeof(de)) < sizeof(de))
			errorPrintf("Unable to append to directory");
		gCloseFile(d);
	}
#endif
	return ((Dir)f);
}

/*********************************************************************\
**
** O P E N / C L O S E
**
\*********************************************************************/

Bool dOpenXFile(f, s)
	XDir f;
	String s;
{
	register int m = O_BINARY;
	char *p;

	for (p = s; *p; ++p)
		switch (*p) {
		 case 'r': case 'R': m |= O_RDONLY; break;
		 case 'w': case 'W': m |= O_RDWR | O_CREAT; break;
		 case 'u': case 'U': m |= O_RDWR; break;
		 case 'a': case 'A': m |= O_RDWR | O_APPEND; break;
		 case 't': case 'T': m |= O_TEXT; m &= ~O_BINARY; break;
		}
	if (!f -> dir.file)	{
		/* === doesn't create fnode and dir entry on write! === */
		f -> dir.file = (ulong) xopenN(f -> x_dir.loc, m);
		if (! f -> dir.file) {
			errorPrintf("cannot open file '%s' in mode '%s'", 
						gPath(f, NIL), s);
			return (FALSE);
		}
	}
	if (m & O_RDWR) f -> dir.isChanged = TRUE;
	if (m & O_APPEND) gSeek(f, -1L);
	return (TRUE);
}

Bool dCloseXFile(f)
	XDir f;
{
	XFile *xf = (XFile *)(void *)f -> dir.file;

	if (! xf) return (FALSE);
	if (f -> dir.isChanged) {
		/*
		** Set the Dir's size from the file's fnode,
		** but set the file's date from the Dir if it's new.
		*/
		f -> dir.size = FILE_SIZE(&xf -> rib);
		if (f -> dir.time == 0)
			/* === f -> dir.time = xf -> rib.mod_time === convert === */ ;
		else {
			xf -> rib_dirty = TRUE;
			/* === xf -> rib.mod_time = f -> dir.time === convert === */ ;
		}
	}
	xclose(xf);
	f -> dir.file = 0L;
	f -> dir.isChanged = FALSE;
	return (TRUE);
}

/*********************************************************************\
**
** R E A D  /  W R I T E   F I L E S
**
**		This section supports the reading of individual files.
**		Note that a count of 0 closes the file.
**
\*********************************************************************/

int dReadXFile(f, b, n)
	XDir f;
	char huge *b;
	int	n;
{
	if (n == 0) {
		dCloseXFile(f);
		return (0);
	}
	if (!f -> dir.file && !dOpenXFile(f, "rb")) {
		return (0);
	}
	return (xread((XFile *)(void *)f -> dir.file, b, n));
}

int dWriteXFile(f, b, n)
	XDir f;
	char huge *b;
	int	n;
{
	if (n == 0) {
		dCloseXFile(f);
		return (0);
	}
	if (!f -> dir.file && !dOpenXFile(f, "wb")) {
		return (0);
	}
	return (xwrite((XFile *)(void *)f -> dir.file, b, n));
}

global unsigned dValidateX(d)
	Dir d;
{
	char c;

	if (ISNULL(d)) return (FALSE);
	return (TRUE);			/* === really ought to read volID === */
}

global int dSeekX(d, loc)
	Dir d;
	long loc;
{
	if (!d -> dir.file)	{return (FALSE);}
	if (loc >= 0) {
		loc = xseek(d -> dir.file, loc);
	} else {
		loc = xseek(d -> dir.file, d -> dir.size - loc + 1);
	}
	return(loc != -1L);
}

global long dTellX(d)
	Dir d;
{
	if (!d -> dir.file)	{return (-1L);}
	return(xtell(d -> dir.file));
}


String dirXHeader(d, buf, len)
	XDir  	d;
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
		if (d -> dir.time == -1) {
			sprintf(buf + 19, "%c %8ld pre-1980 (%6.6s)",
					*dFtypeName[d -> dir.ftype],
					(unsigned long)(d -> dir.size), 
					d -> x_dir.mdate);
		} else if (d -> dir.time) {
			tm = localtime((time_t*) &d -> dir.time);
			sprintf(buf + 19, "%c %8ld %02d/%02d/%02d         ",
					*dFtypeName[d -> dir.ftype],
					(unsigned long)(d -> dir.size),
					tm -> tm_year, tm -> tm_mon + 1, tm -> tm_mday);
		} else {
			sprintf(buf + 19, "%c %8ld un-dated (%6.6s)",
					*dFtypeName[d -> dir.ftype],
					(unsigned long)(d -> dir.size),
					d -> x_dir.mdate);
		}
		sprintf(buf + strlen(buf), " %02x", /* %02x%02x%02x %02x%02x%02x */
				d -> x_dir.dirent.reclen,
				d -> x_dir.dirent.pad0[0] & 255,
				d -> x_dir.dirent.pad0[1] & 255,
				d -> x_dir.dirent.pad0[2] & 255,
				d -> x_dir.dirent.pad1[0] & 255,
				d -> x_dir.dirent.pad1[1] & 255,
				d -> x_dir.dirent.pad1[2] & 255
				);
	}
}

/*********************************************************************\
**
** Classes
**
\*********************************************************************/

global ExtListRec dXExts[] = {
	{0, ".SP"},
	{0,	NULL}	
};

global DirClassRec crXDir = {
	&crClass,				/* class */				/* Object Part */
   {												/* Class Part */
	sizeof (XDirRec),		/* instance size */
	"PETOS",					/* class name */
	(Class)&crDir,				/* super */
	NIL,					/* method dict */
	objInit,				/* initialize */
	dirNew,					/* new object */
	objClone,				/* clone self */
	dirKill,				/* kill self */
	dOpenXDir,				/* open */
	objDoesNotImplement,	/* close */
	dirName,				/* name */
   }, {
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
	dirXHeader,				/* header */
	treePath,				/* pathname */
	treeFind,				/* find by name */
   }, {
   	14,						/* max name length */
   	0,
    dReadXDisk,
	dCreateXFile,
	dOpenXFile,
	dCloseXFile,
	dReadXFile,
	dWriteXFile,
	dValidateX,
	dSeekX,
	dTellX,
	0, /* === rename === */
	0, /* === unlink === */
	dXExts,
   },
};

Class clXDir   = (Class)&crXDir;



