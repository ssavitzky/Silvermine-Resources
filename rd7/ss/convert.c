/***/static char *moduleID="convert 1.0 (c)1988 S. Savitzky";/***/

/*********************************************************************\
**
**	F I L E   C O N V E R S I O N
**
**		Here is where we do all conversion of Idris representations
**		to in-core and PC formats.
**
**		Contains:
**			various utilities
**			dCopyXyz	the various file-translation routines
**			dCopyFile	dispatch on a file type and copy
**			dCopyDirs	copy all tagged files in a directory
**
**	880509	SS	create
**
\*********************************************************************/

#include "rd7.h"
#include "coops.h"
#include "trees.h"
#include "dirs.h"
#include "view.h"
#include "../lib/curse.h"
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/utime.h>
#include <sys/stat.h>
#include <dos.h>

#include "disk.h"

#undef  global
#define global
#include "convert.h"

#define CHUNK 1024			/* size of chunk to write */

extern void errorSet(), errorClear();
extern int errorCheck();
extern int errno;

/*********************************************************************\
**
** Output Path
**
**		outPrefix is the current output directory path prefix;
**		outPath is the pathname of the file currently being written.
**
\*********************************************************************/

static char outPrefix[256];
static char outPath[256];


/*********************************************************************\
**
** Destination Directory updating
**
\*********************************************************************/


static Dir destDir;		/* destination directory to re-read */
static Dir outDir;		/* directory we're reading in now */

static Bool reread = 1;	/* directory is bogus -- re-read when done */


/*********************************************************************\
**
** Byte-swapping macros
**
**		SWAP_S(w)		swap a short
**		SWAP_L(w)		swap a long
**
**	For arrays of shorts, use swab(src, dst, nbytes)
**
\*********************************************************************/

#define SWAP_S(w)		(((w) & 0xFF) << 8) | (((w) >> 8) & 0xFF))
#define SWAP_L(w)		( (SWAP_S((w) & 0xffffL) << 16L)	| \
						  (SWAP_S(((w) >> 16L) & 0xffffL)) )

/*********************************************************************\
**
** Filename Mapping
**
\*********************************************************************/

/*
** Map lowercase to uppercase, [] to (), everything else to ~.
*/

static char fixTbl[128] = {
/*	   0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F */
/*0*/  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
/*1*/  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
/*2*/  0, '!', 0, '#','$','%','&', 39,'(',')', 0,  0,  0, '-', 0,  0, 
/*3*/ '0','1','2','3','4','5','6','7','8','9', 0,  0,  0,  0,  0,  0, 
/*4*/ '@','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O',
/*5*/ 'P','Q','R','S','T','U','V','W','X','Y','Z','(', 0, ')','^','_',
/*6*/ '`','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O',
/*7*/ 'P','Q','R','S','T','U','V','W','X','Y','Z','{', 0, '}','~', 0, 
};

/*
** fixcpy(dst, src, len)
**
**		copy at most len bytes of src to dst,
**		fixing characters in the process.
*/
static void fixcpy(dst, src, len)
	char *dst;
	char *src;
	int   len;
{
	for ( ; *src && len; ++dst, ++src, --len) 
		*dst = fixTbl[*src] ? fixTbl[*src] : '~';
	*dst = 0;
}

/*
** shorten(dst, dstlen, src, srclen)
**
**		shorten a string by truncating at the right,
**		uppercasing and fixing characters in the process.
*/
static void shorten(dst, dstlen, src, srclen)
	char *dst;
	int   dstlen;
	char *src;
	int   srclen;
{
	/*
	** First see if it fits.
	**		if it does, copy, substituting characters as needed
	**		if not, truncate it
	*/
	if (srclen <= dstlen) {
		fixcpy(dst, src, srclen);
	} else {
		fixcpy(dst, src, dstlen);
	}
}

