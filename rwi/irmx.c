/***/static char *moduleID="disk 1.0 (c)1988 S. Savitzky";/***/

/*********************************************************************\
**
**	D I S K   H A N D L I N G
**
**		Here is where we read iRMX-format disks, files, and directories.
**
**  891209 SS	split Dir stuff out.
**	890622 SS	create from Idris version
**	880111 SS	create PC version based on stuff from A. Savitzky
**
** NOTES:
**		We assume that the fnode file, fnode map, and free block map
**		are contiguous, and small enough to fit in memory all at once.
**
**		We don't use the bad-block map.
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

/*********************************************************************\
**
** The volume information record and related stuff
**
\*********************************************************************/

char		  blockBuf[MAXBLOCK];
char		  trkZeroBuf[T0_BYTES];
DWORD		  trkZeroSize = T0_BYTES;
DWORD		  trkZeroBlks = 16;

#define CCINFO 0
#define LDINFO 1
#define MDINFO 3


VolInfo		  volInfoBuf;		/* a volume label */
VolInfo		 *curVolInfo;		/* -> current volume label */
char		 *fnodeBuf;			/* The fnode file or portion thereof */
int			  fnodeOrig;		/* # of first fnode in buffer */
int			  fnodeCount;		/* # of	fnodes in buffer */
DWORD		  fnodeBlk;			/* block number of start of Fnode file */
int			  fnodeBc;			/* block count of Fnode file */

static BYTE *blkMap, *fnodeMap;	/* fnode and block allocation maps */

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
		*track  = (devblk / spt) + 1;		/* +1 -- we subtracted trk 0  */
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
** long dFindFnodes(drive)
**
**		Attempt to locate the fnode file on a drive.  Returns
**		block number if successful, else -1.  Munches *curVolInfo
**		to return the rest of its information.
**
**		Assumes that most or all of dOpenDisk has been done.
**
\*********************************************************************/

