/***/static char *moduleID="irmxdir 1.0 (c)1988 S. Savitzky";/***/

/*********************************************************************\
**
** irmxdir.c -- D I R E C T O R Y   T R E E S
**
**	891209 SS	create by splitting from irmx.c
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
#include "irmx.h"

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


/* This default volinfo is for Series 4 / 310 low density format */
VolInfo LDf200e41 = {	/* in case we can't read track 0 */
	{'L', 'D', 'f','2', '0', '0', 'e', '4', '1', 0},/* vol_name */
	VF_DENSITY | VF_SIDES | VF_MINI,/* flags */
	4,								/* driver */			   	
	512,							/* size of volume block in bytes */
	325632,							/* size of volume in bytes */
	207,							/* # fnodes in fnode file */
	150528,							/* first byte in fnode file (0 orig) */
	128,							/* size of an fnode in bytes */
	6,								/* the fnode of the root directory */
	512,							/* size of a sector (except trk 0) */
	0,								/* block interleave */
	0, 								/* skew between tracks */
	0,								/* system ID */
	{'i', 'R', 'M', 'X', ' ', '8', '6', 0},
	{0},							/* zero on floppies */
};

VolInfo LDf050e41 = {	/* in case we can't read track 0 */
	{'L', 'D', 'f','0', '5', '0', 'e', '4', '1', 0},/* vol_name */
	VF_DENSITY | VF_SIDES | VF_MINI,/* flags */
	4,								/* driver */			   	
	512,							/* size of volume block in bytes */
	325632,							/* size of volume in bytes */
	57,								/* # fnodes in fnode file */
	312L * 512,						/* first byte in fnode file (0 orig) */
	128,							/* size of an fnode in bytes */
	6,								/* the fnode of the root directory */
	512,							/* size of a sector (except trk 0) */
	0,								/* block interleave */
	0, 								/* skew between tracks */
	0,								/* system ID */
	{'i', 'R', 'M', 'X', ' ', '8', '6', 0},
	{0},							/* zero on floppies */
};

/* This default volinfo is for Series 4 high density format */
VolInfo HDf200e41 = {	/* in case we can't read track 0 */
	{'H', 'D', 'f','2', '0', '0', 'e', '4', '1', 0},/* vol_name */
	VF_DENSITY | VF_SIDES | VF_MINI,/* flags */
	4,								/* driver */			   	
	512,							/* size of volume block in bytes */
	653312,							/* size of volume in bytes */
	207,							/* # fnodes in fnode file */
	614L * 512,						/* first byte in fnode file (0 orig) */
	128,							/* size of an fnode in bytes */
	6,								/* the fnode of the root directory */
	512,							/* size of a sector (except trk 0) */
	0, 								/* block interleave */
	0, 								/* skew between tracks */
	0,								/* system ID */
	{'i', 'R', 'M', 'X', ' ', '8', '6', 0},
	{0},							/* zero on floppies */
};

VolInfo HDf200e03 = {	/* in case we can't read track 0 */
	{'H', 'D', 'f','2', '0', '0', 'e', '0', '3', 0},/* vol_name */
	VF_DENSITY | VF_SIDES | VF_MINI,/* flags */
	4,								/* driver */			   	
	512,							/* size of volume block in bytes */
	653312,							/* size of volume in bytes */
	207,							/* # fnodes in fnode file */
	621L * 512,						/* first byte in fnode file (0 orig) */
	90,								/* size of an fnode in bytes */
	6,								/* the fnode of the root directory */
	512,							/* size of a sector (except trk 0) */
	0, 								/* block interleave */
	0, 								/* skew between tracks */
	0,								/* system ID */
	{'i', 'R', 'M', 'X', ' ', '8', '6', 0},
	{0},							/* zero on floppies */
};

VolInfo HDf002e41 = {	/* in case we can't read track 0 */
	{'H', 'D', 'f','0', '0', '2', 'e', '4', '1', 0},/* vol_name */
	VF_DENSITY | VF_SIDES | VF_MINI,/* flags */
	4,								/* driver */			   	
	512,							/* size of volume block in bytes */
	653312,							/* size of volume in bytes */
	2+7,							/* # fnodes in fnode file */
	630L * 512,						/* first byte in fnode file (0 orig) */
	128,							/* size of an fnode in bytes */
	6,								/* the fnode of the root directory */
	512,							/* size of a sector (except trk 0) */
	0, 								/* block interleave */
	0, 								/* skew between tracks */
	0,								/* system ID */
	{'i', 'R', 'M', 'X', ' ', '8', '6', 0},
	{0},							/* zero on floppies */
};

