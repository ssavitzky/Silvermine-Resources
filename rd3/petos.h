/*** headerID = "petos.h 1.0 (c) 1990 S. Savitzky ***/

/*********************************************************************\
**
**	petos.h -- 	Header file for PETOS disk format
**
**	890626	SS	create 
**
\*********************************************************************/

/*********************************************************************\
**
** Disk Data Types (and flag definitions)
**
\*********************************************************************/

typedef unsigned char BYTE;
typedef unsigned int  WORD;
typedef unsigned long DWORD;

#define SECLEN	128					/* size of a sector */

/* Track 0 layout (by sector number) */

#define T0_VolInfo	1				/* Volume info sector */
#define T0_SecTable	2				/* Sector usage table (4-sec clusters) */
#define T0_DirRIB	5				/* Directory's RIB */
#define T0_DirLoc	6				/* Directory data start */
#define DIRLEN		88				/* Directory contains 88 entries */


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

/*
** Directory Entries
*/
typedef struct _DirEntry {
	char	fname[5];
	char	ftype[2];
	uchar	pad0[3];
	uchar	rib_loc[2];
	uchar	pad1[3];				/* === total 16 bytes === */
	uchar	reclen;
} DirEntry;

/*
** RIB (Retrieval Information Block) sectors and their contents
**		The documentation seems to be wrong about the order of 
**		nsecs and nbytes.
*/
typedef struct _RIB {
	uchar	clusters[99];			/* cluster info */
	char	cdate[6];				/* creation date:	YYMMDD */
	char	mdate[6];				/* modified date:	YYMMDD */
	char	pad1[4];
	uchar	nbytes;					/* #bytes used in last sector */
	uchar	nsecs[2];				/* #sectors in file */
	char	pad2[2];
	char	ftype;					/* file type */
	/* The rest of the block contains task data, which we ignore */
	char	pad3[7];
} RIB;

#define X_INT(p)		((255 & (p)[0]) * 256 + (255 & (p)[1]))
#define X_INT_(p,n)		((p)[1]=((n) & 255), (p)[0] = ((n) >> 8) & 255)
#define FILE_SIZE(rp) 	((255 & (rp) -> nbytes)	\
							? (X_INT((rp) -> nsecs) ) * 128L \
								+ (255 & (rp) -> nbytes) \
							: (X_INT((rp) -> nsecs) ) * 128L )

/* We don't actually use the following format for a cluster pointer... */

typedef struct _Cluster {
	BYTE count;
	BYTE hi;
	BYTE lo;
} Cluster;

/* ... instead, we use these architecture-independent macros  */
/*     which invisibly convert from cluster counts to sectors */

#define CPTR_SIZE 3
#define CPTR_NUM(p) (((p)[0] & 255) * 4)
#define CPTR_BLK(p) (4 * ((p)[2]+((unsigned int)(p)[1]<<8)))
#define CPTR_NUM_(p,n) ((p)[0]=(((n)/4) & 255))
#define CPTR_BLK_(p,n) ((p)[2]=(((n)/4) & 255), (p)[1] = (((n)/4) >> 8) & 255)

/* ... That way, even if some future compiler decides to word-align
**     structures, or we port to the 68000, we're still clean.
*/


/*********************************************************************\
**
** XFile, the logical equivalent of FILE
**
\*********************************************************************/

typedef struct XFile {
	RIB		rib;					/* the file's RIB */
	int		rib_loc;				/* The RIB's block number */
	int		blk_len;				/* volume block length */

	uchar   *c_ptr;					/* -> current cluster pointer */
	int		c_sec;					/* current sector within it */

	uchar  *sec_buf;				/* -> current sector buffer */
	DWORD	sec_loc;				/* its logical block number */
	DWORD	sec_off;				/* its location in bytes from BOF */
	int     sec_ptr;				/* current r/w offset into sec_buf */
	DWORD	cur_loc;				/* its location from BOF */

	Bool	rib_dirty: 1;			/* dirty bits */
	Bool	sec_dirty: 1;
	Bool	text_mode: 1;
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



/*********************************************************************\
**
** XDir class
**
\*********************************************************************/

typedef struct _DirClassRec *XDirClass;

typedef struct _XDirPart {
	int			loc;
	DirEntry	dirent;
	char 		mdate[6];
} XDirPart;

typedef struct _XDirRec {
	ObjectPart	object;
	TreePart	tree;
	DirPart		dir;
	XDirPart 	x_dir;
} XDirRec, *XDir;


