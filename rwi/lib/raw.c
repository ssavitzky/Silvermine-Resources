/***/static char *moduleID="raw 1.0 (c)1988 S. Savitzky";/***/

/*********************************************************************\
**
**	D I S K   H A N D L I N G
**
**		Here is where we read raw disks.
**
**	891209 SS	split out of disk.c
**
\*********************************************************************/

#include "coops.h"
#include "tree.h"
#include "dir.h"
#include "viewer.h"
#include "view.h"
#include "disk.h"

#include <dos.h>
#include <bios.h>
#include <stdio.h>
#include <malloc.h>
#include "curse.h"

#undef  global
#define global
#include "raw.h"

extern Object objInit(), objClone(), objKill(), objNew();
extern Object objDoesNotImplement(), objRetSelf();
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

/*********************************************************************\
**
** Set raw disk parameters
**
**		If the Dir being modified is the root, the parameters are
**		propagated to the track nodes.
**
\*********************************************************************/

global Dir dSetRaw(n, dens, ds, spt, seclen)
	DskDir n;
	int dens, ds, spt, seclen;
{
	DskDir  t;

	/*
	** Mung Root Node
	*/
	n -> dsk_dir.seclen = seclen;
	n -> dsk_dir.spt    = spt;
	n -> dsk_dir.dens   = dens;
	n -> dsk_dir.ds     = ds;
	n -> dir.size =	seclen * spt;

	/* 
	** Mung kids, if any
	*/
	for (t = (DskDir) gDown(n); t; t = (DskDir) gNext(t)) 
		dSetRaw(t, dens, ds, spt, seclen);

	return ((Dir) n);
}


/*********************************************************************\
**
** R E A D   R A W   D I S K
**
**		This section supports the reading of raw physical disks.
**		You read them a track at a time.
**
**		dReadRaw doesn't actually read anything, it just constructs
**		a phony "directory" in which each track is a file.
**
\*********************************************************************/


Dir dReadRaw(cl, drive)
	Class cl;
	int  drive;
{
	char driveName[16];
	char nam[16];
	DskDir  n, t;
	int h, i;
	ulong s;
	int ntrks, dens;

	++inUse[drive];
	sprintf(driveName, "%c:(%d)", drive + 'A', drive);

	dErrors = 0;

	/*
	** Construct Root Node
	*/
	n = (DskDir) cfNew(cl)(cl, NIL, driveName, TRUE, 0L);
	if (NOTNULL(dDrives[drive])) gKill(dDrives[drive]);
	dDrives[drive] = (Dir) n;
	if (!n) {
		errorPrintf(
"Out of memory creating root; tree will be empty."
				   );
		return ((Dir) n);
	}
	n -> dsk_dir.drive  = drive;
	n -> dsk_dir.track  = 0;
	n -> dsk_dir.ds     = 1;
	n -> dsk_dir.head	= 0;
	if (dEachNewDir) (*dEachNewDir)(n);

	/* 
	** construct tree
	**		Check drive info for initial parameters.
	*/
	s = n -> dsk_dir.seclen * n -> dsk_dir.spt;
	if (dDriveType[drive] & DRV_96TPI)  ntrks = 80;
	else								ntrks = 40;
	for (i = 0; i < ntrks; ++i) 
		for (h = 0; h <= n -> dsk_dir.ds; ++h) {
			sprintf(nam, "t%02d.%d", i, h);
			t = (DskDir) cfNew(clDskDir)(clDskDir, n, nam, FALSE, s);
			if (!t) {
				errorPrintf(
		"Out of memory creating %s; tree will be incomplete."	, nam
						   );
				return ((Dir) n);
			}
			t -> dsk_dir.drive  = drive;
			t -> dsk_dir.track  = i;
			t -> dsk_dir.head   = h;
			t -> dir.ftype      = binary;
		}
	dSetRaw(n, 1, 1, 8, 512);
	if (dEachNewDir) (*dEachNewDir)(t);
errorExit:
	return ((Dir) n);
}


/*********************************************************************\
**
** char huge *dReadTrack(f)
**
**		Suck some sectors into a buffer.
**		assumes that read is multiple of sector size.
**
\*********************************************************************/

int dReadTrack(f, b, n)
	DskDir f;
	char huge *b;
	int	n;
{
	ulong size;
	short sec, count;

	if (n == 0) {
		f -> dsk_dir.loc = 0L;
		return (0);			/* fake a close */
	}

	size = f -> dir.size;
	if (size > n) size = n;
	memset(b, 0, (size_t)size);

	sec = f -> dsk_dir.loc / f -> dsk_dir.seclen;
	count = size / f -> dsk_dir.seclen;
	if (count + sec > f -> dsk_dir.spt) {
		count = (f -> dsk_dir.spt - sec);
		size  = count * f -> dsk_dir.seclen;
	}

	dInitDrive(f -> dsk_dir.drive, f -> dsk_dir.seclen, 
		   	   f -> dsk_dir.spt,   f -> dsk_dir.dens);
	dRdSector(b, f -> dsk_dir.track, f -> dsk_dir.head, sec + 1, count);
	f -> dsk_dir.loc += size;
	return (size);
}