long dFindFnodes(drv)
	int drv;
{
	int retval;
	long blk;							/* current block */
	static char fnbuf[1024];			/* temporary */
	static Fnode *fn = (Fnode *)fnbuf;
	Fnode *fp;
	int i;

	move(24, 0);
	addstr("Searching for Fnode file ");
	clrtoeol();
	for (blk = curVolInfo -> vol_size / curVolInfo -> vol_gran / 4; 
		 blk < curVolInfo -> vol_size / curVolInfo -> vol_gran; 
		 ++blk) {
		if (blk % 5 == 0) {
			move (24, 25);
			wprintf(stdscr, "%5ld", blk);
		}
		retval = dRdBlock(fnbuf, blk, 1);
		if (retval) return (-1);
		if (fn -> type == FT_FNODE && (fn -> flags & FN_ALLOC) && 
			blk == DPTR_BLK(fn -> pointr)) break;
	}
	curVolInfo -> fnode_start = blk * curVolInfo -> vol_gran;
	for (i = sizeof(Fnode) - 10, fp = (Fnode *)(fnbuf + i);
		 fp -> type != FT_VOLMAP  && ! (fp -> flags & FN_ALLOC);
		 ++i, fp = (Fnode *)(fnbuf + i) ) 
	    if (i > 512) return (-1);
	curVolInfo -> fnode_size = i;
	curVolInfo -> max_fnode = fn -> total_size / curVolInfo -> fnode_size;
	return (blk);
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

	/*
	 * Free previous fnode file and the	previous disk's
	 * block and fnode allocation maps, if any.
	 */
	if (blkMap) {free(blkMap); blkMap = 0;} 
	if (fnodeMap) {free(fnodeMap); fnodeMap = 0;} 
	if (fnodeBuf) {free(fnodeBuf); fnodeBuf = 0;} 

	/* read track 0 if possible */

	curVolInfo = volInfos[drvVolInfo[drv]];
	if (dDriveType[drv] & DRV_COMPAT) {
		dInitDrive(drv, T0_GRAN, T0_BYTES/T0_GRAN, 0);
		/* dErrorsExpected = TRUE; */
		retval = dRdSector(trkZeroBuf, 0, 0, 1, (int)trkZeroBlks);
		if (retval) {
			dCloseDisk();
			return (FALSE);
		}
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
	if (DPTR_BLK(fn -> pointr) != fnodeBlk) {
		if (dDriveType[drv] & DRV_COMPAT) {
			errorPrintf("Invalid fnode file or fnode file location");
			return (FALSE);
		} else if (dFindFnodes(drv) == -1L) {
			errorPrintf("Cannot locate fnode file -- DOS disk?");
			return (FALSE);
		} else {
			fnodeCount= curVolInfo -> max_fnode;
			fnodeBlk  = dCvtByte(curVolInfo -> fnode_start);
			if (dRdBlock(fnbuf, fnodeBlk, 1)) return (FALSE);
		}
	}
	count = fnodeBc = fn -> total_blks;
	fnodeBuf = malloc(count * curVolInfo -> vol_gran);
	if (dRdBlock(fnodeBuf, fnodeBlk, count)) return (FALSE);

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
	if (fn < fnodeOrig || fn >= fnodeOrig + fnodeCount) {
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
	if (!fnodeBuf) return (-1);
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
	if (f -> sec_off / f -> blk_len >= f -> fnode.total_blks) 
		return (0);
	if (f -> fnode.flags & FN_LONG) {
		/* 
		 * advance to next block in indirect extent 
		 */
#ifdef RDEBUG
	errorPrintf("advancing to next indirect block, # %ld, %d",
				1 + f -> ind_sec, IPTR_NUM(f -> ind_ptr));
#endif
		n = ++f -> ind_sec;
		if (n < IPTR_NUM(f -> ind_ptr)) {
			f -> sec_loc = n + IPTR_BLK(f -> ind_ptr);
			return (f -> sec_loc);
		}
		/* 
		 * Advance to next indirect pointer 
		 */
#ifdef RDEBUG
	errorPrintf("advancing to next indirect extent");
#endif
		f -> ind_sec = 0;
		if ((f -> ind_ptr += IPTR_SIZE) < f -> ind_buf + f -> blk_len) {
			f -> sec_loc = IPTR_BLK(f -> ind_ptr);
			return (f -> sec_loc);
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
		 f -> sec_loc = IPTR_NUM(f -> ind_ptr);
	} else {
		/*
		 * direct: return n
		 */
		f -> sec_loc = n;
	}
	return (f -> sec_loc);
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
** Bool xReForm(xf, blk)		change xf from short to long format
**		if blk != 0, use it as the (first) indirect block
**		return TRUE if successful.
**
\*********************************************************************/

static Bool xReForm(xf, blk)
	XFile *xf;
	DWORD blk;
{
	register int dpc, ipc;
	ulong dpb, ipb;
	DWORD off;
	BYTE *dp, *ip, *fdp;		/* fdp = fixed direct ptr */

	/* Allocate a block if it wasn't provided */

	if (!blk) {
		blk = xAllocB(16L);
		if (blk == -1) return (FALSE);
	}

	/* allocate the indirect block and point to it. */

	xf -> ind_buf = calloc(1, xf -> blk_len);
	if (! xf -> ind_buf) return(FALSE);
	xf -> ind_loc = blk;
	xf -> ind_ptr = xf -> ind_buf;
	xf -> ind_sec = 0;
	xf -> fnode.flags |= FN_LONG;

	/* 
	** Loop through the dptrs copying to the iptrs. 
	**     Ought to worry about allocating another iblock if needed,
	**	   but in fact there aren't enough sectors on a floppy disk
	**	   to make this necessary.
	*/
	for (dp = fdp = xf -> fnode.pointr, ip = xf -> ind_buf, ipc = 0;
		 dp < xf -> fnode.pointr + sizeof(xf -> fnode.pointr);
		 dp += DPTR_SIZE) {
		for (dpc = DPTR_NUM(dp), dpb = DPTR_BLK(dp); 
			 dpc > 0; 
			 dpc -= 255, dpb += 255) {
			if (dpc > 255) {
				IPTR_NUM_(ip, 255);
				ipc += 255;
			} else {
				IPTR_NUM_(ip, dpc);
				ipc += dpc;
			}
			IPTR_BLK_(ip, dpb);
			ip += IPTR_SIZE;
			if (ip > xf -> ind_buf + xf -> blk_len) {
				return (FALSE);
				/* === If we fill up the ind block we're in trouble === */
				/* === really ought to allocate another.            === */
				/* === But a floppy doesn't have that many sectors! === */
			}
		}
	}

	/* 
	** Replace the first dptr with pointer to the iblock
	** 		Its count becomes the total count
	*/
	DPTR_BLK_(fdp, blk);
	DPTR_NUM_(fdp, ipc);
	xf -> dir_ptr = xf -> fnode.pointr;

	/* clear the rest */

	for (fdp += DPTR_SIZE;
		 fdp < xf -> fnode.pointr + sizeof(xf -> fnode.pointr);
		 fdp += DPTR_SIZE) {
		DPTR_BLK_(fdp, 0);
		DPTR_NUM_(fdp, 0);
	}

	/* Fake a seek to current block */

	for (off = 0L; off < xf -> sec_off; off += xf -> blk_len) {
		if (++xf -> ind_sec >= IPTR_NUM(xf -> ind_ptr)) {
			xf -> ind_sec = 0;
			xf -> ind_ptr += IPTR_SIZE;
		}
	}

	/* Mark the fnode and ind. block as having changed */

	xf -> fnode_dirty = TRUE;
	xf -> ind_dirty   = TRUE;

	return (TRUE);
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
	int i;
	ulong sec;
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
	if (sec == -1L) {
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
		/* === xReForm(xf, 0L);		TEST === */
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
			if (dRdBlock(xf -> sec_buf, n, 1)) {
				errorPrintf("error reading block: @%ld(%lx) offset %ld(%lx)", 
							n, n, xf -> sec_off, xf -> sec_off);
				break;
			}
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
	   retry:
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
						 && (m = DPTR_NUM(xf -> dir_ptr)) != 0xFFFF ) {
					/*
					 * If it's contiguous to the last one,
					 * just update the count in the current pointer.
					 */
					++m;
				 	DPTR_NUM_(xf -> dir_ptr, m);
				} else if (n == xf -> sec_loc + 1 
				 	     && (xf -> fnode.flags & FN_LONG) 
						 && (m = IPTR_NUM(xf -> ind_ptr)) < 255 ) {
					/*
					 * Same thing, but in an indirect pointer.
					 * Note the limit check -- they're smaller.
					 */
					++m;
				 	IPTR_NUM_(xf -> ind_ptr, m);
					xf -> ind_dirty = TRUE;
				} else if (! (xf -> fnode.flags & FN_LONG)) {
					/*
					 * Otherwise, create a new pointer as well.
					 * If we're out of room, re-format.
					 */
				 	if (xf -> dir_ptr + DPTR_SIZE 
					    >= xf -> fnode.pointr + sizeof(xf -> fnode.pointr)) {
						if (! xReForm(xf, n)) break;
						goto retry;
					}
					xf -> dir_ptr += DPTR_SIZE;
					DPTR_BLK_(xf -> dir_ptr, n);
				 	DPTR_NUM_(xf -> dir_ptr, 1);
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
					xf -> ind_ptr = xf -> ind_buf;
					IPTR_BLK_(xf -> ind_ptr, n);
				 	IPTR_NUM_(xf -> ind_ptr, 1);
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
					xf -> ind_dirty = TRUE;
				}
				xf -> sec_loc = n;
				++xf -> fnode.total_blks;
				xf -> fnode_dirty = TRUE;
			}
		}
		xf -> sec_dirty = TRUE;
		xf -> sec_buf[xf -> sec_ptr++] = *buf++;
		++xf -> cur_loc;
	}
	if (xf -> fnode.total_size < xf -> cur_loc) {
		xf -> fnode.total_size = xf -> cur_loc;
		xf -> fnode_dirty = TRUE;
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
	n = xf -> sec_loc;		/* in case loop body isn't needed */
	while (xf -> sec_off + xf -> blk_len <= off) {
		if ((n = xNxtBlk(xf)) == 0) return (-1L);
	}
#ifdef DDEBUG
errorPrintf("xseek: reading final block %ld(%lx)", n, n);
#endif
	if (n != xf -> sec_loc)
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



