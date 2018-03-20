/*** headerID = "dirs.h 1.0 (c) 1988 S. Savitzky ***/

/*********************************************************************\
**
**	Dirs -- 	Header file for Directory trees
**
**	880111	SS	create from DD/data
**	880419	SS	rename from data to dirs
**	880619	SS	remove dirs.files
**
\*********************************************************************/

#define NAMELEN 33			/* NOTE:  must be at least 1 char longer 	*/
							/*        than the longest name on the disk	*/
							/*		  so names will have null at end.	*/
							/* 32+1 handles bsd4.2 Unix, VMS, and Mac.	*/

#define MAXDRIVES 16

#define STATICBUF 1			/* if true, allocate a big static buffer	*/
#define STATICBUFSIZE 120	/* in K */
#define STATICBUFMAX  (1024L * STATICBUFSIZE)


/*********************************************************************\
**
** Directory Tree Node
**		Note that subdirectories and files are kept separately
**		for convenience.  (subs and files are null for a file node)
**
**		One would be tempted to call it File, but Dir is less
**		likely to get confused with FILE.
**
**		isTagged in a directory means that the directory contains
**		tagged files (possibly in a subdirectory), and so needs to
**		be created when copying the tree.
**
\*********************************************************************/

typedef enum {unknown, ascii, binary, css, jcamp, spectrum} FileMode;

typedef struct _DirRec *Dir;
typedef struct _DirClassRec *DirClass;

typedef struct _DirPart {
	unsigned	isDir:	1;		/* TRUE if is directory */
	unsigned	isOpen:	1;		/* TRUE if has been opened */
	FileMode	mode;			/* ascii/binary/JCAMP */
	ulong		size;			/* size in bytes */
	ulong		fsize;			/* total file size */
	ushort		fcount;			/* file count if dir, 1 if file */
	ushort		dcount;			/* directory count if dir, 0 if file */
	ulong		tsize;			/* total size of tagged files */
	ushort		tcount;			/* count of tagged files */
	ulong		time;			/* time of last write: Unix format */
								/* sec since 1970/01/01.00:00:00 */
	char		name[NAMELEN];
} DirPart;

typedef struct _DirRec {
	ObjectPart	object;
	TreePart	tree;
	DirPart		dir;
} DirRec, *Dir;

typedef struct _DirClassPart {
	Dir			freelist;
} DirClassPart;

typedef struct _DirClassRec {
	ObjectPart	obj;
	ObjectClassPart objClass;
	TreeClassPart	treeClass;
	DirClassPart	dirClass;
} DirClassRec;

global Class clDir;

#define ISDIR(d)	(((Dir)(d)) -> dir.isDir)

#define dirFreelist(c)	(((DirClassRec *)(c)) -> dirClass.freelist)

/*********************************************************************\
**
** Drives
**
**		The dDrives array keeps track of what's in which drive.
**		Among other things, it is used to make sure that we don't
**		try to read a drive in two different operating systems.
**
**		0 in dWorkingDrive means that a volume from another OS has
**		been mounted in that drive, so dWorkingPath is invalid.
**
**		dWorkingPath never changes.	 It's only the root pathname
**		for '.' when that's the virtual root.
**
\*********************************************************************/

global Dir   dDrives[MAXDRIVES];
global Bool	 inUse[MAXDRIVES];

global char	 dWorkingPath[256];
global unsigned dWorkingDrive;		/* A = 1, as in DOS; 0 = invalid */

#define dDriveNum(str)	(((*(str) == '.') ? *dWorkingPath : *(str)) - 'A')

global VoidFunc dEachNewDir;	/* Called on each new Dir when reading  */
								/* a whole tree.						*/

/*********************************************************************\
**
** Static buffer crockery
**
\*********************************************************************/

#if STATICBUF

global  char huge BigBuf[STATICBUFSIZE][1024];

#define BufAlloc(x,y)	(((long)(x)*(long)(y)/1024L > STATICBUFSIZE)? 0 : (char huge *)BigBuf)
#define BufFree(x)		/* nothing */

#else
#define	BufAlloc(x,y) 	halloc(x, y)
#define BufFree(x)		hfree(x)
#endif


/*********************************************************************\
**
** Functions
**
\*********************************************************************/

global long	  dTaggedSize();	/* (n) -> size	total tagged size		*/
global long	  dTag();			/* (n) -> size	tag node				*/
global long	  dUntag();			/* (n) -> size	untag node				*/

global Dir	  dReadDosDir();	/* (drive)	read a directory tree		*/
global Dir	  dReadDosTree();	/* (drive)	read tree for a drive		*/
global Bool   dSetWorkingDir();	/* (n)		set working directory		*/

global char huge *dReadDosFile();   /* (node)  read entire file into	*/
									/*			an allocated buffer		*/
global char huge *dReadDosBuffer(); /* (n, b)  read more of file if		*/
									/* buffer too small the first time	*/


