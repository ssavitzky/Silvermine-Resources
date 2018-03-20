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
#include "tree.h"
#include "dir.h"
#include "viewer.h"

#include <dos.h>
#include <bios.h>
#include <stdio.h>
#include <signal.h>
#include <malloc.h>

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
static int dDens;

global int dErrorsExpected = 0;		/* don't complain about errors */
global int diskStateChanged = FALSE;

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

void (*oldSigInt)();		/* control-C signal stash */

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
		diskStateChanged = TRUE;
		/* Now check to see if the door opened */
		inregs.h.ah = 22;
		int86(0x13, &inregs, &inregs);
		if (inregs.h.ah != 0) {
			static char buf[1024];
			/* Dummy read forces BIOS to check & reset door state */
			/* Read track 1, side 0 to avoid media oddities.      */
			/* === Ought to check error code for media info	===	  */ 
			++ dErrorsExpected;
			dRdSector(buf, 1, 0, 1, 1);
			-- dErrorsExpected;
			/* ... which forgets media info, so re-set it */
			DSK_STATE[dPhysDrive[drv]] = ds;
			diskStateChanged = TRUE;
		}
	}

	dDrive  = drv;
	dSecLen = seclen;
	dSpt    = spt;
	dDens	= dens;
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
		if (diskStateChanged) {
			DSK_STATE[0] = 0;
			DSK_STATE[1] = 0;
			diskStateChanged = FALSE;
		}
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
	int try, retval=0, c;
	static struct diskinfo_t diskinfo;
#define MAXTRIES 10

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
			if (try == 2) {
				_bios_disk(_DISK_RESET, &diskinfo);
				if (dDriveType[dDrive] & DRV_COMPAT) {
					dInitDrive(dDrive, dSecLen, dSpt, dDens);
				}
			}
			retval = _bios_disk(_DISK_READ, &diskinfo);
			if ((retval & 0xFF00) == 0) break;
			if (try == MAXTRIES) goto bomb;
		}
		diskinfo.buffer = (bufr += dSecLen);
		if (++diskinfo.sector > dSpt) {
			diskinfo.sector = 1;
			if (++diskinfo.head > 1 || dMediaType[dDrive] & MT_SS) {
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
		_bios_disk(_DISK_RESET, &diskinfo);
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
			if (++diskinfo.head > 1 || dMediaType[dDrive] & MT_SS) {
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

	int maxtries = 10, try, retval;
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
				dFileSys[d]	? gName(dFileSys[d]) : "DOS "
			   );
	} else {
		sprintf(buf, "%s%c:\tdrive #%2d: %1ds%cd media=%cs%cd, %s %s",
				d == dOutputDrive - 1? (d == dInputDrive -1? "<>" : "->") :
				 d == dInputDrive - 1? "<-" : "  ",
				d + 'A', dPhysDrive[d],
				dt & DRV_SS ? 1  : 2, 
				dt & DRV_SD? 	 'S' :
				dt & DRV_HIDENS? 'H' : 'D',
				dMediaType[d] & MT_SS? '1' : '2',
				dMediaType[d] & MT_HIDENS? 	'H' :
				dMediaType[d] & MT_SD? 		'S' :
				dMediaType[d] & MT_96TPI?  	'M' : 'D',
				dFileSys[d]	? gName(dFileSys[d]) : "DOS ",
				dt & DRV_COMPAT? "Compatibility" : ""
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


/*********************************************************************\
**
** Drive Classifier
**
**		Identify ordinary floppies (A and B), hard disks, and
**		compaticard drives.  Obtain their physical drive numbers.
**
\*********************************************************************/

void dClassifyDrives()
{
    unsigned status = 0;
    struct diskinfo_t di;
    struct diskfree_t df;
    unsigned char far *p, linebuf[17];
	int m, n;
    struct diskfree_t drvinfo;
    unsigned drive, drivecount, memory, pstatus;
	static char diskbuf[512];
    union {                             /* Access equiment either as:    */
		unsigned u;                     /*   unsigned or                 */
		struct {                        /*   bit fields                  */
	    	unsigned diskflag : 1;      /* Diskette drive installed?     */
	    	unsigned coprocessor : 1;   /* Coprocessor? (except on PC)   */
	    	unsigned sysram : 2;        /* Ram on system board           */
	    	unsigned video : 2;         /* Startup video mode            */
	    	unsigned disks : 2;         /* Drives 00=1, 01=2, 10=3, 11=4 */
	    	unsigned dma : 1;           /* 0=Yes, 1=No (1 for PC Jr.)    */
	    	unsigned comports : 3;      /* Serial ports                  */
	    	unsigned game : 1;          /* Game adapter installed?       */
	    	unsigned modem : 1;         /* Internal modem?               */
	    	unsigned printers : 2;      /* Number of printers            */
		} bits;
    } equip;

	di.head = 0;
	di.track = 0;
	di.sector = 1;
    di.nsectors = 1;
    di.buffer   = diskbuf;

	/* 
	 * Find out how many drives DOS thinks it's using
	 * Then find out how many floppies the BIOS knows about.
	 */
	_dos_getdrive(&dWorkingDrive);
	_dos_setdrive(dWorkingDrive, &dMaxDrives);
	equip.u = _bios_equiplist();
	di.drive = equip.bits.disks + 1;

#if 1
	/* Use int 21 function 4408 to find first non-hard disk */
	inregs.h.ah = 0x44;
	inregs.h.al = 0x08;
	for (n = 3; n <= dMaxDrives; ++n) {
		inregs.h.bl = n;
		if (intdos(&inregs, &outregs) == 0 
		    && outregs.x.cflag == 0) 
			break;
	}

#else
	/* Use Int 13 function 8 to get hard disk parameters. */
	inregs.h.ah = 8;
	inregs.h.dl = 0x80;
	int86x(0x13,&inregs,&outregs,&segregs);
	n = 3 + outregs.h.dl;
#endif
#if 1
	/*
	 * Start with drive n and look for a compaticard.
	 * 		n = drive number (letter - 'a' + 1)
	 *		m = last-used physical unit number
	 */
	for (m = di.drive; n <= dMaxDrives; ++n) {
		for ( ; m < 40; ++m) {
			inregs.h.ah = 8;
			inregs.h.dl = m;
			int86x(0x13,&inregs,&outregs,&segregs);
			if (((outregs.h.bl == 1) || (outregs.h.bl == 2)) &&
					 !outregs.x.cflag) {
				dPhysDrive[n-1] = m;
				dDriveType[n-1] |= DRV_COMPAT;
				dDriveType[n-1] &= ~DRV_HARD;
				++m;
				break;
			}
		}
	}
#endif
#if 0
	for (n = dMaxDrives-1, m = 15; m > di.drive; m--) {
		status = /* _bios_disk(_DISK_RESET,&di) */ 0;
		if (!status) {
			inregs.h.ah = 8;
			inregs.h.dl = m;
			int86x(0x13,&inregs,&outregs,&segregs);
			if (((outregs.h.bl == 1) || (outregs.h.bl == 2)) &&
					 !outregs.x.cflag) {
				dPhysDrive[n] = m;
				dDriveType[n] |= DRV_COMPAT;
				dDriveType[n] &= ~DRV_HARD;
				--n;
			}
		}
	}
#endif
}