/*
** appendCount (dst, clobber)
*/
void appendCount (dst, clobber)
	char *dst;
	int   clobber;
{
	int len = strlen(dst);
	char nn[6];

	sprintf(nn, "%d", clobber);
	if (strlen(dst) + strlen(nn) <= 8)
		strcat(dst, nn);
	else {
		strcpy(dst + 8 - strlen(nn), nn);
	}
}
/*
** dFixName (dst, src, ext, clobber)
**
**		Fix a filename, putting it in MS-DOS format.
**		If clobber > 0, it is a count of the number of conflicting
**		files found so far.  It is appended to the name.
*/
void dFixName(dst, src, ext, clobber)
	char *dst;
	char *src;
	char *ext;
	int clobber;
{
	char *extDot = strrchr(src, '.');

	if (extDot) {
		shorten(dst, 8, src, extDot - src);
		if (clobber) appendCount(dst, clobber);
		strcat(dst, ".");
		if (ext != NULL)
			strcat(dst, ext);
		else
			shorten(dst + strlen(dst), 3, extDot + 1, strlen(extDot + 1));
	} else {
		shorten(dst, 8, src, strlen(src));
		if (ext != NULL) strcat(dst, ext);
		if (clobber) appendCount(dst, clobber);
	}
}


/*********************************************************************\
**
** Pre-write drive validation:
**
** Boolean dValidateDest()
** Boolean dValidateDos(dir)
** Boolean dValidateIdris(dir)
**
**		Return TRUE if the drive has been read and belongs to the
**		correct operating system.
**
\*********************************************************************/

global unsigned dValidateDest()
{
	struct diskfree_t diskspace;

	errorClear();
	if (dOutputDrive == 0) {
		destDir = (Dir) NIL;
		return (FALSE);
	}
	/*
	** Make sure drive can still be read
	** Notice the kludgery we have to go through because
	** 		a:/ is valid		a:	isn't
	** but  ./ isn't valid		.	is
	*/
	if (_dos_getdiskfree(dOutputDrive, &diskspace))
		if (errorCheck()) return (FALSE);
		else {
			errorPrintf("Output drive invalid or inaccessible");
			return (FALSE);
		}
	if (errorCheck()) return (FALSE);
	if (strlen(dOutputPath) == 2 && dOutputPath[1] == ':') {
		char fake[4];
		strcpy(fake, " :/");
		fake[0] = dOutputPath[0];
		if (access(fake, 0))
			if (errorCheck()) return (FALSE);
			else {
				errorPrintf("Output drive invalid or inaccessible");
				return (FALSE);
			}
		else if (errorCheck()) return (FALSE);
	} else {
		if (access(dOutputPath, 0))
			if (errorCheck()) return (FALSE);
			else {
				errorPrintf("Output drive invalid or inaccessible");
				return (FALSE);
			}
		else if (errorCheck()) return (FALSE);
	}
	if (dDrives[dDriveNum(dOutputPath)]) {
		destDir = dOutputDir;
	} else {
		destDir = (Dir) NIL;
	}
	return (TRUE);
}

global unsigned dValidateDos(d)
	Dir d;
{
	struct stat buf;
	char path[255];
	struct diskfree_t diskspace;

	errorClear();
	if (ISNULL(d)) return (FALSE);
	GetPath(d, path);
	if (_dos_getdiskfree(dDriveNum(path) + 1, &diskspace)) 
		if (errorCheck()) return (FALSE);
		else {
			errorPrintf("DOS input drive invalid or inaccessible");
			return (FALSE);
		}
	if (errorCheck()) return (FALSE);
	if (strlen(path) == 2 && path[1] == ':') strcat(path, "\\");
	if (stat(path, &buf))
		if (errorCheck()) return (FALSE);
		else {
			errorPrintf("DOS input drive invalid or inaccessible");
			return (FALSE);
		}
	if (errorCheck()) return (FALSE);
	if ((buf.st_mode & S_IFDIR))
		return (d -> dir.isDir != 0);
	else
		return (d -> dir.isDir == 0);
}

global unsigned dValidateIdris(d)
	Dir d;
{
	char c;

	if (ISNULL(d)) return (FALSE);
	return ('/' == *gPath(d, NIL));		/* gotta name an Idris drive! */
}