VolInfo HDf002e03 = {	/* in case we can't read track 0 */
	{'H', 'D', 'f','0', '0', '2', 'e', '0', '3', 0},/* vol_name */
	VF_DENSITY | VF_SIDES | VF_MINI,/* flags */
	4,								/* driver */			   	
	512,							/* size of volume block in bytes */
	653312,							/* size of volume in bytes */
	2+7,							/* # fnodes in fnode file */
	639L * 512,						/* first byte in fnode file (0 orig) */
	90,								/* size of an fnode in bytes */
	6,								/* the fnode of the root directory */
	512,							/* size of a sector (except trk 0) */
	0, 								/* block interleave */
	0, 								/* skew between tracks */
	0,								/* system ID */
	{'i', 'R', 'M', 'X', ' ', '8', '6', 0},
	{0},							/* zero on floppies */
};

#define CCINFO 0
#define LDINFO 1
#define MDINFO 3

extern char trkZeroBuf[];
int			drvVolInfo[MAXDRIVES] = {MDINFO, LDINFO, CCINFO};
VolInfo		*volInfos[] ={
	(VolInfo *) (trkZeroBuf + 384),
	&LDf200e41,
	&LDf050e41,
	&HDf200e41,
	&HDf200e03,
	&HDf002e41,
	&HDf002e03,
};
#define MAXINFOS (sizeof(volInfos) / sizeof(VolInfo *))

/*********************************************************************\
**
** dOpenXDir(Dir n)			read a directory file
**
**		dOpenXDir takes a Dir node that refers to a directory, and
**		reads that directory, constructing nodes for its immediate
**		subdirectories and ordinary files.
**
\*********************************************************************/

