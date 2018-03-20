/*** headerID = "irmx.h 1.0 (c) 1989 S. Savitzky ***/

/*********************************************************************\
**
**	irmx -- 	Header file for iRMX-86 disk format
**
**	890626	SS	create 
**
\*********************************************************************/

/*********************************************************************\
**
** Disk Data Types (and flag definitions)
**
**		Note that we avoid mention of iRMX-86 so as to make it
**		easier to port this to the next machine.
**
** MACHINE DEPENDENT!
**
**		The use of "WORD" and "DWORD" here works only because
**		the target (PC family) is little-endian.
**
\*********************************************************************/

typedef unsigned char BYTE;
typedef unsigned int  WORD;
typedef unsigned long DWORD;

#define MAXBLOCK 1024				/* max # of bytes in a sector */
#define MINBLOCK  128				/* min # of bytes in a sector */

#define T0_GRAN	  128				/* size of block in track 0 */
#define T0_BLKS	   16				/* # of blocks in track 0 */
#define T0_BYTES (T0_GRAN * T0_BLKS)

typedef struct volume_info {
	BYTE	vol_name[10];			/* left-just; zero-fill */
	BYTE	flags;					/* see below */
	BYTE	file_driver;			/* = 4 */			   	
	WORD	vol_gran;				/* size of volume block in bytes */
	DWORD	vol_size;				/* in bytes */
	WORD	max_fnode;				/* # fnodes in fnode file */
	DWORD	fnode_start;			/* first byte in fnode file (0 orig) */
	WORD	fnode_size;				/* size of an fnode in bytes */
	WORD	root_fnode;				/* the fnode of the root directory */
	WORD	dev_gran;				/* size of a sector (except trk 0) */
	WORD	interleave;				/* block interleave */
	WORD	track_skew;				/* skew between tracks */
	WORD	system_id;				/* magic number */
	BYTE	system_name[12];		/* iRMX 86<other stuff> */
	BYTE	device_special[8];		/* zero on floppies */
} VolInfo;

/* VolInfo flag bits */

	/* 
	** Note that the Intel documentation defines the same names,
	** but defines them as bit numbers 
	*/

#define VF_AUTO		1				/* 1 = valid for auto device recog. */
#define VF_DENSITY	2				/* 0 = FM; 1 = MFM */
#define VF_SIDES	4				/* 1 = double-sided */
#define VF_MINI		8				/* 1 = 5.25 inch disk */
#define VF_NOT_FLOPPY 16			/* 1 = winchester */

/* Disk pointers -- these structures are actually not used */

typedef struct dir_ptr {	/* direct */
	WORD	num_blocks;				/* size of this extent */
	BYTE	blk_ptr[3];				/* starting block number, LSB first */
} DirPtr;

typedef struct ind_ptr {   	/* indirect */
	BYTE	num_blocks;				/* size of this extent */
	BYTE	blk_ptr[3];				/* starting block number, LSB first */
} IndPtr;

/* ... instead, we use these architecture-independent macros */

#define DPTR_SIZE 5
#define DPTR_NUM(p) ((p)[0]+((short)(p)[1]<<8))
#define DPTR_BLK(p) ((p)[2]+((ulong)(p)[3]<<8)+((ulong)(p)[4]<<16))
#define DPTR_NUM_(p,n) ((p)[0]=((n) & 255), (p)[1] = ((n) >> 8) & 255)
#define DPTR_BLK_(p,n) ((p)[2]=((n) & 255), (p)[3] = ((n) >> 8) & 255,\
						(p)[4]=((n) >> 16) & 255)

#define IPTR_SIZE 4
#define IPTR_NUM(p) ((ushort)(p)[0])
#define IPTR_BLK(p) ((p)[1]+((ulong)(p)[2]<<8)+((ulong)(p)[3]<<16))
#define IPTR_NUM_(p,n) ((p)[0]=((n) & 255))
#define IPTR_BLK_(p,n) ((p)[1]=((n) & 255), (p)[2] = ((n) >> 8) & 255,\
						(p)[3]=((n) >> 16) & 255)

/* ... That way, even if some future compiler decides to word-align
**     structures, or we port to the 68000, we're still clean.
*/

/* fnode */

typedef struct fnode {
	WORD	flags;
	BYTE	type;
	BYTE	gran;	  				/* file granularity in vol blks */
	WORD	owner;					/* -1 for root */
	DWORD	cr_time;				/* seconds from 1-1-78 */
	DWORD	access_time;
	DWORD	mod_time;
	DWORD	total_size;				/* size of data in bytes */
	DWORD	total_blks;				/* # of vol blocks */
	BYTE	pointr[40];				/* first 8 block ptrs */
	DWORD	this_size;				/* size allocated, in bytes */
	WORD	reserved_a;	
	WORD	reserved_b;
	WORD	id_count;				/* # of pairs in ACC field */
	BYTE	acc[9];					/* access rights[3] */
	WORD	parent;					/* fnode # of parent dir. */
	BYTE	aux[3];					/* auxiliary data */
} Fnode;

