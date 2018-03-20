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

#define NAMELEN 15			/* NOTE:  must be at least 1 char longer 	*/
							/*        than the longest name on the disk	*/
							/*		  so names will have null at end.	*/
							/* 32+1 handles bsd4.2 Unix, VMS, and Mac.	*/
							/* 14+1 handles iRMX-86, SysV, and MS-DOS	*/

#ifndef MAXDRIVES
#define MAXDRIVES 26
#endif


/*********************************************************************\
**
** Directory Tree Node
**		One would be tempted to call it File, but Dir is less
**		likely to get confused with FILE.
**
**		isTagged in a directory means that the directory contains
**		tagged files (possibly in a subdirectory), and so needs to
**		be created when copying the tree.
**
**		On the read_file and write_file functions, a count of 0
**		closes the file.
**
\*********************************************************************/

typedef enum {unknown, ascii, binary} FileType;	   /* standard ftypes */

typedef struct _DirRec *Dir;
typedef struct _DirClassRec *DirClass;

typedef struct _DirPart {
	unsigned	isDir:	1;		/* TRUE if is directory */
	unsigned	isOpen:	1;		/* TRUE if has been opened */
	unsigned	isVisible: 1;	/* TRUE if visible on screen */
	unsigned	isChanged: 1;	/* TRUE if file has been written */
	ushort		ftype;			/* ascii/binary/whatever */
	ulong		size;			/* size in bytes */
	ulong		fsize;			/* total file size */
	ushort		fcount;			/* file count if dir, 1 if file */
	ushort		dcount;			/* directory count if dir, 0 if file */
	ulong		tsize;			/* total size of tagged files */
	ushort		tcount;			/* count of tagged files */
	ulong		time;			/* time of last write: Unix format */
								/* sec since 1970/01/01.00:00:00 */
	char		name[NAMELEN];
	ulong		file;			/* handle for open file */
} DirPart;

typedef struct _DirRec {
	ObjectPart	object;
	TreePart	tree;
	DirPart		dir;
} DirRec, *Dir;

typedef struct _DirClassPart {
	int			name_size;
	Dir			freelist;
	Dir			(*read_tree)();	/* (drive) -> root */
	Dir			(*create_file)(); /* (dir, name, dir) -> new file */
	Bool		(*open_file)();	/* (dir, string) -> success */
	Bool		(*close_file)();/* (dir) -> success */
	int			(*read_file)();	/* (dir, &buf, size) -> count */
	int			(*write_file)();/* (dir, &buf, size) -> count */
	unsigned	(*validate)();	/* (dir) -> validate drive */
	int			(*seek)();		/* (dir, loc) -> success */
	long		(*tell)();		/* (dir) -> loc */
	Bool		(*rename)();	/* (dir, name) -> success */
	Bool		(*unlink)();	/* (dir) -> success */
} DirClassPart;

typedef struct _DirClassRec {
	ObjectPart	obj;
	ObjectClassPart objClass;
	TreeClassPart	treeClass;
	DirClassPart	dirClass;
} DirClassRec;

global Class clDir;

#define ISDIR(d)	(((Dir)(d)) -> dir.isDir)
#define fFile(d)	((Dir)(d) -> dir.file)

#define dirFreelist(c)		(((DirClassRec *)(c)) -> dirClass.freelist)
#define cfReadTree(o)		CMETH((o), DirClass, dirClass.read_tree)
#define gCreateFile(o,s,d)	METH2((o), DirClass, dirClass.create_file, s, d)
#define gOpenFile(o,s)		METH1((o), DirClass, dirClass.open_file, s)
#define gCloseFile(o)		METH0((o), DirClass, dirClass.close_file)
#define gReadFile(o,b,n)	METH2((o), DirClass, dirClass.read_file, b,n)
#define gWriteFile(o,b,n)	METH2((o), DirClass, dirClass.write_file,b,n)
#define gValidate(o)		METH0((o), DirClass, dirClass.validate)
#define gSeek(o,l)			METH1((o), DirClass, dirClass.seek,l)
#define gTell(o)			METH0((o), DirClass, dirClass.tell,l)
#define gRename(o,s)		METH1((o), DirClass, dirClass.rename,s)
#define gUnlink(o)			METH0((o), DirClass, dirClass.unlink)

/*********************************************************************\
**
** Drives & Other Variables
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
global int	 inUse[MAXDRIVES];		/* actually use count */

global char	 dWorkingPath[256];
global unsigned dWorkingDrive;		/* A = 1, as in DOS; 0 = invalid */

#define dDriveNum(str)	(((*(str) == '.') ? *dWorkingPath : *(str)) - 'A')

global VoidFunc dEachNewDir;	/* Called on each new Dir when reading  */
								/* a whole tree.						*/

extern char *dFtypeName[];	  	/* Array of filetype names for this app. */


/*********************************************************************\
**
** Functions
**
\*********************************************************************/

global long	  dTaggedSize();	/* (n) -> size	total tagged size		*/
global long	  dTag();			/* (n) -> size	tag node				*/
global long	  dUntag();			/* (n) -> size	untag node				*/

global Dir	  dReadDirTree();	/* (drive)	read a directory tree		*/

global Dir	  dReadDosTree();	/* (drive)	read tree for a drive		*/
global Bool   dSetWorkingDir();	/* (n)		set working directory		*/