/*********************************************************************\
**
** dStartWrite()		initialize directory and path info
** dFinishWrite()		re-read directory if necessary.
**
\*********************************************************************/

static void dStartWrite()
{
	strcpy(outPrefix, dOutputPath);
	if (outPrefix[strlen(outPrefix - 1)] != '\\')
		strcat(outPrefix, "\\");
	if (destDir) {
		outDir = destDir;
		reread = 0;
	} else {
		reread = 1;
	}
}

static void dFinishWrite()
{
	if (destDir && reread) {
		dReadDosDir(destDir);
	}
}


/*********************************************************************\
**
** FILE *dOpenDosFile(name, ext, mode)
**
**		Opens a DOS file with filename as close to name as possible,
**		in the given (output) mode.  An optional new extension can 
**		be supplied.
**
**		Sets outPath to outPrefix + name.
**		cache FILE and name for later call to dCloseDosFile
**
** dCloseDosFile()
**
**		Make sure a Dir node exists under outDir for the
**		most-recently-opened Dos file.
**		Update its size and date.
**
\*********************************************************************/

static FILE *dosFile;
static char *dosFileName;

FILE *dOpenDosFile(name, ext, mode)
	char *name, *ext, *mode;
{
	int  n;
	static char dosname[32];
	FILE *f;

	/* 
	** truncate name intelligently 
	*/
	strcpy(outPath, outPrefix);
	dosFileName = outPath + strlen(outPath);
	dFixName(dosFileName, name, ext, 0);

	/* 
	** Check for file already existing
	** === should ask (skip, write, rename)
	** === but instead we just append a number.
	*/
	for (n = 1; n < 1000 && !access(outPath, 0); ++n) {
		strcpy(outPath, outPrefix);
		dFixName(outPath + strlen(outPath), name, ext, n);
	}

	dosFile = f = fopen(outPath, mode);
	if (!f) {
		errorPrintf("Cannot open %s file %s", 
					*mode == 'w'? "output" : "input",
					outPath);
	}
	return (f);
}

void dCloseDosFile()
{
	Dir dp; 
	struct stat stat_buf;

	if (outDir)	{
		fstat(fileno(dosFile), &stat_buf);
		fclose(dosFile);
		dp = (Dir) cfNew(clDir)(clDir, outDir, dosFileName, 
								0, stat_buf.st_size);
		if (dp)
			dp -> dir.time = stat_buf.st_mtime;	
		else {
			errorPrintf(
"Out of memory closing \"%s\"; tree will be incomplete",
						dosFileName);
		}
	} else {
		fclose(dosFile);
	}
}

/*********************************************************************\
**
** Dir dNewDir(parent, name)
**
**		Create a new directory under the given parent
**
\*********************************************************************/

Dir dNewDir(d, name)
	Dir d;
	char *name;
{
	char fn[256];
	char *n;

	GetPath(d, fn);
	if (fn[strlen(fn) - 1] != '\\')	strcat(fn, "\\");
	n = fn + strlen(fn);
	dFixName(n, name, (char *)NULL, 0);

	/* 
	** Check for dir already existing
	** create if it doesn't exist.
	*/
	if (mkdir(fn)) {
		errorPrintf("Unable to create %s", fn);
		return ((Dir) NIL);
	}
	
	/* === ought to complain if ordinary file === */

	/*
	** Create a directory node if necessary.
	*/
	if (d) {
		d = (Dir) cfNew(clDir)(clDir, d, n, 1, 0L);
		if (!d) {
			errorPrintf(
"Out of memory creating \"%s\"; tree will be incomplete.",
						n);
		}
	}
	return(d);
}
	
/*********************************************************************\
**
** dOpenDosDir(name)
**
**		Opens a DOS directory as close to name as possible,
**		creating it if necessary.
**
**		appends name\ to outputPrefix
**
\*********************************************************************/

