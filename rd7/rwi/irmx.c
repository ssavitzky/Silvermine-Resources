/***/static char *moduleID="disk 1.0 (c)1988 S. Savitzky";/***/

/*********************************************************************\
**
**	D I S K   H A N D L I N G
**
**		Here is where we read iRMX-format disks, files, and directories.
**
**	890622 SS	create from Idris version
**	880111 SS	create PC version based on stuff from A. Savitzky
**
** NOTES:
**		We assume that the fnode file, fnode map, and free block map
**		are contiguous, and small enough to fit in memory all at once.
**
**		We don't use the bad-block map.
**
**		We only write short-format files.  
**
\*********************************************************************/

#include "coops.h"
#include "trees.h"
#include "dirs.h"
#include "viewer.h"

#include <dos.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include <signal.h>
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


/*********************************************************************\
**
** The volume information record and related stuff
**
\*********************************************************************/

char		  blockBuf[MAXBLOCK];
char		  trkZeroBuf[T0_BYTES];
DWORD		  trkZeroSize = T0_BYTES;
DWORD		  trkZeroBlks = 16;

VolInfo		  volInfoBuf;		/* a volume label */
VolInfo		 *curVolInfo;		/* -> current volume label */
char		 *fnodeBuf;			/* The fnode file or portion thereof */
int			  fnodeOrig;		/* # of first fnode in buffer */
int			  fnodeCount;		/* # of	fnodes in buffer */
DWORD		  fnodeBlk;			/* block number of start of Fnode file */
int			  fnodeBc;			/* block count of Fnode file */

static BYTE *blkMap, *fnodeMap;	/* fnode and block allocation maps */

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
	614L * 512,						/* first byte in fnode file (0 orig) */
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

