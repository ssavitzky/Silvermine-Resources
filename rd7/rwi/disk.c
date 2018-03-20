/***/static char *moduleID="disk 1.0 (c)1988 S. Savitzky";/***/

/*********************************************************************\
**
**	D I S K   H A N D L I N G
**
**		Here is where we read raw disks.
**
**	89-622 SS	create from Idris version
**	880111 SS	create PC version based on stuff from A. Savitzky
**
\*********************************************************************/

#include "coops.h"
#include "trees.h"
#include "dirs.h"
#include "viewer.h"

#include <dos.h>
#include <bios.h>
#include <stdio.h>
#include <signal.h>
#include <malloc.h>
#include "../lib/curse.h"

#undef  global
#define global
#include "disk.h"

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

/* BIOS-internal disk state info */

#define DSK_STATE ((char *)0x00400090L)
#define DS_360_360EST	0x03
#define DS_360_1200EST	0x04
#define DS_1200_1200EST	0x05

#define DS_ESTAB		0x10
#define DS_DBLSTEP		0x20
#define DS_XFER500		0x00
#define DS_XFER300	 	0x40
#define DS_XFER250		0x80		/* for 360K disks */

#define DS_DD (DS_360_1200EST | DS_ESTAB | DS_DBLSTEP | DS_XFER300) /*360K*/
#define DS_MD (DS_360_1200EST | DS_ESTAB | DS_XFER300)				/*720K*/
#define DS_HD (DS_1200_1200EST | DS_ESTAB | DS_XFER500)				/*1200K*/


/*********************************************************************\
**
** Default drive information
**
\*********************************************************************/

global int dDriveType[MAXDRIVES] = {
 /* A */	DRV_96TPI | DRV_HIDENS, 			  /* AT drive */
 /* B */	0,									   /* PC drive */
 /* C */	DRV_HARD,
 /* D */	DRV_HARD,
 /* E */	DRV_HARD,
};

global int dPhysDrive[MAXDRIVES] = {
 /* A	B	C	D	E	... */
	0,	1,	-1,	-1,	-1,
};

global int dMediaType[MAXDRIVES] = {
	MT_MD,	MT_DD,
};

global unsigned dMaxDrives = MAXDRIVES;

global int dDrive;				/* current drive (set by dInitDrive) */
static int dSecLen;				/* sector length of current drive */
static int dSpt;				/* sectors/track of current drive */

global int dErrorsExpected = 0;		/* don't complain about errors */

/*********************************************************************\
**
** Debugging kludgery
**
\*********************************************************************/

void dstat(col, str)
	int col;
	char *str;
{
#ifdef DDEBUG
	move(0, col);
	addstr(str);
	refresh();
#endif /* DDEBUG */
}

void dstatn(col, str, n)
	int col;
	char *str;
	int n;
{
#ifdef DDEBUG
	char buf[20];
	sprintf(buf, str, n);
	dstat(col, buf);
#endif /* DDEBUG */
}


/*********************************************************************\
**
** S E C T O R S   &   P H Y S I C A L   I / O
**
\*********************************************************************/

/* 
** This is the magic required to trick the BIOS into reading
** odd-sized sectors
*/

union REGS inregs, outregs;
struct SREGS segregs;

typedef struct diskbase {
	char	steptime,
			headtime,
			waittime,
			seclen,			/* 0: 128, 1: 256, 2: 512, 3: 1024 */
			maxsec,
			gaplen,
			datalen,
			gaplenFormat,
			dataFormat,
			headsettle,
			motorstart;
} DiskBase, far* DskBasPtr;

DiskBase mybase, far*oldbase = 0L, far*far*int30vec;


/*********************************************************************\
**
** Stuff to restore disk info after a ctrl-C
**
\*********************************************************************/

void *oldSigInt;		/* control-C signal stash */

void sigIntHdlr()
{
	dCloseDisk();		/* put things back as they were */
	exit(1)	;
}


/*********************************************************************\
**
** dInitDrive(drive, seclen, spt, dens)	-- initialize BIOS table
**
\*********************************************************************/

