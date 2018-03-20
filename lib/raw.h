/*** headerID = "raw.h 1.0 (c) 1989 S. Savitzky ***/

/*********************************************************************\
**
**	Raw -- 	Header file for Raw Disk pseudo-directory class
**
**	891209	SS	create 
**
\*********************************************************************/


/*********************************************************************\
**
** Exported Routines -- Raw Disk
**
**	NOTE:  Drive numbers specified to these routines are logical:
**		   drive A = 0, B = 1, etc.
**		   dPhysDrive[d] converts these to physical drive numbers
**
\*********************************************************************/

global Dir dReadRaw();			/* (drive, view) -> root	read raw dsk*/
global int dReadTrack();		/* (dir, buf, len)					    */
global Dir dSetRaw();			/* (dir, dens, ds, spt, seclen)			*/


/*********************************************************************\
**
** DskDir class
**
**		This rather peculiar class allows a raw, physical disk
**		to be treated as a "directory" with each track a "file"
**
\*********************************************************************/

typedef struct _DirClassRec *DskDirClass;

typedef struct _DskDirPart {
	int			drive;					/* drive number */
	int			track;					/* track number/max */
	int			head;					/* head number/max */
	char		dens;					/* 0 = low, 1 = double, 2 = high */
	char		ds;						/* 0 = ss; 1 = ds */
	int			spt;					/* sectors per track */
	int			seclen;					/* sector length */
	long		loc;					/* location for seeks */
} DskDirPart;

typedef struct _DskDirRec {
	ObjectPart	object;
	TreePart	tree;
	DirPart		dir;
	DskDirPart 	dsk_dir;
} DskDirRec, *DskDir;

global Class clDskDir;