VolInfo HDf002e03 = {	/* in case we can't read track 0 */
	{'H', 'D', 'f','0', '0', '2', 'e', '0', '3', 0},/* vol_name */
	VF_DENSITY | VF_SIDES | VF_MINI,/* flags */
	4,								/* driver */			   	
	512,							/* size of volume block in bytes */
	653312,							/* size of volume in bytes */
	2+7,							/* # fnodes in fnode file */
	614L * 512,						/* first byte in fnode file (0 orig) */
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
** Debugging output routines
**
\*********************************************************************/

#ifdef DDEBUG
static void printVolInfo(vi)
	VolInfo *vi;
{
	errorPrintf("\"%.10s\" %02x dg=%d vg=%d vs=%ld fns=%d @%ld sz=%d",
				vi -> vol_name,
				vi -> flags,
				vi -> dev_gran,
				vi -> vol_gran,
				vi -> vol_size,
				vi -> max_fnode,
				vi -> fnode_start,
				vi -> fnode_size
			   );
}
static void printFnode(fn, buf)
	int fn;
	Fnode *buf;
{	
	errorPrintf("Fnode[%d] f=%04x %c t=%x g=%d size=%ld/%ld, p: %02x%02x %02x%02x%02x %02x%02x %02x%02x%02x",
				fn, buf -> flags, buf -> flags & FN_LONG? 'L' : 'S',
				buf -> type, buf -> gran,  
				buf -> total_size, buf -> this_size,
				buf -> pointr[0], buf -> pointr[1],
				buf -> pointr[2], buf -> pointr[3], buf -> pointr[4],
				buf -> pointr[5], buf -> pointr[6],
				buf -> pointr[7], buf -> pointr[8], buf -> pointr[9]
			   );
}
#endif


/*********************************************************************\
**
** long dCvtByte(logbyte) 	convert byte to block number
**
**		Convert logical byte # to logical block# 
**
\*********************************************************************/

DWORD dCvtByte(logbyte)
	DWORD logbyte;
{
	return(logbyte / curVolInfo -> vol_gran);	
}

/*********************************************************************\
**
** dCvtBlock(logblk, &trk, &side, &sector) 	convert block number
**
**		Convert logical block# to physical track, side, and sector;
**		(Only works for minifloppies)
**
\*********************************************************************/

void dCvtBlock(logblk, track, side, sector)
	DWORD logblk;
	int *track, *side, *sector;
{
	int devblk, ts, spt, vg;

	if (logblk < trkZeroSize / curVolInfo -> vol_gran) {
		/* dev_gran == 128 for track 0 in ALL cases*/
		vg = curVolInfo -> vol_gran / T0_GRAN;
		spt     = 16;
		*sector = logblk * vg + 1;
		*track  = 0;
		*side   = 0;
	} else {
		switch (curVolInfo -> dev_gran) {
		 case 128:	spt = 16; break;
		 case 256:	spt =  9; break;
		 case 512:	spt =  4; break;
		 case 1024:	spt =  2; break;
		}
		logblk -= trkZeroSize / curVolInfo -> vol_gran;
		if (curVolInfo -> flags & VF_DENSITY) {spt &= ~1; spt *= 2;}
		vg = curVolInfo -> vol_gran / curVolInfo -> dev_gran;
		devblk  = logblk * vg;
		*sector = (devblk % spt) + 1;	  	/* +1 -- sectors are 1-origin */
		*track  = (devblk)/spt + 1;			/* +1 -- we subtracted trk 0  */
		if (curVolInfo -> flags & VF_SIDES) {
			*side = *track % 2;
			*track = (*track - *side)/2;
		} else {
			*side = 0;
		}
	}
}

/*********************************************************************\
**
** dOpenDisk(drv)
**
**		Read volume info if possible.
**		Read fnode file
**
\*********************************************************************/

int dOpenDisk(drv)
	int drv;					/* drive number */
{
	int seclen, spt, retval;
	int count;
	static char fnbuf[1024];			/* temporary */
	static Fnode *fn = (Fnode *)fnbuf;

	/* read track 0 if possible */

	curVolInfo = volInfos[drvVolInfo[drv]];
	if (dDriveType[drv] & DRV_COMPAT) {
		dInitDrive(drv, T0_GRAN, T0_BYTES/T0_GRAN, 0);
		dErrorsExpected = TRUE;
		retval = dRdSector(trkZeroBuf, 0, 0, 1, (int)trkZeroBlks);
		if (!retval) {
			drvVolInfo[drv] = 0;
		} else if (drvVolInfo[drv] == 0) {
			drvVolInfo[drv] = (dDriveType[drv] & DRV_96TPI) ? MDINFO : LDINFO;
		}
		curVolInfo = volInfos[drvVolInfo[drv]];
		if ((dDriveType[drv] & DRV_96TPI) 
		 && curVolInfo -> vol_size > 400000L)	/* === 327680, actually === */
			dMediaType[drv] |= MT_96TPI;
		else
			dMediaType[drv] &= ~MT_96TPI;
	}
	dErrorsExpected = FALSE;

#ifdef RDEBUG
	printVolInfo(curVolInfo);
#endif

	/* Now, based on what we read in track 0, set up the rest */

	seclen = curVolInfo -> dev_gran;
	spt    = curVolInfo -> flags & VF_DENSITY? ( 	/* double density */
				seclen == 256? 16 :
				seclen == 512?  8 :	4
	         ) : (									/* single density */
				seclen == 128? 16 :
				seclen == 256?  9 :
				seclen == 512?  4 : 2
			 );

	dInitDrive(drv, seclen, spt, (curVolInfo -> flags & VF_DENSITY)? 1 : 0);

	/* 
	** read the fnode file
	**		We actually have to read the first block twice, 
	**		the first time to see how big it is.
	*/

	fnodeOrig = 0;
	fnodeCount= curVolInfo -> max_fnode;
	fnodeBlk  = dCvtByte(curVolInfo -> fnode_start);
	retval = dRdBlock(fnbuf, fnodeBlk, 1);
	if (retval) {
		return (FALSE);
	}
	count = fnodeBc = fn -> total_blks;
	if (fnodeBuf) {free(fnodeBuf); fnodeBuf = 0;} 
	fnodeBuf = malloc(count * curVolInfo -> vol_gran);
	dRdBlock(fnodeBuf, fnodeBlk, count);

	/*
	 * Free previous block and fnode allocation maps, if any
	 */
	if (blkMap) {free(blkMap); blkMap = 0;} 
	if (fnodeMap) {free(fnodeMap); fnodeMap = 0;} 

	return (TRUE);
}

/*********************************************************************\
**
** int dRdBlock(buf, logblk, count)		read logical block(s)
** int dWrBlock(buf, logblk, count)		write logical block(s)
**
\*********************************************************************/

int dRdBlock(buf, logblk, count)
	char *buf;
	DWORD logblk;
	int count;
{
	int track, side, sector;
	int vg;

	dCvtBlock(logblk, &track, &side, &sector);
	vg = curVolInfo -> vol_gran / curVolInfo -> dev_gran;
	return (dRdSector (buf, track, side, sector, count * vg));
}

int dWrBlock(buf, logblk, count)
	char *buf;
	DWORD logblk;
	int count;
{
	int track, side, sector;
	int vg;

	dCvtBlock(logblk, &track, &side, &sector);
#ifdef WDEBUG
errorPrintf("Writing blk %ld: t=%d, s=%d, s=%d, count=%d", 
			logblk, track, side, sector, count);
#endif
	vg = curVolInfo -> vol_gran / curVolInfo -> dev_gran;
	return (dWrSector (buf, track, side, sector, count * vg));
}


/*********************************************************************\
**
** F I L E S
**
**		This stuff implements the necessary data structures
**		for reading and writing files.
**
\*********************************************************************/

/*********************************************************************\
**
** Fnode *fnodeP(fnodenum)		return a pointer to a fnode
** xstatN(fnodenum, buf)  		read file status (== fnode)
**
**		returns 0 normally, -1 if unsuccessful
**
\*********************************************************************/

static Fnode *fnodeP(fn)
	int fn;
{
	if (fn < fnodeOrig || fn >= fnodeOrig + fnodeCount)	{
		/* Read appropriate fnode block into the buffer */
		/* === for now, assume it's all there === */
		errorPrintf("fnodeP/xstatN: fnode %d not in memory (BUG)", fn);
	}
	return ((Fnode *)&fnodeBuf[(fn - fnodeOrig) * curVolInfo -> fnode_size]);
}

int xstatN(fn, buf)
	int	fn;
	Fnode *buf;
{
	*buf = *fnodeP(fn);
#ifdef RDEBUG
	printFnode(fn, buf);
#endif 
	return (0);
}


/*********************************************************************\
**
** static ulong xNxtBlk(XFile) -> next block in file
**
**		returns the next logical volume block number of the file.
**		returns 0 at EOF
**
\*********************************************************************/

static ulong xNxtBlk(f)
	XFile *f;
{
	ulong n;

	f -> sec_off += f -> blk_len;
	f -> sec_ptr = 0;
	if (f -> fnode.flags & FN_LONG) {
		/* 
		 * advance to next block in indirect extent 
		 */
		n = ++f -> ind_sec;
		if (n < IPTR_NUM(f -> ind_ptr)) {
			return (f -> sec_loc = n + IPTR_BLK(f -> ind_ptr));
		}
		/* 
		 * Advance to next indirect pointer 
		 */
		f -> ind_sec = 0;
		if ((f -> ind_ptr += IPTR_SIZE) < f -> ind_buf + f -> blk_len) {
			return (f -> sec_loc = IPTR_BLK(f -> ind_ptr));
		}
		/* If we need another direct block, fall through.
		 *   Note that the count in the direct pointer is the TOTAL 
		 *   for the entire indirect block.
		 */
	} 
	/* 
	 * set n = next direct block 
	 */
	n = ++f -> dir_sec;
	if (n < DPTR_NUM(f -> dir_ptr) && !(f -> fnode.flags & FN_LONG)) {
		/*
		 * next block in current extent
		 */
		n += DPTR_BLK(f -> dir_ptr);
	} else {
		/* 
		 * Advance to first block in next extent 
		 */
		f -> dir_sec = 0;
		if ((f -> dir_ptr += DPTR_SIZE) 
			< f -> fnode.pointr + sizeof(f -> fnode.pointr)
			&& DPTR_NUM(f -> dir_ptr) > 0)	{
				n = DPTR_BLK(f -> dir_ptr);
		} else {
			/* 
			 * END OF FILE 
			 */
			return (0);
		}
	}
	if (f -> fnode.flags & FN_LONG) {
		/* 
		 * indirect: find the first indirect pointer of block n 
		 */
		 f -> ind_loc = n;
		 if (dRdBlock(f -> ind_buf, n, 1)) return (0);
		 f -> ind_ptr = f -> ind_buf;
		 f -> ind_sec = 0;
		 return (f -> sec_loc = IPTR_NUM(f -> ind_ptr));
	} else {
		/*
		 * direct: return n
		 */
		return (f -> sec_loc = n);
	}
}

/*********************************************************************\
**
** static void xFlush(xf)
**
**		Flush current buffer(s) if dirty
**
\*********************************************************************/

static void xFlush(f)
	XFile *f;
{
#ifdef WDEBUG
errorPrintf("Flushing: sec %s, ind %s, fnode %s",
			f -> sec_dirty? "dirty": "clean",
			f -> ind_dirty? "dirty": "clean",
			f -> fnode_dirty? "dirty": "clean"
			);
#endif
	 if (f -> sec_dirty) {
	 	dWrBlock(f -> sec_buf, f -> sec_loc, 1);
		f -> sec_dirty = FALSE;
	 }
	 if (f -> ind_dirty) {
	 	dWrBlock(f -> ind_buf, f -> ind_loc, 1);
		f -> ind_dirty = FALSE;
	 }
	 if (f -> fnode_dirty) {
		/* 
		 * write back into fnode buffer 
		 */
		*fnodeP(f -> fnode_num) = f -> fnode;
		/* 
		 * write changed block
		 *	=== to keep it simple, write the whole file. ===
		 *  === the fnode may be split across sectors,   ===
		 *  === so this is not totally unreasonable		 ===
		 */
	 	dWrBlock(fnodeBuf, fnodeBlk, fnodeBc);
		f -> fnode_dirty = FALSE;
	 }
}

/*********************************************************************\
**
** static long xAlloc(n, fnode, map) -> next free bit
** static long xAllocB(n) -> next free block
** static long xAllocF(n) -> next free fnode
**
**		Allocate the next free (block/fnode) after n.
**		Return -1 on failure.
**		Assume that the map files are single contiguous chunks.
**
\*********************************************************************/

static long xAlloc(n, fnn, mapp)
	long n;
	int fnn;
	uchar **mapp;
{
	int bit;
	ulong off, max;
	Fnode *fn;
	int siz;
	ulong blk;

	/*
	 * First make sure the allocation table is in memory.
	 * Whether it is or not, find out where it is on the disk.
	 */
	fn = fnodeP(fnn);
	siz = fn -> total_blks;
	blk = DPTR_BLK(fn -> pointr);
	max = fn -> total_size * 8;
	if (!*mapp) {
#ifdef WDEBUG
	errorPrintf("Reading map @%ld size=%d", blk, siz);
#endif
		if (siz != DPTR_NUM(fn -> pointr)) {
			errorPrintf("allocation map file not contiguous!");
		}
		*mapp = malloc(siz * curVolInfo -> vol_gran);
		dRdBlock(*mapp, blk, siz);
	}

	/*
	 * Then, start at blk and start searching for a free one
	 * With typical perversity, allocated items are marked with 0 bits.
	 * This probably permitted the use of JFFO on a PDP-6 somewhere.
	 */
	for (off = n >> 3, bit = n & 7; 
		 n < max && !((*mapp)[off] & (1 << bit)); 
		 ++n, off = n >> 3, bit = n & 7) ;
	if (n == max) 
		for (off = 0, bit = 0, n = 0; 
			 n < max && !((*mapp)[off] & (1 << bit)); 
			 ++n, off = n >> 3, bit = n & 7) ;
	if (n == max) return (-1L);
	(*mapp)[off] &= ~(1 << bit);

	/*
	 * Finally, flush the portion of the buffer we just touched.
	 */
#ifdef WDEBUG
errorPrintf("Alloc %ld; max = %ld, map @%ld, sz=%d; data=%02x in blk %ld",
			n, max, blk, siz, (*mapp)[off] & 255, blk + off/curVolInfo -> vol_gran);
#endif
	dWrBlock((*mapp) + off - off % curVolInfo -> vol_gran, 
			 blk + off/curVolInfo -> vol_gran, 1);

	return (n);
}

static long xAllocB(n)
	long n;
{
#ifdef WDEBUG
errorPrintf("xAllocB(%ld)",	n);
#endif
	return (xAlloc(n, 1, &blkMap));
}

static long xAllocF(n)
	long n;
{
#ifdef WDEBUG
errorPrintf("xAllocF(%ld)",	n);
#endif
	return (xAlloc(n, 2, &fnodeMap));
}

/*********************************************************************\
**
** int xcreatN(isDir)		    create an fnode for a file.
**
**		Allocates and initializes the fnode, and allocates the first
**		sector of the file.  It is up to the caller to create a
**		directory entry and, if necessary, make the new fnode a
**		directory itself.
**
**		The caller must also supply the parent directory and the
**		assorted time fields.
**
\*********************************************************************/

int xcreatN(isDir)
	Bool isDir;
{
	int fn;
	Fnode *xf;
	int sec, i;
	char *p;

#ifdef WDEBUG
	errorPrintf("About to create an Fnode");
#endif
	fn = xAllocF(0L);
	if (fn == -1) return (fn);
	xf = fnodeP(fn);
	xf -> flags = FN_RES | FN_MOD;
	xf -> type  = isDir? FT_DIR : FT_DATA;
	xf -> gran  = 1;
	xf -> owner = -1;
	xf -> cr_time    = time((time_t*)0) - 3600L * 24L * (365L * 8 + 2);
	xf -> mod_time   = xf -> access_time = xf -> cr_time;
	xf -> total_size = 0;
	xf -> total_blks = 1;
	xf -> this_size  = xf -> total_blks * curVolInfo -> vol_gran;
	xf -> id_count   = 0;

	for (i = 0; i < sizeof(xf -> pointr); ++i) xf -> pointr[i] = 0;

	sec = xAllocB(16L);
	if (sec == -1) {
		return (-1);
	}
	p = xf -> pointr;
	DPTR_BLK_(p, sec);
	DPTR_NUM_(p, 1);

#ifdef WDEBUG
	printFnode(fn, xf);
#endif

	return (fn);
}

/*********************************************************************\
**
** xrewind(xfile)
**
\*********************************************************************/

int xrewind(xf)
	XFile *xf;
{
	ulong  n;

	xf -> dir_ptr  = xf -> fnode.pointr;
	xf -> dir_sec  = 0;
	xf -> sec_ptr  = 0;
	xf -> cur_loc  = xf -> sec_loc = xf -> sec_off = 0L;
	if (xf -> fnode.flags & FN_LONG) {
		n = xf -> ind_loc = DPTR_BLK(xf -> dir_ptr);
		if (n != 0) {
#ifdef RDEBUG
errorPrintf("xrewind: reading indirect block: %ld(%lx)", n, n);
#endif
			if (dRdBlock(xf -> ind_buf, n, 1)) return (-1);
			xf -> ind_ptr = xf -> ind_buf;
			xf -> ind_sec = 0;
			n = xf -> sec_loc = IPTR_BLK(xf -> ind_ptr);
		} else {
			n = xf -> sec_loc = 0;
		}
	} else {
		n = xf -> sec_loc = DPTR_BLK(xf -> dir_ptr);
	}
#ifdef RDEBUG
errorPrintf("xrewind: reading first block: %ld(%lx)", n, n);
#endif
	if (dRdBlock(xf -> sec_buf, n, 1)) return (-1);
	return (0);
}


/*********************************************************************\
**
** XFile *xopenN(fnode)		open a file given an fnode number
** 		  xclose(xfile)		close a file
**
**		  If xopenN is given an fnode pointer of 0, a new file is
**		  created.	NOTE: This does NOT create a directory entry!
**
** === There really ought to be xopen(fname) as well.
** === We will want it for creating fnode and dir. entry when writing.
**
\*********************************************************************/

void xclose(xf)
	XFile *xf;
{
	if (xf -> fnode_dirty || xf -> sec_dirty || xf -> ind_dirty)
		xFlush(xf);
	if (xf -> ind_buf) free(xf -> ind_buf);
	if (xf -> sec_buf) free(xf -> sec_buf);
	free(xf);	
}

XFile *xopenN(fn, flags)
	int fn;
	int flags;
{
	XFile *xf;
	int	   ret;
	ulong  n;
	Bool   new = (flags & O_CREAT) != 0;

	if (fn == -1) {new = TRUE; fn = xcreatN(FALSE);}
	if (fn == -1) return ((XFile *)0L);

	xf = (XFile *) calloc(1, sizeof(*xf));
	if (!xf) return (xf);
	if (xstatN(fn, &xf -> fnode)) {
  error:
		xclose(xf);
		return ((XFile *)0L);
	}
	xf -> fnode_num= fn;
	xf -> vol_info = curVolInfo;
	xf -> blk_len  = curVolInfo -> vol_gran;
	xf -> dir_ptr  = xf -> fnode.pointr;
	xf -> dir_sec  = 0;
	xf -> sec_ptr  = 0;
	xf -> cur_loc  = xf -> sec_loc = xf -> sec_off = 0L;

	xf -> sec_buf  = calloc(1, curVolInfo -> vol_gran);

	if (xf -> fnode.flags & FN_LONG) {
		xf -> ind_buf = calloc(1, curVolInfo -> vol_gran);
		n = xf -> ind_loc = DPTR_BLK(xf -> dir_ptr);
		if (n != 0) {
#ifdef RDEBUG
errorPrintf("reading indirect block: %ld(%lx)", n, n);
#endif
			if (dRdBlock(xf -> ind_buf, n, 1)) goto error;
			xf -> ind_ptr = xf -> ind_buf;
			xf -> ind_sec = 0;
			n = xf -> sec_loc = IPTR_BLK(xf -> ind_ptr);
		} else {
			n = xf -> sec_loc = 0;
		}
	} else {
		n = xf -> sec_loc = DPTR_BLK(xf -> dir_ptr);
	}
	if (!new && n) {
		/*
		 * Read first sector
		 */
#ifdef RDEBUG
errorPrintf("reading first block: %ld(%lx)", n, n);
#endif
		if (dRdBlock(xf -> sec_buf, n, 1)) goto error;
	} else if (new) {
		/*
		 * New file: mark fnode and first sector dirty
		 */
		xf -> fnode_dirty = TRUE;
		xf -> sec_dirty   = TRUE;
	}
	return (xf);
}

/*********************************************************************\
**
** int	  xread(xfile, buf, count)
** int	  xwrite(xfile, buf, count)
**
\*********************************************************************/

int xread(xf, buf, count)
	XFile *xf;
	char  *buf;
	int	   count;
{
	register int i;
	register int c;
	register DWORD n;

	/* NOTE: this can be done more efficiently using memcpy;
	 *  ===	 I'm opting for speed of implementation
	 */
	if (count > xf -> fnode.total_size - xf -> cur_loc)
		count = xf -> fnode.total_size - xf -> cur_loc;
	for (i = 0; i < count; ++i) {
		if (xf -> sec_ptr >= xf -> blk_len) {
			if ((n = xNxtBlk(xf)) == 0) break;
#ifdef RDEBUG
errorPrintf("reading next block: @%ld(%lx) offset %ld(%lx)", 
			n, n, xf -> sec_off, xf -> sec_off);
#endif
			if (dRdBlock(xf -> sec_buf, n, 1)) break;
		}
		*buf++ = xf -> sec_buf[xf -> sec_ptr++];
		++xf -> cur_loc;
	}
	return (i);
}

int xwrite(xf, buf, count)
	XFile *xf;
	char  *buf;
	int	   count;
{
	register int i;
	register int c;
	register DWORD n;
	unsigned int m, mm;

	for (i = 0; i < count; ++i) {
		if (xf -> sec_ptr >= xf -> blk_len) {
			/*
			 * We're at the end of a block, so flush the old one
			 * (if necessary) and get or allocate the next one.
			 * For efficiency we don't actually call xFlush here.
			 */
			if (xf -> sec_dirty) {
			 	dWrBlock(xf -> sec_buf, xf -> sec_loc, 1);
				xf -> sec_dirty = FALSE;
			}
			if ((n = xNxtBlk(xf)) != 0) {
				/*
				 * If the next block exists, read it
				 */
#ifdef WDEBUG
errorPrintf("getting next block to write: @%ld(%lx) offset %ld(%lx)", 
			n, n, xf -> sec_off, xf -> sec_off);
#endif
				if (dRdBlock(xf -> sec_buf, n, 1)) break;
			} else {
				/*
				 * Otherwise, allocate one.
				 */
				 n = xAllocB(xf -> sec_loc);
#ifdef WDEBUG
errorPrintf("allocating block to write: @%ld(%lx) offset %ld(%lx)", 
			n, n, xf -> sec_off, xf -> sec_off);
#endif
				 if (n == -1) {
				 	/* 
					 * disk full 
					 */
				 	break;
				 } else if (n == xf -> sec_loc + 1 
				 	     && ! (xf -> fnode.flags & FN_LONG) 
						 && (m = DPTR_NUM(xf -> ind_ptr)) != 0xFFFF ) {
					/*
					 * If it's contiguous to the last one,
					 * just update the count in the current pointer.
					 */
					++m;
				 	DPTR_NUM_(xf -> dir_ptr, m);
					xf -> sec_loc = n;
					xf -> fnode_dirty = TRUE;
				 } else if (n == xf -> sec_loc + 1 
				 	     && (xf -> fnode.flags & FN_LONG) 
						 && (m = IPTR_NUM(xf -> ind_ptr)) < 255 ) {
					/*
					 * Same thing, but in an indirect pointer.
					 * Note the limit check -- they're smaller.
					 */
					++m;
				 	IPTR_NUM_(xf -> ind_ptr, m);
					xf -> sec_loc = n;
					xf -> fnode_dirty = TRUE;
				 } else if (! (xf -> fnode.flags & FN_LONG)) {
					/*
					 * Otherwise, create a new pointer as well.
					 * If we're out of room, we're out of luck.
					 */
				 	if (xf -> dir_ptr + DPTR_SIZE 
					    >= xf -> fnode.pointr + sizeof(xf -> fnode.pointr))
						break;
					xf -> dir_ptr += DPTR_SIZE;
					DPTR_BLK_(xf -> dir_ptr, n);
				 	DPTR_NUM_(xf -> dir_ptr, 1);
					xf -> sec_loc = n;
					xf -> fnode_dirty = TRUE;
				 } else if (xf -> ind_ptr + IPTR_SIZE
				            >= xf -> ind_buf + xf -> vol_info -> vol_gran) {
					/*
					 * For LONG files, if we're out of room we try to 
					 * start a new indirect block.
					 */
				 	if (xf -> dir_ptr + DPTR_SIZE 
					    >= xf -> fnode.pointr + sizeof(xf -> fnode.pointr))
						break;
					if (xf -> ind_dirty) 
					 	dWrBlock(xf -> ind_buf, xf -> ind_loc, 1);
					xf -> ind_loc = n;
					n = xAllocB(xf -> ind_loc);		/* allocate new one */
					if (n == -1) break;
					xf -> dir_ptr += DPTR_SIZE;
					DPTR_BLK_(xf -> dir_ptr, xf -> ind_loc);
				 	DPTR_NUM_(xf -> dir_ptr, 1);
					xf -> sec_loc = n;
					xf -> ind_ptr = xf -> ind_buf;
					IPTR_BLK_(xf -> ind_ptr, n);
				 	IPTR_NUM_(xf -> ind_ptr, 1);
					xf -> sec_loc = n;
					xf -> fnode_dirty = TRUE;
					xf -> ind_dirty = TRUE;
				 } else {
					/*
					 * otherwise just add a pointer to the current one.
					 */
					xf -> ind_ptr += IPTR_SIZE;
					IPTR_BLK_(xf -> ind_ptr, n);
				 	IPTR_NUM_(xf -> ind_ptr, 1);
					mm = 1 + DPTR_NUM(xf -> dir_ptr);
					DPTR_NUM_(xf -> dir_ptr, mm);
					xf -> sec_loc = n;
					xf -> fnode_dirty = TRUE;
					xf -> ind_dirty = TRUE;
				 }
			}
		}
		xf -> sec_dirty = TRUE;
		xf -> sec_buf[xf -> sec_ptr++] = *buf++;
		if (xf -> fnode.total_size < ++xf -> cur_loc) {
			++ xf -> fnode.total_size;
			xf -> fnode_dirty = TRUE;
		}
	}
	return (i);
}

/*********************************************************************\
**
** long xseek(xf, offset)
** long xtell(xf)
**
\*********************************************************************/

long xseek(xf, off)
	XFile  *xf;
	long	off;
{
#if 1
	register DWORD n;

	/*
	 * This is the brute-force way to do it -- simulate a read.
	 */
	if (off < 0 || off > xf -> fnode.total_size) return (-1L);
	if (off < xf -> sec_off) {
		xrewind(xf);
	}
	while (xf -> sec_off + xf -> blk_len <= off) {
		if ((n = xNxtBlk(xf)) == 0) return (-1L);
	}
	if (dRdBlock(xf -> sec_buf, n, 1)) return (-1L);
	xf -> sec_ptr = (int) (off - xf -> sec_off);
	xf -> cur_loc = off;
	if (xf -> sec_ptr >= xf -> blk_len) {
		errorPrintf("Bug alert: xseek(%ld) offset = %ld", 
					off, off - xf -> sec_off);
		return (-1L);
	}
	return (off);
#else
	register int sec;
	register DWORD n, loc;
	register uchar *ip, *dp;

	/* === this croaks if off is negative or > file size === */
	/* === needs special kludgery if off == file size    === */

	/* First seek to the right direct block */

	loc = 0;
	dp = xf -> fnode.pointr;
	while (loc + xf -> blk_len * DPTR_NUM(dp) < off) {
		loc += xf -> blk_len * DPTR_NUM(dp);
		dp += DPTR_SIZE;
		if (dp >= xf -> fnode.pointr + sizeof(xf -> fnode.pointr))
			return (-1L);
	}
	if (loc + xf -> blk_len * DPTR_NUM(dp) == off
	  && off < xf -> fnode.total_size) {
		loc += xf -> blk_len * DPTR_NUM(dp);
		dp += DPTR_SIZE;
	}
	/* 
	 * If LONG, read the indirect block and find the right pointer. 
	 * In either case, find the right block in the extent.
	 * Note that we only do the reads if we've changed block.
	 */
	if (xf -> fnode.flags & FN_LONG) {
		n = DPTR_BLK(dp);
#ifdef RDEBUG
errorPrintf("reading indirect block: %ld(%lx)", n, n);
#endif
		if (n != xf -> ind_loc) {
			if (dRdBlock(xf -> ind_buf, n, 1)) return (-1);
			xf -> ind_loc = n;
		}
		ip = xf -> ind_buf;
		while (loc + xf -> blk_len * IPTR_NUM(ip) < off) {
			loc += xf -> blk_len * IPTR_NUM(ip);
			ip += IPTR_SIZE;
			if (ip >= xf -> ind_buf + xf -> blk_len) return (-1L);
		}
		if (loc + xf -> blk_len * IPTR_NUM(ip) == off
		  && off < xf -> fnode.total_size) {
			loc += xf -> blk_len * IPTR_NUM(ip);
			ip += IPTR_SIZE;
		}
		xf -> ind_ptr = ip;
		xf -> ind_sec = (off - loc) / xf -> blk_len;
		n = IPTR_BLK(xf -> ind_ptr) + xf -> ind_sec;
		xf -> dir_sec = 0;
		xf -> ind_loc = DPTR_BLK(dp);
	} else {
		n = xf -> dir_sec = (off - loc) / xf -> blk_len;
		n += DPTR_BLK(dp);
	}
	xf -> dir_ptr = dp;

	/* finally, read the block and point at the right byte in it */

#ifdef RDEBUG
errorPrintf("seeked to: %ld(%lx) sector %d bl=%d", 
			n, n, xf -> dir_sec, xf -> blk_len);
#endif
	xf -> sec_off = loc;
	if (n != xf -> sec_loc) {
		if (dRdBlock(xf -> sec_buf, n, 1)) return (-1);
		xf -> sec_loc = n;
	}
	xf -> sec_ptr = (off - loc) % xf -> blk_len;
	xf -> cur_loc = off;
	return (off);
#endif
}

long xtell(f)
	XFile *f;
{
	return ((long) f -> cur_loc);
}


/*********************************************************************\
**
** D I R E C T O R Y   T R E E S
**
**		Stuff in this section reads a complete directory tree into
**		the in-core tree structure (see DATA.C)
**
\*********************************************************************/


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
	if (!f) return;

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
		xstatN(ent.fnode, &fn_buf);
		/* === need error check here === */
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
	++inUse[dDrive];
	sprintf(driveName, "%c:(%d)", drive + 'A', drive);

	dErrors = 0;
	if (! dOpenDisk(dDrive)) {
		if (NOTNULL(dDrives[dDrive])) gKill(dDrives[dDrive]);
		dDrives[dDrive] = (Dir) NIL;
		return ((Dir) NIL);
	}
	dErrors = 0;

	/*
	** Construct Root Node
	*/
	xstatN(6, &fn_buf);
	n = (XDir) cfNew(cl)(cl, NIL, driveName, TRUE, fn_buf.total_size);
	if (NOTNULL(dDrives[dDrive])) gKill(dDrives[dDrive]);
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
	
	/*
	** Put the OS tables back
	*/
errorExit:
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
	(Class)&crDir,			/* class */				/* Object Part */
   {												/* Class Part */
	sizeof (XDirRec),		/* instance size */
	"XDir",					/* class name */
	&crObject,				/* super */
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


