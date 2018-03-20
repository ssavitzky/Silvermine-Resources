/***/static char *moduleID="disk 1.0 (c)1988 S. Savitzky";/***/

/*********************************************************************\
**
**	D I S K   H A N D L I N G
**
**		Here is where we read 7000-format disks, files, and directories.
**		(This was originally three separate files, but they have been
**		 combined here for reasons that seemed good at the time.)
**
**	HOLES:
**		usage of the variable "rderror" needs to be checked.
**		error handling in general needs to be put in.
**
**	880111 SS	create PC version based on stuff from A. Savitzky
**
\*********************************************************************/

#include "rd7.h"
#include "coops.h"
#include "trees.h"
#include "dirs.h"

#include <dos.h>
#include <stdio.h>
#include <signal.h>
#include <malloc.h>
#include "../lib/curse.h"

#undef  global
#define global
#include "disk.h"


extern Object objInit(), objClone(), objKill(), objNew();
extern Object objDoesNotImplement();
extern Tree   treeNext(), treePrev(), treeDown(), treeUp(),
			  treeSucc(), treePred(), treeFirst(), treeLast(),
			  treeAfter(), treeBefore(), treeFront(), treeBack(),
			  treeCut(), treeFind();
extern Object treeKill();
extern String   treePath(), treeHeader();

extern DirClassRec crDir;
extern String dirHeader(), dirName();
extern Object dirNew(), dirKill(), dirOpen();


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

DiskBase mybase, far*oldbase, far*far*int30vec;

int preverror = 1;		/* prevents retest for every read */


/*********************************************************************\
**
** Stuff to restore disk info after a ctrl-C
**
\*********************************************************************/

void *oldSigInt;		/* control-C signal stash */

void sigIntHdlr()
{
	*int30vec = oldbase;		/* put things back as they were */
	exit(1)	;
}


/*********************************************************************\
**
** dInit512(drive)	-- initialize BIOS table
**
\*********************************************************************/

void dInit512(drv)
	int drv;
{
	int i;

	/* set up the disk base pointer */

	int30vec = (DskBasPtr far*)120L; 	/* interrupt 30 vector */
	oldSigInt = signal(SIGINT, sigIntHdlr);

#ifdef DDEBUG
 	printf("int30vec = %lx, *int30vec = %lx\n", 
 		   (long)int30vec, 
 		   (long)*int30vec);
#endif /* DDEBUG */

	oldbase = *int30vec;				/* save ptr to old disk base table */
	mybase = *oldbase;					/* copy the old table into mine */
	mybase.seclen = 2;					/* 512 bytes */
	mybase.maxsec = 8;    
	mybase.gaplen = 42;					/*as-try */
/*	mybase.datalen = 128; */
/*	mybase.headsettle =15; */
/*	mybase.motorstart = 8; */

	*int30vec = &mybase;				/* point int 30 at new table */

#ifdef DDEBUG
 	printf("old disk base table: %lx\n", (long)oldbase);
 	for (i = 0; i < sizeof(mybase); ++i) {
 		printf("%02x ", 0xff & *(((unsigned char far*)oldbase) + i));
 	}
 
 	printf("\n");
 	printf("new disk base table: %lx\n", (long)(DskBasPtr)&mybase);
 	for (i = 0; i < sizeof(mybase); ++i) {
 		printf("%02x ", 0xff & *(((unsigned char *)&mybase) + i));
 	}
 	printf("\n");
#endif /* DDEBUG */
}


/*********************************************************************\
**
** dCloseDisk()	restore disk table to its original state
**
\*********************************************************************/

void dCloseDisk()
{
	*int30vec = oldbase;		/* put things back as they were */
	signal(SIGINT, oldSigInt);
}



/*********************************************************************\
**
** errcode = dRdSector(buf, track, side, sector, count)	-- read sector
**
\*********************************************************************/