/*********************************************************************\
**
** header line for track listing
**
\*********************************************************************/

String dskDirHeader(d, buf, len)
	DskDir 	d;
	String	buf;
	Cardinal len;
{
	register int i;
	struct tm *tm;

	if (!d -> dir.isDir && len < 38) 
		treeHeader(d, buf, len);
	else if (d -> dir.isDir) {
		sprintf(buf, "%s, phys: %d", 
				gName(d), dPhysDrive[d -> dsk_dir.drive]);
		sprintf(buf + strlen(buf), " %1ds%cd %02ds*%04d %s",
				d -> dsk_dir.ds + 1, 
				d -> dsk_dir.dens == 0? 's' :
				d -> dsk_dir.dens == 1? 'd' : 'h', 
				d -> dsk_dir.spt, d -> dsk_dir.seclen,
				gName(dFileSys[d -> dsk_dir.drive])
			   );
	} else {
		sprintf(buf, "%s%c", gName(d), d ->dir.tcount? '*' : ' ');
		sprintf(buf + strlen(buf), " %1ds%cd %02ds*%04d %08ld b#=%d",
				d -> dsk_dir.ds + 1, 
				d -> dsk_dir.dens == 0? 's' :
				d -> dsk_dir.dens == 1? 'd' : 'h', 
				d -> dsk_dir.spt, d -> dsk_dir.seclen,
				d -> dir.size,
				d -> dsk_dir.spt * (d -> dsk_dir.track * 2 + d -> dsk_dir.head)
					- 4  /* KLUDGE for iRMX-86 disks with LD track 0 */
				);
	}
}

global unsigned dValidateRaw(d)
	Dir d;
{
	char c;

	if (ISNULL(d)) return (FALSE);
	return (TRUE);			/* === should check for drive ready! === */
}

static Bool dNop(d)
	DskDir d;
{
	d -> dsk_dir.loc = 0L;
	return (TRUE);
}

static Dir dCreateRaw(d)
	Dir d;
{
	objDoesNotImplement();
	return (0);
}

static int dWriteTrack(d)
	Dir d;
{
	objDoesNotImplement();
	return (0);
}

static Bool dSeekRaw(d, n)
	DskDir d;
	long n;
{
	return (d -> dsk_dir.loc = n);
}

static long dTellRaw(d)
	DskDir d;
{
	return (d -> dsk_dir.loc);
}

/*********************************************************************\
**
** Classes
**
\*********************************************************************/

global DirClassRec crDskDir = {
	&crClass,				/* class */				/* Object Part */
   {												/* Class Part */
	sizeof (DskDirRec),		/* instance size */
	"RAW",					/* class name */
	(Class)&crDir,			/* super */
	NIL,					/* method dict */
	objInit,				/* initialize */
	dirNew,					/* new object */
	objClone,				/* clone self */
	dirKill,				/* kill self */
	objRetSelf,				/* open */
	objRetSelf,				/* close */
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
	dskDirHeader,			/* header */
	treePath,				/* pathname */
	treeFind,				/* find by name */
   }, {
   	16,						/* max name length */
   	0,
    dReadRaw,
	dCreateRaw,				/* create */
	dNop,					/* open */
	dNop,					/* close */
	dReadTrack,				/* read file */
	dWriteTrack,			/* write file */
	dValidateRaw,
	dSeekRaw,				/* seek */
	dTellRaw,				/* tell */
	/* === rename === */
	/* === unlink === */
   },
};

Class clDskDir   = (Class)&crDskDir;


/*********************************************************************\
**
** Raw Disk Command Handlers
**
\*********************************************************************/

extern Dir curFile();
extern int scrOpen();
extern void *theView, *theScreen;

/*
** These set various raw disk parameters of the current disk or track
** Since choices are limited, they are done from a submenu.
*/
int cRawDens(s)
	char *s;
{
	DskDir d = (DskDir) (curFile(theView));

	dSetRaw(d, 
		    atoi(s),
		    d -> dsk_dir.ds,
			d -> dsk_dir.spt,
			d -> dsk_dir.seclen);

	return (scrOpen(theScreen));
}

int cRawDs(s)
	char *s;
{
	DskDir d = (DskDir) (curFile(theView));

	dSetRaw(d, 
		    d -> dsk_dir.dens,
		    atoi(s),
			d -> dsk_dir.spt,
			d -> dsk_dir.seclen);

	return (scrOpen(theScreen));
}

int cRawSpt(s)
	char *s;
{
	DskDir d = (DskDir) (curFile(theView));

	dSetRaw(d, 
		    d -> dsk_dir.dens,
			d -> dsk_dir.ds,
		    atoi(s),
			d -> dsk_dir.seclen);

	return (scrOpen(theScreen));
}

int cRawSeclen(s)
	char *s;
{
	DskDir d = (DskDir) (curFile(theView));

	dSetRaw(d, 
		    d -> dsk_dir.dens,
			d -> dsk_dir.ds,
			d -> dsk_dir.spt,
		    atoi(s));

	return (scrOpen(theScreen));
}


