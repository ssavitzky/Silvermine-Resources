/*** headerID = "disk.h 1.0 (c) 1988 S. Savitzky ***/

/*********************************************************************\
**
**	Disk -- 	Header file for Disk utilities
**
**	880111	SS	create 
**
\*********************************************************************/

/* combined headers from 7000,for reading idris disks on IBM */
/*	THE STANDARD HEADER
 *	copyright (c) 1978 by Whitesmiths, Ltd.
 */

/* the pseudo storage classes
 */
#define FAST	register
#define GLOBAL	extern
#define IMPORT	extern
#define INTERN	static
#define LOCAL	static

/* the pseudo types
 */
typedef unsigned char TEXT;
typedef TEXT TBOOL;
typedef char TINY;
typedef double DOUBLE;
typedef float FLOAT;
typedef int ARGINT, BOOL, VOID;
typedef int COUNT, METACH;
typedef long LONG;
typedef unsigned BYTES;
typedef unsigned char UTINY;
typedef unsigned long ULONG;
typedef unsigned short BITS, UCOUNT;

/* system parameters
 */
#define STDIN	0
#define STDOUT	1
#define STDERR	2
#define YES		1
#define NO		0
#define FOREVER	for (;;)
#define BUFSIZE	512
#define BWRITE	-1
#define READ	0
#define WRITE	1
#define UPDATE	2
#define BYTMASK	0377


/*  macros
 */
#define isblk(mod)  (((mod) & 060000) == 060000)
#define ischr(mod)  (((mod) & 060000) == 020000)
#define isdir(mod)  (((mod) & 060000) == 040000)

#define DIRSIZE 14

/*  directory structure
 */
typedef struct {
	UCOUNT d_ino;
	TEXT d_name[DIRSIZE];
} DIR;

/*	INODE AND SUPERBLOCK STRUCTURES
 *	copyright (c) 1980 by Whitesmiths, Ltd.
 */

/*	scalar types
 */
#define BLOCK	UCOUNT
#define INUM	UCOUNT
#define UID		UTINY

/*	the filesystem superblock
 */
typedef struct {
	BLOCK s_isize;
	BLOCK s_fsize;
	UCOUNT s_nfree;
	BLOCK s_free[100];
	UCOUNT s_ninode;
	INUM s_inode[100];
	TEXT s_pad[4];
	LONG s_time;
} FILSYS;

/*	inode usage bits
 */
#define IALLOC	0100000
#define IFMT	060000
#define ILARG	010000
#define ISUID	004000
#define ISGID	002000

/*	the filesystem inode
 */
typedef struct {
	BITS n_mode;
	TEXT n_link;
	UID n_uid;
	UID n_gid;
	UTINY n_size0;
	UCOUNT n_size1;
	BLOCK n_addr[8];
	LONG n_actime;
	unsigned n_uptime0;
	unsigned n_uptime1;
} FINODE;


/*********************************************************************\
**
** Variables
**
\*********************************************************************/

global int  dErrors;			/* # of disk errors */


/*********************************************************************\
**
** Routines
**
\*********************************************************************/

global int dDrive;				/* [var]	drive number (default B)	*/

global Dir dReadDisk();			/* (drive, view) -> root	read disk	*/
global long dOpenFile();		/* (node)	open a file	for reading		*/
								/*  	 -> length; -1 if nonexistant	*/
global int dNextBlock();		/* (buf)	read next block of file		*/
								/*		 -> non-zero on error or EOF	*/

global int dNextChar();			/* () ->	next char in file			*/

global char huge *dReadIdrisFile(); /* (node)  read entire file into	*/
									/*			an allocated buffer		*/
global char huge *dReadIdrisBuffer(); /* (n, b)  read more of file if	*/
									/* buffer too small the first time	*/


/*********************************************************************\
**
** IdrisDir class
**
\*********************************************************************/

typedef struct _IdrisDirRec *IdrisDir;
typedef struct _DirClassRec *IdrisDirClass;

typedef struct _IdrisDirPart {
	short		inode;
} IdrisDirPart;

typedef struct _IdrisDirRec {
	ObjectPart	object;
	TreePart	tree;
	DirPart		dir;
	IdrisDirPart idris_dir;
} IdrisDirRec, *IdrisDir;

global Class clIdrisDir;