int dRdSector(bufr, track, side, sector, count)
	char *bufr;
	int side, track, sector, count;
{
	int maxtries=10, try, retval, rval;
	char ef;			/* error flags */
	char *es;			/* error string */
	
#ifdef DDEBUG
	printf("track: %d, side: %d, sector: %d\n", 
	   	track, side, sector);
#endif /* DDEBUG */

	if (track < 0 || sector < 0 || sector > 8) {
		dCloseDisk();
		fprintf(stderr, "bogus read to track %d, sector %d\n", 
				track, sector);
		exit();
	}

	*int30vec = &mybase;
	for (try = 0; try < maxtries; ++try) {
		if (preverror)	{
			inregs.h.ah = 0;			/* reset */
			rval = int86(0x13, &inregs, &outregs);
			preverror = 0;
		}
		inregs.h.dl = dDrive;
		inregs.h.dh = side;
		inregs.h.ch = track;
		inregs.h.cl = sector;
		inregs.h.al = count;		/* No. Sectors to be read */
		inregs.h.ah = 2;			/* read diskette */
		inregs.x.bx = (int)bufr;
		retval = int86(0x13, &inregs, &outregs);
		if ((outregs.x.cflag) == 0) break;
		else preverror = 1;
	}
	/*
	** One ought to be able to check for outregs.h.al == 0
	** but some BIOSs aparently blow this.
	*/
	if (outregs.x.cflag != 0) {
		++dErrors;
		ef = outregs.h.ah;
		if      (ef & 0x80) es = "Disk not ready";
		else if (ef & 0x40) es = "Seek failure (DOS disk?)";
		else if (ef & 0x20) es = "Controller Error (DOS disk?)";
		else if (ef & 0x10) es = "CRC Error (DOS disk?)";
		else if (ef & 0x08) es = "DMA overrun (DOS disk?)";
		else if (ef & 0x04) es = "Sector not found (DOS disk?)";
		else if (ef & 0x02) es = "Write protected";
		else if (ef & 0x01) es = "Illegal command";
		errorPrintf(
"Error: %s, flags= %04d, trk: %d hd: %d sec: %d",
				es, track, side, sector);
#ifdef DDEBUG
		dCloseDisk();
		exit();
#endif /* DDEBUG */
		*int30vec = oldbase;
		signal(SIGINT, oldSigInt);
		return(0xFF00 | (int)outregs.h.ah);
	}
	*int30vec = oldbase;	/* In case copying from floppy to floppy */
	return(0);
}

/*********************************************************************\
**
** int dRdBlock(buf, logsect)		read logical block
**
**		Convert logical sector to physical track, side, and sector;
**		Read the block and return the error code if any.
**
**		logical sector = 8 * (track * 2) + 8 * side + sector - 1
**
\*********************************************************************/

int dRdBlock(buf, logsect, count)
	char *buf;
	int logsect;
	int count;
{
	int ts;
	int track, side, sector;

	sector = (logsect % 8) +1;
	ts = (logsect)/8;
	side = ts % 2;
	track = (ts - side)/2;

dstatn(5, "block %5d", logsect);

	return (dRdSector (buf, track, side, sector, count));
}


/*********************************************************************\
**
** F I L E S
**
**		This stuff implements the necessary data structures for
**		reading ONE FILE AT A TIME from the current 7000 disk.
**
\*********************************************************************/

BLOCK largeblock[256];			/* block pointers for large file */
BLOCK d_addr[8 + 256];
#define largelarge (d_addr + 8)	/* second-level block */

int nextaddr;
int nextlarge;
int largeflag;
int rderror;

FINODE inodeb[16];
FINODE curInode;		/* holds current inode */
int curnodebuf;			/*current inode in buffer */


/*********************************************************************\
**
** rdlarge(largeno)		read block pointers for large files
**
\*********************************************************************/

void rdlarge(largeno)
	int largeno;
{
	if (largeno == 0) {
		register int i;
		for (i = 0; i < 256; ++i) largeblock[i] = 0;
	} else {
		rderror = dRdBlock((char *)largeblock, largeno, 1);
	}
	nextlarge = 0;
}

/*********************************************************************\
**
** logsect = nodeSect(inodeNum)		convert inode # to logical sector
**
\*********************************************************************/

int nodeSect(nodenum)
	int nodenum;
{
	return ((nodenum-1)/16 + 2);
}


/*********************************************************************\
**
** getInode(inum)		read an Inode
**
**		Inodes are packed 16 to a block.  
**		We only buffer a single Inode block, which could be inefficient.
**
\*********************************************************************/

void getInode(inum)
	int inum;
{
	int sector;

dstatn(20, "Inode %5d", inum);

	sector = nodeSect(inum);
	if (sector != curnodebuf) {	/* sector currently in memory */
		rderror = dRdBlock(inodeb, sector, 1);			
	}
/*	curInode = inodeb[(inum-1) % 16]; 	/* why doesn't this work ? */
memcpy(&curInode,&inodeb[(inum - 1) % 16], sizeof(curInode));

}

/*********************************************************************\
**
** initblock(inode)		initialize for block pointers
**
\*********************************************************************/