void dInitDrive(drv, seclen, spt, dens)
	int drv;					/* drive number */
	int seclen;					/* sector length in bytes */
	int spt;					/* sectors per track */
	int dens;					/* 0 = low, 1 = double (normal), 2 = high */
{
	int i;
	char ds;

	/* set up the disk base pointer */

	int30vec = (DskBasPtr far*)120L; 	/* interrupt 30 vector */
	if (!oldSigInt) oldSigInt = signal(SIGINT, sigIntHdlr);

#ifdef DDEBUG
 	printf("int30vec = %lx, *int30vec = %lx\n", 
 		   (long)int30vec, 
 		   (long)*int30vec);
#endif /* DDEBUG */

	if (*int30vec != &mybase) {
		oldbase = *int30vec;			/* save ptr to old disk base table */
		mybase = *oldbase;				/* copy the old table into mine */
	}

	mybase.seclen = seclen == 128? 0 :
					seclen == 256? 1 :
					seclen == 512? 2 : 3;
	mybase.maxsec = spt;    			/* sectors per track */
/*	mybase.gaplen = 42;	*/				/*as-try */
	mybase.datalen = seclen;
/*	mybase.headsettle =15; */
	mybase.motorstart = 8; 

	*int30vec = &mybase;				/* point int 30 at new table */
	inregs.h.ah = 0;					/* reset */
	int86(0x13, &inregs, &outregs);

	if (dDriveType[drv] & DRV_COMPAT) {
		inregs.h.ah = 8;
		inregs.h.dl = dPhysDrive[drv];
		int86(0x13, &inregs, &inregs);
		inregs.h.ah = 9;
		inregs.h.dl = dPhysDrive[drv];
		inregs.h.cl = dens;
		inregs.h.ch = dMediaType[drv] & MT_96TPI? 0 : 1;
		int86(0x13, &inregs, &inregs);
	} else if (dDriveType[drv] & DRV_96TPI) {
/* === this stuff attempted to set media type, but it didn't work ===
		inregs.h.ah = 23;
		inregs.h.al = dMediaType[drv];
		inregs.h.dl = dPhysDrive[drv];
		int86(0x13, &inregs, &inregs);
*/ /* === so we do this instead === */
		ds = DS_DD;
		if (dMediaType[drv] & MT_HIDENS) ds = DS_HD;
		else if (dMediaType[drv] & MT_96TPI) ds = DS_MD;
		DSK_STATE[dPhysDrive[drv]] = ds;
		/* Now check to see if the door opened */
		inregs.h.ah = 22;
		int86(0x13, &inregs, &inregs);
		if (inregs.h.ah != 0) {
			static char buf[1024];
			/* Dummy read forces BIOS to check & reset door state */
			++ dErrorsExpected;
			dRdSector(buf, 0, 1, 1, 1);
			-- dErrorsExpected;
			/* ... which forgets media info, so re-set it */
			DSK_STATE[dPhysDrive[drv]] = ds;
		}
	}

	dDrive  = drv;
	dSecLen = seclen;
	dSpt    = spt;
}

/*********************************************************************\
**
** dCloseDisk()	restore disk table to its original state
**
\*********************************************************************/

void dCloseDisk()
{
	if (oldbase) {
		*int30vec = oldbase;		/* put things back as they were */
		DSK_STATE[0] = 0;
		DSK_STATE[1] = 0;
	}
	if (oldSigInt)
		signal(SIGINT, oldSigInt);
}


/*********************************************************************\
**
** int checkError(retval, track, side, sector, op)
**
**		Check a _bios_disk return value for an error.
**		If it's an error, call errorPrintf and return the retval,
**		Else return zero.
**
\*********************************************************************/

static int checkError(retval, track, side, sector, op)
	int retval, track, side, sector;
	char *op;
{
	char *es;

	if ((retval & 0xFF00) != 0) {
		++dErrors;
		if (dErrorsExpected) return (retval);
		switch ((retval >> 8) & 0xff) {
		 case 0x01:	es = "Invalid request";			break;
		 case 0x02: es = "Address mark not found"; 	break;
		 case 0x04:	es = "Sector not found";		break;
		 case 0x05:	es = "Reset failed";			break;
		 case 0x07:	es = "Drive parameter activity failed";	break;
		 case 0x09:	es = "DMA overrun";				break;
		 case 0x0A:	es = "Bad sector flag";			break;
		 case 0x10:	es = "ECC Error";				break;
		 case 0x11:	es = "Corrected ECC Error";		break;	/* === ? === */
		 case 0x20:	es = "Controller failure";		break;
		 case 0x40:	es = "Seek error";				break;
		 case 0x80:	es = "Disk timed out";			break;
		 case 0xAA:	es = "Drive not ready";			break;
		 case 0xBB:	es = "Undefined error";			break;
		 case 0xCC:	es = "Write fault";				break;
		 case 0xE0:	es = "Status error";			break;
		 default:	es = "Unknown error";			break;
		}
		errorPrintf("%s Error: %s (%04x), trk: %d hd: %d sec: %d",
					op, es, retval, track, side, sector);
		return (retval);
	} else {
		return (0);
	}
}


/*********************************************************************\
**
** errcode = dRdSector(buf, track, side, sector, count)	-- read sectors
**
**		Automatically steps to next side and track if necessary;
**		note that this is incorrect if the drive is single-sided.
**
\*********************************************************************/