/* Fnode flag bits */

#define FN_ALLOC	1			/* 0 = free; 1 = file */
#define FN_LONG		2			/* 0 = short; 1 = long */
#define FN_RES		4			/* reserved = 1 */
#define FN_MOD		32			/* 1 if modified */
#define FN_DELETE	64			/* 1 if temporary */

/* Fnode file types */

#define FT_FNODE	0			/* fnode file 				Fnode 0 */
#define FT_VOLMAP	1			/* volume free space map.  	Fnode 1 */
#define FT_FNODEMAP	2			/* free fnodes map 			Fnode 2 */
#define FT_ACCOUNT	3			/* space accounting file 	Fnode 3 */
#define FT_BADBLOCK	4			/* device bad blocks file	Fnode 4 */
#define FT_DIR		6			/* directory -- root is in  Fnode 6 */
#define FT_DATA		8			/* data file */
#define FT_VLABEL	9			/* volume label file		Fnode 5 */

/* Directory entry */

typedef struct dir_entry {
	WORD	fnode;
	BYTE	component[14];		/* left justified, zero padded name */
} DirEntry;

/*********************************************************************\
**
** Disk Layout Constants (absolute byte positions)
**
**		Note that sectors are numbered from 1
**		Note that starting files in sector 27 makes sense for
**		8-inch single density disks, which have 26 sectors/track.
**
\*********************************************************************/

#define D_BOOTLOC		0
#define D_VOL_LBL_LOC	384			/* Sector 4 if single density */
#define D_ISO_LBL_LOC	768
#define D_FILE_LOC		3328		/* Sector 27 if single density */


/*********************************************************************\
**
** XFile, the logical equivalent of FILE
**
\*********************************************************************/

typedef struct XFile {
	VolInfo *vol_info;				/* -> volume info */
	Fnode	fnode;					/* the file's fnode */
	int		fnode_num;				/* the file's fnode number */
	int		blk_len;				/* volume block length */

	uchar   *dir_ptr;				/* -> current direct pointer */
	int		dir_sec;				/* current sector within it */

	uchar  *ind_buf;				/* -> indirect pointer block */
	DWORD	ind_loc;				/* its logical block number */
	uchar  *ind_ptr;				/* -> current pointer */
	int		ind_sec;				/* sector's index in pointer's extent */

	uchar  *sec_buf;				/* -> current sector buffer */
	DWORD	sec_loc;				/* its logical block number */
	DWORD	sec_off;				/* its location in bytes from BOF */
	int     sec_ptr;				/* current r/w offset into sec_buf */
	DWORD	cur_loc;				/* its location from BOF */

	Bool	fnode_dirty: 1;			/* dirty bits */
	Bool	ind_dirty: 1;
	Bool	sec_dirty: 1;
} XFile;

/*********************************************************************\
**
** Exported Routines and Data
**
\*********************************************************************/

/*
** Operations on XFiles
**		Operations whose names end in N take an fnode number
**		where the corresponding Unix operation takes a filename.
*/
int xstatN();			/* (fn, &buf) */
int xrewind();			/* (xf) */
void xclose();			/* (xf) */
XFile *xopenN();		/* (fn, flags) */
int xread();			/* (xf, buf, count) */
int xwrite();			/* (xf, buf, count) */
long xseek();			/* (xf, off) */
long xtell();			/* (xf) */

/*
** Volume info for drives; canned volume info.
*/
global int			drvVolInfo[];
global VolInfo		*volInfos[];



/*********************************************************************\
**
** XDir class
**
\*********************************************************************/

typedef struct _DirClassRec *XDirClass;

typedef struct _XDirPart {
	int			fnode;
} XDirPart;

typedef struct _XDirRec {
	ObjectPart	object;
	TreePart	tree;
	DirPart		dir;
	XDirPart 	x_dir;
} XDirRec, *XDir;

/*********************************************************************\
**
** VolVr class -- viewer for volume format info
**
\*********************************************************************/

typedef struct _VolVrPart {
	int			cur;			/* index of current line */
	int			lim;			/* index of last line */
} VolVrPart;

typedef struct _VolVrRec {
	ObjectPart 	object;
	VolVrPart	volVr;
} VolVrRec, *VolVr;

global Class clVolVr;