void dOpenDosDir(name)
	char *name;
{
	Dir dp;
	char *n;

	/* 
	** truncate name intelligently 
	*/
	n = outPrefix + strlen(outPrefix);
	dFixName(n, name, (char *)NULL, 0);

	/* 
	** Check for dir already existing
	** create if it doesn't exist.
	*/
	mkdir(outPrefix);
	
	/* === ought to complain if ordinary file === */

	/*
	** Create a directory node if necessary, and enter it.
	*/
	if (outDir) {
		dp = (Dir) cfNew(clDir)(clDir, outDir, n, 1, 0L);
		outDir = dp;
		if (!dp) {
			errorPrintf(
"Out of memory creating \"%s\"; tree will be incomplete",
						n);
		}
	}

	if (outPrefix[strlen(outPrefix) - 1] != '\\')
		strcat(outPrefix, "\\");
}

/*********************************************************************\
**
** fixtime(n)
**
**		Change the time on the DOS file corresponding to node n
**		to correspond to the modify time in n.
**
**		n must be the last file written.
**
\*********************************************************************/

static void fixtime(n)
	Dir n;
{
	struct utimbuf times;

	times.actime = times.modtime = n -> dir.time;
	utime(outPath, &times);
}

/*********************************************************************\
**
** dCopyAscii(n)
** dCopyBinary(n)
** dCopyJcamp(n)
** dCopySpectrum(n)
**
**		These all take a Dir node as their sole argument, and open
**		a destination DOS file in the current directory in the
**		appropriate mode.
**
**		They return 0 if unsuccessful, 1 if successful.
**
\*********************************************************************/

static int dCopyAscii(n)
	Dir n;
{
	register FILE *f;
	register int c;
	struct utimbuf times;
	char huge *buf, huge *p;
	ulong len;

	if (dEachNewDir) (*dEachNewDir)(n);
	buf = dReadIdrisFile(n);
	if (!buf) {
		errorPrintf("unable to read '%s'", gPath(n, NIL));
		return(FALSE);
	}

	f = dOpenDosFile(n -> dir.name, (char *) NULL, "wt");
	if (!f) {
		errorPrintf("unable to open '%s' for output", outPath);
		BufFree(buf);
		return(FALSE);
	}
	move(24, 40); addstr("  -> "); addstr(outPath); refresh();

	len = n -> dir.size;
	for (p = buf; len >= CHUNK; len -= CHUNK, p += CHUNK) {
		if (p >= buf + STATICBUFMAX) {
			p = buf = dReadIdrisBuffer(n, buf);
		}
		if (CHUNK > fwrite(p, 1, CHUNK, f)) {
full:		errorPrintf("error: %s writing %s", 
						strerror(errno),  n -> dir.name);
			fclose(f);
			BufFree(buf);
			return(FALSE);
		}
	}
	if (len > 0 && len > fwrite(p, 1, len, f)) goto full;

	dCloseDosFile();
	BufFree(buf);
	fixtime(n);
	return(TRUE);
}


static int dCopyBinary(n)
	Dir n;
{
	register FILE *f;
	register int c;
	char *ext = (char *)NULL;
	char huge *buf, huge *p;
	ulong len;
	extern int rd7sp;

	if (dEachNewDir) (*dEachNewDir)(n);
	buf = dReadIdrisFile(n);
	if (!buf) {
		errorPrintf("unable to read '%s'", gPath(n, NIL));
		return(FALSE);
	}
	move(24, 40); addstr("  -> "); addstr(outPath); refresh();

	/* Convert extension from .sp to .SPI if necessary */
	if (rd7sp && !strcmp(".sp", n -> dir.name + strlen(n -> dir.name) - 3))
		ext = "SPI";
	f = dOpenDosFile(n -> dir.name, ext, "wb");		/* binary mode */
	if (!f) {
		errorPrintf("unable to open '%s' for output", outPath);
		BufFree(buf);
		return(FALSE);
	}

	len = n -> dir.size;
	for (p = buf; len >= CHUNK; len -= CHUNK, p += CHUNK) {
		if (p >= buf + STATICBUFMAX) {
			p = buf = dReadIdrisBuffer(n, buf);
		}
		if (CHUNK != fwrite(p, 1, CHUNK, f)) {
full:		errorPrintf("error: %s writing %s", 
						strerror(errno), n -> dir.name);
			fclose(f);
			BufFree(buf);
			return(FALSE);
		}
	}
	if (len > 0 && len > fwrite(p, 1, len, f)) goto full;

	dCloseDosFile();
	BufFree(buf);
	fixtime(n);
	return(TRUE);
}


