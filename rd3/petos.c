/***/static char *moduleID="petos.c 1.0 (c)1988 S. Savitzky";/***/

/*********************************************************************\
**
**	PETOS.C
**
**		Here is where we read PETOS-format disks, files, and directories.
**
**  900114	SS	Create from irmx.c
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
#include "petos.h"

/* #define WDEBUG 1 */
/* #define RDEBUG 1 */
/* #define DDEBUG 1 */
#if (!defined(DDEBUG) && (defined(WDEBUG) || defined(RDEBUG)))
#define DDEBUG 1
#endif


/*********************************************************************\
**
** Buffers
**
**		Keeping the directory in a buffer relies on the fact that
**		PETOS has a flat file system with one directory per disk.
**
**		We will need more of these if we want to have more than
**		one PETOS disk open at a time.
**
\*********************************************************************/

uchar	 VolInfoBuf[128];
uchar 	 SecUsageBuf[128];
RIB		 DirRibBuf;
DirEntry DirBuf[DIRLEN];


/*********************************************************************\
**
** Debugging output routines
**
\*********************************************************************/

#ifdef DDEBUG
static void printRIB(blk, buf)
	int  blk;
	RIB *buf;
{ 
	errorPrintf("RIB[%d] %6.6s %6.6s  %02x%02x%02x%02x %02x %02x%02x %02x%02x %02x c: %02x %02x%02x  %02x %02x%02x  %02x %02x%02x",
				blk, buf -> cdate, buf -> mdate, /* FILE_SIZE(buf), */
				buf->pad1[0]&255, buf->pad1[1]&255, 
				buf->pad1[2]&255, buf->pad1[3]&255, 
				buf -> nbytes&255, buf -> nsecs[0]&255, buf -> nsecs[1]&255,
				buf->pad2[0]&255, buf->pad2[1]&255, buf -> ftype,
				buf -> clusters[0], buf -> clusters[1],	buf -> clusters[2],
				buf -> clusters[3], buf -> clusters[4],	buf -> clusters[5],
				buf -> clusters[6], buf -> clusters[7],	buf -> clusters[8]
			   );
}

static void printDirEnt(buf)
	DirEntry *buf;
{
	char *p = (char*)buf + 7;
	errorPrintf("DirEntry: '%7.7s', %02x  %02x %02x %02x %02x %02x %02x %02x %02x",
				buf -> fname,
				p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], p[8]
			   );
}
#endif


/*********************************************************************\
**
** dCvtBlock(logblk, &trk, &side, &sector) 	convert block number
**
**		Convert logical block# to physical track, side, and sector;
**		(Only works for minifloppies)
**
\*********************************************************************/