Object dOpenXDir(n)
	XDir n;
{
	register XDir dp;
	char c;
	XFile *f;
	DirEntry ent;
	Fnode fn_buf;

	f = xopenN(n -> x_dir.fnode, O_RDONLY);
	if (!f) return (NIL);

	while (xread(f, &ent, sizeof(ent))) {
		/* 
		** Skip entries with null name, zero inode, or
		** name starting with '.'
		*/
		if (!(c = ent.component[0]))
			continue;
		if (c == '.')
			continue;
		if (!(ent.fnode))
			continue;
		if (xstatN(ent.fnode, &fn_buf)) continue;

		dp = (XDir) cfNew(clXDir)(clXDir, n, 
								&ent.component[0],
								fn_buf.type == FT_DIR,
							    fn_buf.total_size
							   );
		if (!dp) {
			errorPrintf(
"Out of memory reading directory \"%s\"; tree will be incomplete.",
					   &ent.component[0]);
			break;
		}
		dp -> x_dir.fnode = ent.fnode;
		dp -> dir.time = fn_buf.mod_time /* + seconds from 1970 to 1978 */
			  + 3600L * 24L * (365L * 8 + 2);
	}
	xclose(f);
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
	Fnode fn_buf;

	dDrive = drive;
	sprintf(driveName, "%c:(%d)", drive + 'A', drive);
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
	xstatN(6, &fn_buf);
	n = (XDir) cfNew(cl)(cl, NIL, driveName, TRUE, fn_buf.total_size);
	dDrives[dDrive] = (Dir) n;
	if (!n) {
		errorPrintf(
"Out of memory creating root; tree will be empty."
				   );
		return ((Dir) n);
	}
	n -> x_dir.fnode = 6;
	n -> dir.time = fn_buf.mod_time /* + seconds from 1970 to 1978 */
		  + 3600L * 24L * (365L * 8 + 2);
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
		 * Allocate and initialize an fnode;
		 * Construct the directory entry.
		 */
		de.fnode = f -> x_dir.fnode = xcreatN(isDir);
		if (de.fnode == -1) return ((Dir) NIL);
		strncpy(de.component, name, sizeof(de.component));

#ifdef WDEBUG
	errorPrintf("Making dir entry: fnode=%d, name='%s', dir size = %ld", 
				de.fnode, name, d -> dir.size);
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
		 /* === ought to deal with binary and text modes here === */
		}
	if (!f -> dir.file)	{
		/* === doesn't create fnode and dir entry on write! === */
		f -> dir.file = (ulong) xopenN(f -> x_dir.fnode, m);
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
	Dir f;
{
	XFile *xf = (XFile *)f -> dir.file;

	if (! xf) return (FALSE);
	if (f -> dir.isChanged) {
		/*
		** Set the Dir's size from the file's fnode,
		** but set the file's date from the Dir if it's new.
		*/
		f -> dir.size = xf -> fnode.total_size;
		if (f -> dir.time == 0)
			f -> dir.time = xf -> fnode.mod_time /* secs 1970 to 1978 */
			  				+ 3600L * 24L * (365L * 8 + 2);
		else {
			xf -> fnode_dirty = TRUE;
			xf -> fnode.mod_time = f -> dir.time /* secs 1970 to 1978 */
			  					   - 3600L * 24L * (365L * 8 + 2);
		}
	}
	xclose(xf);
	f -> dir.file = 0;
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
	return (xread((XFile *) f -> dir.file, b, n));
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
	return (xwrite((XFile *)f -> dir.file, b, n));
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


/*********************************************************************\
**
** Classes
**
\*********************************************************************/

global DirClassRec crXDir = {
	&crClass,				/* class */				/* Object Part */
   {												/* Class Part */
	sizeof (XDirRec),		/* instance size */
	"iRMX",					/* class name */
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
	dirHeader,				/* header */
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
   },
};

Class clXDir   = (Class)&crXDir;

/*********************************************************************\
**
** Volume Format List Viewer
**
**		This viewer	is used to present the list of volume formats.
**
\*********************************************************************/

static Opaque vrVolGet(v)
	VolVr v;
{
	return ((Opaque) v -> volVr.cur);
}

static void vrVolSet(v, x)
	VolVr v;
	long x;
{
	v -> volVr.lim = MAXINFOS; 
	v -> volVr.cur = x;
}

static void vrVolCopy(v, o)
	VolVr v, o;
{
	*v = *o;
}

static void vrVolRewind(v)
	VolVr v;
{
	v -> volVr.cur = 0;
}

static Bool vrVolNext(v)
	VolVr v;
{
	if (v -> volVr.cur >= (v -> volVr.lim - 1))	return (FALSE);
	++ v -> volVr.cur;
	return (TRUE);
}
								  
static Bool vrVolPrev(v)
	VolVr v;
{
	if (v -> volVr.cur <= 0)	return (FALSE);
	-- v -> volVr.cur;
	return (TRUE);
}

static String vrVolString(v, w)
	VolVr v;
	int w;
{
	VolInfo *vi = volInfos[v -> volVr.cur];
	static char buf[256];

	if (vi -> max_fnode == 0) 
		sprintf(buf, "---- No volume info has been read ----");
	else {
	    sprintf(buf, 
			    "\"%-10.10s\" files=%03d es=%02d flags=%02x dg=%d vg=%d vs=%ld map@%d",
				vi -> vol_name,
				vi -> max_fnode - 7,
				vi -> fnode_size - sizeof(Fnode) + 5, /* === 5 is ad-hoc === */
				vi -> flags,
				vi -> dev_gran,
				vi -> vol_gran,
				vi -> vol_size,
				(int) (vi -> fnode_start / vi -> vol_gran)
			   );
		if (vi -> fnode_start % vi -> vol_gran) 
			sprintf(buf + strlen(buf), "+%d", 
					(int)(vi -> fnode_start % vi -> vol_gran));
	}
	return (buf);
}


VrClassRec crVolVr = {
	&crClass,				/* class */				/* Object Part */
   {												/* ObjectClass Part */
	sizeof (VolVrRec),		/* instance size */
	"VolViewer",			/* class name */
	&crObject,				/* super */
	NIL,					/* method dict */
	objInit,				/* initialize */
	vrNew,					/* new object */
	objClone,				/* clone self */
	objKill,				/* kill self */
	objDoesNotImplement,	/* open */
	objDoesNotImplement,	/* close */
	objName,				/* name */
   }, {												/* ViewClassPart */
	vrVolGet,				/* (v) -> current object */
	vrVolSet,				/* (v, ...) re-direct */
	vrVolCopy,				/* (v, src) -> v   copy src into v */
	vrVolRewind,			/* (v) go back to beginning */
	vrVolNext,				/* (v) -> success  advance one line */
	vrVolPrev,				/* (v) -> success  retreat one line */
	vrVolString,			/* (v, width) -> string to display */
   },
};
global Class clVolVr = (Class) &crVolVr;