void initblock(inode)
	int inode;
{
	int i;

	getInode(inode);
	if (rderror) return;
	nextaddr = nextlarge = 0;
	for (i = 0; i < 8; i++)
		d_addr[i] = curInode.n_addr[i];
	largeflag = curInode.n_mode & ILARG;
	if (largeflag) {
		rdlarge(d_addr[0]);
		nextaddr++;
	}
}


/*********************************************************************\
**
** int nextblok()		return the next block number in the current file
**
**		returns 0 at EOF.
**
\*********************************************************************/

int nextblok()
{
	if(largeflag) {
		if (nextlarge >= 256) {
			if (nextaddr == 7) {
				/* 
				** The last pointer in the original inode points to
				** a block of more pointers.  Stick it after the
				** original pointer block and keep going!
				*/
				rderror = dRdBlock(largelarge, d_addr[nextaddr++], 1);
			}
			/* get large block for next read */
			rdlarge(d_addr[nextaddr++]);
		}
		return (largeblock[nextlarge++]);
	} else {
		if (nextaddr < 8 )
			return (d_addr[nextaddr++]);
		else
			return (0);		/* list exhausted */
	}
}


/*********************************************************************\
**
** D I R E C T O R Y   T R E E S
**
**		Stuff in this section reads a complete directory tree into
**		the in-core tree structure (see DATA.C)
**
\*********************************************************************/


char buf[BUFSIZE];		/* note local buffers */
DIR dbuf[32];

/*********************************************************************\
**
** dRdDir(Dir n)			read a directory file
**
**		dRdDir takes a Dir node that refers to a directory, and
**		reads that directory, constructing nodes for its immediate
**		subdirectories and ordinary files.
**
\*********************************************************************/


void dRdDir(n)
	IdrisDir n;
{
	register IdrisDir dp;
	int dirblock, dnum;
	int nodenum;
	char c;

	initblock(n -> idris_dir.inode);
	if (rderror) return;
	while (dirblock = nextblok()) {
		rderror = dRdBlock(dbuf, dirblock, 1);			
		if (rderror) return;
		/* 
		** examine each of the 32 entries 
		*/
		for (dnum = 0; dnum < 32; dnum++) {
			/* 
			** Skip entries with null name, zero inode, or
			** name starting with '.'
			*/
			if (!(c = dbuf[dnum].d_name[0]))
				continue;
			if (c == '.')
				continue;
			if (!(nodenum = dbuf[dnum].d_ino))
				continue;
			getInode(nodenum);	/* now curInode contains inode */
			dp = (IdrisDir) cfNew(clIdrisDir)(clIdrisDir, n, 
									&dbuf[dnum].d_name[0],
									isdir(curInode.n_mode),
								    65536L * curInode.n_size0 + 
									  curInode.n_size1
								   );
			if (!dp) {
				errorPrintf(
"Out of memory reading directory \"%s\"; tree will be incomplete.",
						   &dbuf[dnum].d_name[0]);
				break;
			}
			dp -> idris_dir.inode = nodenum;
			/* 
			** Convert PDP-11 order to little-endian Intel order
			*/
			dp -> dir.time = curInode.n_uptime0 * 65536L
						   + curInode.n_uptime1;

			if (dEachNewDir) (*dEachNewDir)(dp);
		}
	}
}


/*********************************************************************\
**
** void dRdTree(Dir n)
**
**		Read the directory tree rooted at n.
**
\*********************************************************************/

void dRdTree(n)
	Dir n;
{
	/*
	** The strategy at this point is to read a directory using rdDir,
	** then recursively read each of its subtrees.
	*/
	dRdDir(n);
	if (rderror) return;
	for (n = (Dir)gDown(n); NOTNULL(n); n = (Dir)gNext(n)) { 
		if (n -> dir.isDir) dRdTree(n);
		if (rderror) return;
	}
}

/*********************************************************************\
**
** R E A D   F I L E   T R E E
**
\*********************************************************************/

int dDrive = 0;


union  {
	FILSYS s;			/* The superblock */
	char fill[512];		/* so can read in whole sector */
} superb;
	