int dRdSector(bufr, track, side, sector, count)
	char *bufr;
	int side, track, sector, count;
{
	int try, retval, c;
	static struct diskinfo_t diskinfo;
#define MAXTRIES 8

	if (side < 0 || track < 0 || sector <= 0 ||
		side > 1 || sector > dSpt || track > 80) {
		errorPrintf("Bad read: track: %d, side: %d, sector: %d", 
		   	track, side, sector);
		return (-1);
	}
	*int30vec = &mybase;
	diskinfo.drive = dPhysDrive[dDrive];
	diskinfo.head = side;
	diskinfo.track = track;
	diskinfo.sector = sector;
	diskinfo.nsectors = 1;
	diskinfo.buffer = bufr;
	for (c = 0; c < count; ++c)	{
#ifdef DDEBUG
		errorPrintf("track: %d, side: %d, sector: %d, dest: %lx", 
		   	track, side, diskinfo.sector, diskinfo.buffer);
#endif /* DDEBUG */
		for (try = 1; try <= MAXTRIES; ++try) {
			if (try == 2)  _bios_disk(_DISK_RESET, &diskinfo);
			retval = _bios_disk(_DISK_READ, &diskinfo);
			if ((retval & 0xFF00) == 0) break;
			if (try == MAXTRIES) goto bomb;
		}
		diskinfo.buffer = (bufr += dSecLen);
		if (++diskinfo.sector > dSpt) {
			diskinfo.sector = 1;
			if (++diskinfo.head > 1) {	/* === LOSE if single sided === */
				diskinfo.head = 0;
				++diskinfo.track;
			}
		}
	}
bomb:
	*int30vec = oldbase;	/* In case copying from floppy to floppy */

	/*
	** Now check the error code.
	*/
#ifdef DDEBUG
	errorPrintf("retval = %x", retval);
#endif
	if (checkError(retval, track, side, sector + c, "Read")) {
		signal(SIGINT, oldSigInt);
		return(retval);
	}
	return(0);
}

/*********************************************************************\
**
** errcode = dWrSector(buf, track, side, sector, count)	-- write sector
**
\*********************************************************************/

int dWrSector(bufr, track, side, sector, count)
	char *bufr;
	int side, track, sector, count;
{
#if 1
	int try, retval, c;
	static struct diskinfo_t diskinfo;

	*int30vec = &mybase;
	diskinfo.drive = dPhysDrive[dDrive];
	diskinfo.head = side;
	diskinfo.track = track;
	diskinfo.sector = sector;
	diskinfo.nsectors = 1;
	diskinfo.buffer = bufr;
	for (c = 0; c < count; ++c)	{
#ifdef DDEBUG
		errorPrintf("WRITE track: %d, side: %d, sector: %d, dest: %lx\n", 
		   	track, side, diskinfo.sector, diskinfo.buffer);
		if (track < 0 || sector <= 0 || sector > 9) {
			dCloseDisk();
			errorPrintf(stderr, "bogus write to track %d, sector %d\n", 
					track, sector);
			return (-1);
		}
#endif /* DDEBUG */
		for (try = 1; try <= MAXTRIES; ++try) {
			if (try == 2)  _bios_disk(_DISK_RESET, &diskinfo);
			retval = _bios_disk(_DISK_WRITE, &diskinfo);
			if ((retval & 0xFF00) == 0) break;
			if (try == MAXTRIES) goto bomb;
		}
		diskinfo.buffer = (bufr += dSecLen);
		if (++diskinfo.sector > dSpt) {
			diskinfo.sector = 1;
			if (++diskinfo.head > 1) {	/* === LOSE if single sided === */
				diskinfo.head = 0;
				++diskinfo.track;
			}
		}
	}
bomb:
	*int30vec = oldbase;	/* In case copying from floppy to floppy */

	/*
	** Now check the error code.
	*/
#ifdef DDEBUG
	errorPrintf("retval = %x", retval);
#endif
	if (checkError(retval, track, side, sector + c)) {
		signal(SIGINT, oldSigInt);
		return(retval);
	}
	return(0);

#else			/* === OLD STUFF === */

	int maxtries = 8, try, retval;
	static struct diskinfo_t diskinfo;
	
#ifdef DDEBUG
	errorPrintf("WRITE track: %d, side: %d, sector: %d, dest: %lx\n", 
	   	track, side, sector, bufr);
	if (track < 0 || sector <= 0 || sector > 9) {
		dCloseDisk();
		errorPrintf(stderr, "bogus write to track %d, sector %d\n", 
				track, sector);
		return (-1);
	}
#endif /* DDEBUG */

	*int30vec = &mybase;
	for (try = 0; try < maxtries; ++try) {
		if (try == 1) _bios_disk(_DISK_RESET, &diskinfo);
		diskinfo.drive = dDrive;
		diskinfo.head = side;
		diskinfo.track = track;
		diskinfo.sector = sector;
		diskinfo.nsectors = count;
		diskinfo.buffer = bufr;
		retval = _bios_disk(_DISK_WRITE, &diskinfo);
		if ((retval & 0xFF00) == 0) break;
	}
	*int30vec = oldbase;	/* In case copying from floppy to floppy */

	/*
	** Now check the error code.
	*/
	if (checkError(retval, track, side, sector, "Write")) {
		signal(SIGINT, oldSigInt);
		return(retval);
	}
	return(0);
#endif
}

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
			t -> dir.mode       = binary;
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
				dFileSys[d -> dsk_dir.drive]
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
	(Class)&crDir,			/* class */				/* Object Part */
   {												/* Class Part */
	sizeof (DskDirRec),		/* instance size */
	"DskDir",				/* class name */
	&crObject,				/* super */
	NIL,					/* method dict */
	objInit,				/* initialize */
	dirNew,					/* new object */
	objClone,				/* clone self */
	dirKill,				/* kill self */
	objDoesNotImplement,	/* open */
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
   },
};