void dCvtBlock(logblk, track, side, sector)
	int logblk;
	int *track, *side, *sector;
{
	int devblk, ts, spt, vg;

	/* PETOS disks are all 40-track, 16 * 128-byte sectors */
	spt     = 16;
	*sector = (logblk % spt) + 1;	  	/* +1 -- sectors are 1-origin */
	*track  = (logblk / spt);

	if (dMediaType[dDrive] & MT_SS) {
		*side = 0;
	} else {
		*side = *track % 2;
		*track = (*track - *side)/2;
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
	/* If we don't have a compaticard drive, forget it */

	if (! (dDriveType[drv] & DRV_COMPAT)) return (FALSE);

	/* read track 0 if possible -- fails if not ready or wrong format */

	dInitDrive(drv, 128, 16, 0);
	if (dRdSector(VolInfoBuf,  0, 0, T0_VolInfo,  1)) goto errorExit;
/*	if (dRdSector(SecUsageBuf, 0, 0, T0_SecTable, 1)) goto errorExit;
*/	if (dRdSector(&DirRibBuf,  0, 0, T0_DirRIB,   1)) goto errorExit;
#ifdef RDEBUG
	printRIB(T0_DirRIB - 1, DirRibBuf);
#endif
	if (dRdSector(DirBuf,      0, 0, T0_DirLoc,  11)) goto errorExit;

	dMediaType[drv] = MT_SD;
	if (VolInfoBuf[6] < 0x20) dMediaType[drv] |= MT_SS;

#ifdef RDEBUG
	printDirEnt(DirBuf);
#endif
	return (TRUE);

 errorExit:
 	dCloseDisk();
	return (FALSE);
}

/*********************************************************************\
**
** int dRdBlock(buf, logblk, count)		read logical block(s)
** int dWrBlock(buf, logblk, count)		write logical block(s)
**
\*********************************************************************/

int dRdBlock(buf, logblk, count)
	char *buf;
	int logblk;
	int count;
{
	int track, side, sector;

	dCvtBlock(logblk, &track, &side, &sector);
	return (dRdSector (buf, track, side, sector, count));
}

int dWrBlock(buf, logblk, count)
	char *buf;
	int logblk;
	int count;
{
	int track, side, sector;

	dCvtBlock(logblk, &track, &side, &sector);
#ifdef WDEBUG
errorPrintf("Writing blk %d: t=%d, s=%d, s=%d, count=%d", 
			logblk, track, side, sector, count);
#endif
	return (dWrSector (buf, track, side, sector, count));
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
** xstatN(loc, buf)  		read file status (== RIB)
**
**		returns 0 normally, -1 if unsuccessful
**
\*********************************************************************/

int xstatN(blk, buf)
	int	blk;
	RIB *buf;
{
	if (dRdBlock(buf, blk, 1)) return (-1);
#ifdef RDEBUG
	printRIB(blk, buf);
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
	int n;

	if ((f -> sec_off / f -> blk_len) + 1 > /* === >=? === */ 
		X_INT(f -> rib.nsecs) + (f -> rib.nbytes ? 0 : 1)) 
		return (0);
	f -> sec_off += f -> blk_len;
	f -> sec_ptr = 0;
	/* 
	 * set n = next direct block 
	 */
	n = ++f -> c_sec;
	if (n < CPTR_NUM(f -> c_ptr)) {
		/*
		 * next block in current extent
		 */
		n += CPTR_BLK(f -> c_ptr);
	} else {
		/* 
		 * Advance to first block in next extent 
		 */
		f -> c_sec = 0;
		if ((f -> c_ptr += CPTR_SIZE) 
			< f -> rib.clusters + sizeof(f -> rib.clusters)
			&& CPTR_NUM(f -> c_ptr) > 0)	{
				n = CPTR_BLK(f -> c_ptr);
		} else {
			/* 
			 * END OF FILE 
			 */
			return (0);
		}
	}
	f -> sec_loc = n;
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
errorPrintf("Flushing: sec %s, rib %s",
			f -> sec_dirty? "dirty": "clean",
			f -> rib_dirty? "dirty": "clean"
			);
#endif
	 if (f -> sec_dirty) {
	 	dWrBlock(f -> sec_buf, f -> sec_loc, 1);
		f -> sec_dirty = FALSE;
	 }
	 if (f -> rib_dirty) {
	 	dWrBlock(f -> rib, f -> rib_loc, 1);
		f -> rib_dirty = FALSE;
	 }
}

/*********************************************************************\
**
** static long xAlloc(n, loc, map) -> next free bit
** static long xAllocB(n) -> next free block
**
**		Allocate the next free (block) after n.
**		Return -1 on failure.
**		Assume that the map file is a single sector.
**
\*********************************************************************/

static long xAllocB(n)
	int n;
{
	int bit;
	unsigned int off;
	uchar *mapp = SecUsageBuf;
	unsigned int max = 128 * 8;

	/*
	 * The allocation table has already been read by dOpenDisk.
	 * Start at blk and start searching for a free one.
	 * With typical perversity, allocated items are marked with 0 bits.
	 * This probably permitted the use of JFFO on a PDP-6 somewhere.
	 */
	for (off = n >> 3, bit = n & 7; 
		 n < max && !(mapp[off] & (1 << bit)); 
		 ++n, off = n >> 3, bit = n & 7) ;
	if (n == max) 
		for (off = 0, bit = 0, n = 0; 
			 n < max && !(mapp[off] & (1 << bit)); 
			 ++n, off = n >> 3, bit = n & 7) ;
	if (n == max) return (-1L);
	mapp[off] &= ~(1 << bit);

	/*
	 * Finally, flush the buffer we just touched.
	 */
#ifdef WDEBUG
errorPrintf("Alloc %d; max = %d, map @%d, sz=%d; data=%02x in blk %d",
			n, max, blk, siz, mapp[off] & 255, T0_SecTable);
#endif
	dWrBlock(mapp, T0_SecTable, 1);

	return (n);
}

/*********************************************************************\
**
** int allocF()
**
**		Return index of next free directory entry.
**		Returns -1 if directory is full.
**
\*********************************************************************/


/*********************************************************************\
**
** int xcreatN()		    create a RIB for a file.
**
**		Allocates and initializes the RIB, and allocates the first
**		cluster of the file.  It is up to the caller to create a
**		directory entry.
**
**		The caller must also supply the assorted time fields.
**
\*********************************************************************/

int xcreatN()
{
	int fn;
	RIB *xf;
	int i;
	ulong sec;
	char *p;

#ifdef WDEBUG
	errorPrintf("About to create a RIB");
#endif

#if 1 /* === Omit writing for now === */
	return ( -1);
#else
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
#endif /* ===  === */

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
	xf -> c_ptr   = xf -> rib.clusters;
	xf -> c_sec   = 1;
	xf -> sec_ptr = 0;
	xf -> cur_loc = xf -> sec_off = 0L;
	xf -> sec_loc = CPTR_BLK(xf -> c_ptr) + xf -> c_sec;
#ifdef RDEBUG
errorPrintf("xrewind: reading first block: %d(%lx)", 
			xf -> sec_loc, xf -> sec_loc );
#endif
	if (dRdBlock(xf -> sec_buf, (int)xf -> sec_loc, 1)) return (-1);
	return (0);
}


/*********************************************************************\
**
** XFile *xopenN(fnode)		open a file given an fnode number
** void	  xclose(xfile)		close a file
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
	if (xf -> rib_dirty || xf -> sec_dirty)
		xFlush(xf);
	if (xf -> sec_buf) free(xf -> sec_buf);
	free(xf);	
}

XFile *xopenN(fn, flags)
	int fn;
	int flags;
{
	XFile *xf;
	int	   ret;
	int	   n;
	Bool   new = (flags & O_CREAT) != 0;

	if (fn == -1) {new = TRUE; fn = xcreatN();}
	if (fn == -1) return ((XFile *)0L);

	xf = (XFile *) calloc(1, sizeof(*xf));
	if (!xf) return (xf);
	if (xstatN(fn, &xf -> rib)) {
  error:
		xclose(xf);
		return ((XFile *)0L);
	}
	xf -> text_mode = (flags & O_TEXT)? 1 : 0;
	xf -> blk_len = SECLEN;
	xf -> rib_loc = fn;
	xf -> c_ptr   = xf -> rib.clusters;
	xf -> c_sec   = 1;
	xf -> sec_ptr = 0;
	xf -> cur_loc = xf -> sec_off = 0L;
	n = xf -> sec_loc = CPTR_BLK(xf -> c_ptr) + xf -> c_sec;

	xf -> sec_buf  = calloc(1, xf -> blk_len);

	if (!new && n) {
		/*
		 * Read first sector
		 */
#ifdef RDEBUG
errorPrintf("reading first block: %d(%x)", n, n);
#endif
		if (dRdBlock(xf -> sec_buf, n, 1)) goto error;
	} else if (new) {
		/*
		 * New file: mark RIB and first sector dirty
		 */
		xf -> rib_dirty = TRUE;
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
	register long c;  				/* File might be very long */
	register int n;					/* Can get away with int for block # */

	/* NOTE: this can be done more efficiently using memcpy;
	 *  ===	 I'm opting for speed of implementation
	 */
	c = FILE_SIZE(&xf->rib) - xf -> cur_loc;
	if (count > c)	count = c;
	if (!xf -> text_mode)
		for (i = 0; i < count; ++i) {
			if (xf -> sec_ptr >= xf -> blk_len) {
				if ((n = xNxtBlk(xf)) == 0) break;
#ifdef RDEBUG
	errorPrintf("reading next block: @%d(%x) offset %ld(%lx)", 
				n, n, xf -> sec_off, xf -> sec_off);
#endif
				if (dRdBlock(xf -> sec_buf, n, 1)) {
					errorPrintf("error reading block: @%d(%x) offset %ld(%lx)", 
								n, n, xf -> sec_off, xf -> sec_off);
					break;
				}
			}
			*buf++ = xf -> sec_buf[xf -> sec_ptr++];
			++xf -> cur_loc;
		}
	else
		for (i = 0; i < count; ++i) {
			if (xf -> sec_ptr >= xf -> blk_len) {
				if ((n = xNxtBlk(xf)) == 0) break;
	#ifdef RDEBUG
	errorPrintf("reading next block: @%d(%x) offset %ld(%lx)", 
				n, n, xf -> sec_off, xf -> sec_off);
	#endif
				if (dRdBlock(xf -> sec_buf, n, 1)) {
					errorPrintf("error reading block: @%d(%x) offset %ld(%lx)", 
								n, n, xf -> sec_off, xf -> sec_off);
					break;
				}
			}
			*buf = xf -> sec_buf[xf -> sec_ptr++];
			if (*buf == '\r') *buf = '\n';
			++buf;
			++xf -> cur_loc;
		}
	return (i);
}

int xwrite(xf, buf, count)
	XFile *xf;
	char  *buf;
	int	   count;
{
	register int i = 0;
	register int c;
	register DWORD n;
	unsigned int m, mm;

#if 0 /* === no writing! === */
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
errorPrintf("getting next block to write: @%d(%x) offset %ld(%lx)", 
			n, n, xf -> sec_off, xf -> sec_off);
#endif
				if (dRdBlock(xf -> sec_buf, n, 1)) break;
			} else {
				/*
				 * Otherwise, allocate one.
				 */
				n = xAllocB(xf -> sec_loc);
#ifdef WDEBUG
errorPrintf("allocating block to write: @%d(%x) offset %ld(%lx)", 
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
#endif
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
	register int n;

	/*
	 * This is the brute-force way to do it -- simulate a read.
	 */
	if (off < 0 || off > FILE_SIZE(&xf -> rib)) return (-1L);
	n = xf -> sec_loc;		/* in case loop body isn't needed */
	if (off < xf -> sec_off) {
		/* Fake a rewind but don't read the first block */
		xf -> c_ptr   = xf -> rib.clusters;
		xf -> c_sec   = 1;
		xf -> sec_ptr = 0;
		xf -> cur_loc = xf -> sec_off = 0L;
		xf -> sec_loc = CPTR_BLK(xf -> c_ptr) + xf -> c_sec;
	}
	while (xf -> sec_off + xf -> blk_len <= off) {
		if ((n = xNxtBlk(xf)) == 0) return (-1L);
	}
#ifdef DDEBUG
errorPrintf("xseek: reading final block %d(%x)", n, n);
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
}

long xtell(f)
	XFile *f;
{
	return ((long) f -> cur_loc);
}