static int dCopyJcamp(n)
	Dir n;
{
	FILE *f;
	FILE *j;
	char huge *buf;

	if (!dJcampHdr || !*dJcampHdr) {
		errorPrintf("No JCAMP header file specified");
		return(FALSE);
	}

	if (dEachNewDir) (*dEachNewDir)(n);
	buf = dReadIdrisFile(n);
	if (!buf) return(FALSE);

	f = dOpenDosFile(n -> dir.name, "DX", "wt");		/* text mode */
	if (!f) {
		BufFree(buf);
		return(FALSE);
	}
	move(24, 40); addstr("  -> "); addstr(outPath); refresh();

	j = fopen(dJcampHdr, "r");
	if (!j) {
		fclose(f);
		BufFree(buf);
		return(FALSE);
	}

	jcampdo(n -> dir.name, f, j, buf, n -> dir.size);

	fclose(j);
	dCloseDosFile();
	BufFree(buf);
	fixtime(n);
	return(TRUE);
}


static int dCopySpectrum(n)
	Dir n;
{
	FILE *f;
	char huge *buf;

	if (dEachNewDir) (*dEachNewDir)(n);
	buf = dReadIdrisFile(n);
	if (!buf) return(FALSE);

	f = dOpenDosFile(n -> dir.name, "SP", "wb");		/* binary mode */
	if (!f) {
		BufFree(buf);
		return(FALSE);
	}
	move(24, 40); addstr("  -> "); addstr(outPath); refresh();

	dmdo(f, fileno(f), buf, n -> dir.size);

	dCloseDosFile();
	BufFree(buf);
	fixtime(n);
	return(TRUE);
}


static int dCopyCss(n)
	Dir n;
{
	FILE *f;
	char huge *buf;

	if (dEachNewDir) (*dEachNewDir)(n);
	buf = dReadIdrisFile(n);
	if (!buf) return(FALSE);

	f = dOpenDosFile(n -> dir.name, "SP", "wb");		/* binary mode */
	if (!f) {
		BufFree(buf);
		return(FALSE);
	}
	move(24, 40); addstr("  -> "); addstr(outPath); refresh();

	cuvdo(f, fileno(f), buf, n -> dir.size);

	dCloseDosFile();
	BufFree(buf);
	fixtime(n);
	return(TRUE);
}


/*********************************************************************\
**
** dCopyFile(n, init)			copy a single file (tagged or not)
**
**		if 'init' is TRUE, initialize the drive before copying,
**		and close it afterward.
**
\*********************************************************************/

global int dCopyFile(n, init)
	Dir n;
	int	init;
{
	int ok;

	if (init) {
		dStartWrite();
		dWriteStart(oDirView, n);
	}

	switch (n -> dir.mode) {
	 case unknown:
	 case ascii:	ok = dCopyAscii(n); 	break;
	 case binary:	ok = dCopyBinary(n);	break;
	 case jcamp:	ok = dCopyJcamp(n);		break;
	 case css:		ok = dCopyCss(n);		break;
	 case spectrum:	ok = dCopySpectrum(n);	break;
	}				
	if (init) {
		dFinishWrite();
	}
	return(ok);
}


/*********************************************************************\
**
** dCopyDirs(n, init)				copy tagged files under n
**
**		if 'init' is TRUE, initialize the drive before copying,
**		and close it afterward.
**
\*********************************************************************/