Dir dReadDisk (drive)
	int  drive;
{
	char driveName[16];
	IdrisDir  n;

	dDrive = drive;
	inUse[dDrive] = TRUE;
	sprintf(driveName, "/%d", dDrive);

	dErrors = 0;

	dInit512(dDrive);

	/*
	** Read SuperBlock
	** init inode buffer
	*/
	rderror = dRdSector(&superb, (int)0, (int)0, (int)2, 1);	  
 	if (rderror) goto errorExit;

	curnodebuf = -1;	 		/* no inode currently in memory */
	dErrors = 0;

	/*
	** Construct Root Node
	*/
	n = (IdrisDir) cfNew(clIdrisDir)(clIdrisDir, NIL, driveName, TRUE, 0L);
	if (NOTNULL(dDrives[dDrive])) gKill(dDrives[dDrive]);
	dDrives[dDrive] = (Dir) n;
	if (!n) {
		errorPrintf(
"Out of memory creating root; tree will be empty."
				   );
		return ((Dir) n);
	}
	n -> idris_dir.inode = 1;
	if (dEachNewDir) (*dEachNewDir)(n);
	/* 
	** traverse tree 
	*/
	dRdTree(n);
	
	/*
	** Put the OS tables back
	*/
errorExit:
	dCloseDisk();
	return ((Dir) n);
}


/*********************************************************************\
**
** R E A D   F I L E S
**
**		This section supports the reading of individual files.
**		You can read them either a block or a character at a time.
**
\*********************************************************************/

long  bytesLeft;
char  theBuf[BUFSIZE];
int	  bytesInBuf;
char *bufPtr;

/*********************************************************************\
**
** length = dOpenFile(node)		-- open a file for reading
**
\*********************************************************************/

long dOpenFile(n)
	IdrisDir n;
{
	initblock(n -> idris_dir.inode);
	bytesInBuf = 0;
	return (bytesLeft = n -> dir.size);
}


/*********************************************************************\
**
** int dNextBlock(buf)		-- read next block of file
**	
**		return -1 at EOF, non-zero on error
**
**		NOTE:  actually never returns EOF because it's perfectly
**			   ok to have a 0 block address, meaning an omitted block.
**			   We rely on bytesLeft to determine end of file.
**
\*********************************************************************/

int dNextBlock(buf)
	char *buf;
{	
	int blk;

	blk = nextblok();
	if (blk == 0) {
		register int i;
		for (i = 0; i < BUFSIZE; ++i) buf[i] = 0;
	} else {
		if (rderror = dRdBlock(buf, blk, 1)) return (-1);
	}
	return (0);
}

/*********************************************************************\
**
** int dNextChar()			-- read next char of file
**							   returns -1 at EOF
**
\*********************************************************************/

int dNextChar()
{
	if (bytesLeft-- == 0) return(-1);
	if (!bytesInBuf) {
		if (dNextBlock(theBuf)) return (-1);
		bufPtr = theBuf;
		bytesInBuf = BUFSIZE;
	}
	--bytesInBuf;
	return(*bufPtr++ & 255);
}

/*********************************************************************\
**
** char huge *dReadIdrisFile(f)
**
**		Suck the whole file into a buffer.
**
\*********************************************************************/

static ulong sizeRemaining;

char huge *dReadIdrisFile(f)
	IdrisDir f;
{
	register char huge *p;
	register int   c;
	ulong size;
	char huge *buf;

	size = f -> dir.size;
	if (size > STATICBUFMAX) size = STATICBUFMAX;
	sizeRemaining = f -> dir.size - size;
	if (!(buf = BufAlloc(size, 1))) {
		return (buf); 
	}

	dInit512(dDrive);
	dOpenFile(f);
	for (p = buf; size-- && (c = dNextChar()) != -1; ++p) *p = c;
	dCloseDisk();
	return (buf);
}

char huge *dReadIdrisBuffer(f, b)
	IdrisDir f;
	char huge *b;
{
	register char huge *p;
	register int   c;
	ulong size;
	char huge *buf;

	size = sizeRemaining;
	if (size > STATICBUFMAX) size = STATICBUFMAX;
	sizeRemaining -= size;
	buf = b;

	dInit512(dDrive);
	for (p = buf; size-- && (c = dNextChar()) != -1; ++p) *p = c;
	dCloseDisk();
	return (buf);
}


/*********************************************************************\
**
** Classes
**
\*********************************************************************/

global DirClassRec crIdrisDir = {
	(Class)&crDir,			/* class */				/* Object Part */
   {												/* Class Part */
	sizeof (IdrisDirRec),		/* instance size */
	"IdrisDir",					/* class name */
	&crObject,				/* super */
	NIL,					/* method dict */
	objInit,				/* initialize */
	dirNew,					/* new object */
	objClone,				/* clone self */
	dirKill,				/* kill self */
	dirOpen,				/* open */
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
   	0
   },
};

Class clIdrisDir   = (Class)&crIdrisDir;