Class clDskDir   = (Class)&crDskDir;


/*********************************************************************\
**
** Drive Viewer
**
**		This viewer	is intended to make it easy to add drive-specific
**		information.  Just modify vrDriveString() (below) to display
**		it, make initDrive() do the appropriate setup, and add a menu
**		in DATA.C to set it.
**
\*********************************************************************/

static Opaque vrDriveGet(v)
	DriveVr v;
{
	return ((Opaque) v -> driveVr.cur);
}

static void vrDriveSet(v, x)
	DriveVr v;
	String *x;
{
	v -> driveVr.lim = dMaxDrives; 
	v -> driveVr.cur = 0;
}

static void vrDriveCopy(v, o)
	DriveVr v, o;
{
	*v = *o;
}

static void vrDriveRewind(v)
	DriveVr v;
{
	v -> driveVr.cur = 0;
}

static Bool vrDriveNext(v)
	DriveVr v;
{
	if (v -> driveVr.cur >= (v -> driveVr.lim - 1))	return (FALSE);
	++ v -> driveVr.cur;
	return (TRUE);
}
								  
static Bool vrDrivePrev(v)
	DriveVr v;
{
	if (v -> driveVr.cur <= 0)	return (FALSE);
	-- v -> driveVr.cur;
	return (TRUE);
}

static String vrDriveString(v, w)
	DriveVr v;
	int w;
{
	int d = v -> driveVr.cur;
	int dt = dDriveType[d];
	static char buf[81];
	extern int dOutputDrive, dInputDrive;

	if (dt & DRV_HARD) {
		sprintf(buf, "%s%c:\thard disk:      media=hard, %s",
				d == dOutputDrive - 1? (d == dInputDrive -1? "<>" : "->") :
				 d == dInputDrive - 1? "<-" : "  ",
				d + 'A',
				dFileSys[d]	? dFileSys[d] : "DOS"
			   );
	} else {
		sprintf(buf, "%s%c:\tdrive #%2d: %1ds%cd media=%cs%cd, %s %s",
				d == dOutputDrive - 1? (d == dInputDrive -1? "<>" : "->") :
				 d == dInputDrive - 1? "<-" : "  ",
				d + 'A', dPhysDrive[d],
				dt & DRV_SS ? 1 : 2, 
				dt & DRV_SD? 'S' :
				dt & DRV_HIDENS? 'H' : 'D',
				dMediaType[d] & MT_SS? '1' : '2',
				dMediaType[d] & MT_HIDENS? 'H' :
				dMediaType[d] & MT_96TPI?  'M' : 'D',
				dFileSys[d]	? dFileSys[d] : "DOS",
				dt & DRV_COMPAT? "CompatiCard" : ""
			   );
	}

	return (buf);
}


VrClassRec crDriveVr = {
	&crClass,				/* class */				/* Object Part */
   {												/* ObjectClass Part */
	sizeof (DriveVrRec),		/* instance size */
	"DriveViewer",			/* class name */
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
	vrDriveGet,				/* (v) -> current object */
	vrDriveSet,				/* (v, ...) re-direct */
	vrDriveCopy,				/* (v, src) -> v   copy src into v */
	vrDriveRewind,			/* (v) go back to beginning */
	vrDriveNext,				/* (v) -> success  advance one line */
	vrDrivePrev,				/* (v) -> success  retreat one line */
	vrDriveString,			/* (v, width) -> string to display */
   },
};
global Class clDriveVr = (Class) &crDriveVr;