global int dCopyDirs(n, init)
	Dir n;
	int	init;
{
	Dir p, dd;
	int d;
	int ok = TRUE;

	if (init) {
		dStartWrite();
		dWriteStart(oDirView, n);
	}

	/*
	** Copy ordinary files
	** For each non-empty subdirectory,
	**		make an MS/DOS directory for it
	**		cd to it
	**		and copy the contents.
	*/
	for (p = (Dir) gDown(n); p && ok; p = (Dir) gNext(p)) {
		if (kbhit() && getch() == 0x1b) break;	/* ESC to abort */
		if (p -> dir.tcount == 0) {
			continue;
		} else if (p -> dir.isDir) {
			dd = outDir;					/* save dest. Dir node */
			d = strlen(outPrefix);			/* save prefix length */
			dOpenDosDir(p -> dir.name);		/* tack on dir. name */
			ok = dCopyDirs (p, FALSE);		/* copy the contents */
			outPrefix[d] = 0;				/* truncate prefix */
			outDir = dd;					/* restore destDir */
		} else {
			ok = dCopyFile(p, FALSE);
		}
	}

	if (init) {
		dFinishWrite();
	}
	return(ok);
}

/*********************************************************************\
**
** dCopyDosFile(n, init)			copy a single file (tagged or not)
**
**		if 'init' is TRUE, this is a top-level call
**
\*********************************************************************/

global int dCopyDosFile(n, init)
	Dir n;
	int	init;
{
	register FILE *f, *g;
	register int sz;
	static char buf[1024];
	int ok = TRUE;

	if (init) {
		dStartWrite();
		dWriteStart(oDosView, n);
	}

	if (dEachNewDir) (*dEachNewDir)(n);
	f = dOpenDosFile(n -> dir.name, (char *)NULL, "wb");
	if (!f) {
		errorPrintf("Cannot open output file '%s'", outPath);
		ok = FALSE;
		goto fail1;
	}
	move(24, 40); addstr("  -> "); addstr(outPath); refresh();

	g = fopen(gPath(n, NIL), "rb");
	if (!f) {
		errorPrintf("Cannot open input file '%s'\n", n -> dir.name);
		fclose(f);
		ok = FALSE;
		goto fail1;
	}

	for ( ; (sz = read(fileno(g), buf, sizeof(buf))); ) 
		if (sz > write(fileno(f), buf, sz)) {
			errorPrintf("error: %s writing '%s'", 
						strerror(errno), outPath);
			ok = FALSE;
			break;
		}

	fclose(g);
	dCloseDosFile();
	fixtime(n);

fail1:
	if (init) {
		dFinishWrite();
	}
	return(ok);
}


/*********************************************************************\
**
** dCopyDosDirs(n, init)				copy tagged files under n
**
**		if 'init' is TRUE this is a top-level call.
**
\*********************************************************************/

global int dCopyDosDirs(n, init)
	Dir n;
	int	init;
{
	Dir p, dd;
	int d;
	int ok = TRUE;

	if (init) {
		dStartWrite();
		dWriteStart(oDosView, n);
	}

	/*
	** Copy ordinary files
	** For each non-empty subdirectory,
	**		make an MS/DOS directory for it
	**		cd to it
	**		and copy the contents.
	*/
	for (p = (Dir) gDown(n); p && ok; p = (Dir) gNext(p)) {
		if (kbhit() && getch() == 0x1b) break;	/* ESC to abort */
		if (p -> dir.tcount == 0) {
			ok = TRUE;
			continue;
		} else if (p -> dir.isDir) {
			dd = outDir;					/* save dest. Dir node */
			d = strlen(outPrefix);			/* save prefix length */
			dOpenDosDir(p -> dir.name);		/* tack on dir. name */
			ok = dCopyDosDirs (p, FALSE);	/* copy the contents */
			outPrefix[d] = 0;				/* restore prefix */
			outDir = dd;					/* restore destDir */
		} else {
			ok = dCopyDosFile(p, FALSE);
		}
	}

	if (init) {
		dFinishWrite();
	}
	return(ok);
}


