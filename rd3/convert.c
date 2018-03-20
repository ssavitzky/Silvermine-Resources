/***/static char *moduleID="convert 1.0 (c)1988 S. Savitzky";/***/

/*********************************************************************\
**
**	F I L E   C O N V E R S I O N
**
**		Here is where we do all conversion of X disk representations
**		to in-core and PC formats.
**
**		Contains:
**			various utilities
**			dCopyXyz	the various file-translation routines
**			dCopyFile	dispatch on a file type and copy
**			dCopyDirs	copy all tagged files in a directory
**			dNextChar	used by conversion routines derived from RD7
**
**	880509	SS	create
**
\*********************************************************************/

#include "coops.h"
#include "tree.h"
#include "dir.h"
#include "petos.h"
#include "../lib/curse.h"
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/utime.h>
#include <time.h>
#include <sys/stat.h>
#include <dos.h>

#include "disk.h"

#undef  global
#define global
#include "convert.h"

#define CHUNK 8192			/* size of chunk to write */

#ifdef V_SP

#include <../as/std.h>
#include <../as/spec.h>
#include "as/okrec.h"
static KREC skrec;
short *oldnew(char *, KREC *,char *);
void getdate(Dir, char *);
float dummyf;
#endif


extern void errorSet(), errorClear();
extern int errorCheck();
extern int errno;
extern char *dFileExt;

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
static char date[8];

/*********************************************************************\
**
** Destination Directory updating
**
\*********************************************************************/


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
** Pre-write drive validation:
**
** Boolean dValidateDest()
**
**		Return TRUE if the output drive and path are still valid.
**		Currently only called if dOutputDir does not exist, i.e.
**		if the output drive has not been read.
**
\*********************************************************************/

global unsigned dValidateDest()
{
	struct diskfree_t diskspace;
	int drive = dDriveNum(dOutputPath) + 1;

	errorClear();
	/*
	** Make sure drive can still be read
	** Notice the kludgery we have to go through because
	** 		a:/ is valid		a:	isn't
	** but  ./ isn't valid		.	is
	*/
	if (_dos_getdiskfree(drive, &diskspace))
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
	if (! dDrives[drive]) {
		dOutputDir = (Dir) NIL;
	}
	return (TRUE);
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
	if (dOutputDir) {
		outDir = dOutputDir;
		reread = 0;
	} else {
		reread = 1;
	}
}

static void dFinishWrite()
{
	if (dOutputDir && reread) {
		gOpen(dOutputDir);
	}
}

/*********************************************************************\
**
** NOTE:  The following stuff is needed only if no Dir node exists
**		  for the current output path.	Eventually this will be fixed.
**
** FILE *cvOpenDosFile(name, ext, mode)
**
**		Opens a DOS file with filename as close to name as possible,
**		in the given (output) mode.  An optional new extension can 
**		be supplied.
**
**		Sets outPath to outPrefix + name.
**		cache FILE and name for later call to cvCloseDosFile
**
** cvCloseDosFile()
**
**		Make sure a Dir node exists under outDir for the
**		most-recently-opened Dos file.
**		Update its size and date.
**
\*********************************************************************/

static FILE *dosFile;
static char *dosFileName;

FILE *cvOpenDosFile(name, ext, mode)
	char *name, *ext, *mode;
{
	int  n;
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

void cvCloseDosFile()
{
	Dir dp; 
	struct stat stat_buf;

	if (outDir)	{
		fflush(dosFile);
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
** cfOpenDosDir(name)
**
**		Opens a DOS directory as close to name as possible,
**		creating it if necessary.
**
**		appends name\ to outputPrefix
**
\*********************************************************************/

static void cfOpenDosDir(name)
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

	if (n -> dir.time == 0 || n -> dir.time == -1) return;
	times.actime = times.modtime = n -> dir.time;
	utime(outPath, &times);
}

/*********************************************************************\
**
** cvInitRead(d, mode)
**
**		Set up to read from d.  Reads the first chunk and returns
**		its address.  Should be big enough to hold a KREC for spectra.
**
** dNextChar()
**
**		Read next character from current input file (dInFile)
**		Initialize by setting bufsize = 0; dInFile -> Dir node.
**
** dSeekChar(n)
**
**		seek to the nth character of the buffer.
**
\*********************************************************************/

Dir dInFile;
static char buf[CHUNK];
static int  bufsize = 0;
static int  bufp = 0;
static long buforg;

char *cvInitRead(d, mode)
	Dir d;
	char *mode;
{
	gOpenFile(d, mode);
	dInFile = d;
 	gSeek(dInFile, 0L);
	bufsize = gReadFile(dInFile, buf, CHUNK);
	bufp = 0;
	buforg = 0L;
	return (buf);
}

int dNextChar()
{
	if (bufp >= bufsize) {
		buforg += bufsize;
		bufsize = gReadFile(dInFile, buf, CHUNK);
		bufp = 0;
	}
	if (bufsize == 0) return (-1);
	return (buf[bufp++]);
}

void dSeekChar(n)
	long n;
{
	if (n < buforg)	{
		buforg = n - (n % CHUNK);
		gSeek(dInFile, buforg);
		bufsize = gReadFile(dInFile, buf, CHUNK);
	}
	bufp = n - buforg;
}


/*********************************************************************\
**
** cvCopy(s, imode, d, omode)
**
**		Open node s and copy it to a new file in
**		the current output directory with mode omode.
**		The parameter d is the output Dir node, if present.
**		Returns 0 if unsuccessful, 1 if successful.
**
\*********************************************************************/

static int cvCopy(s, imode, d, omode)
	Dir s, d;
	char *imode, *omode;
{
	ulong len, l;
	Dir f;
	FILE *ff;

	if (dEachNewDir) (*dEachNewDir)(s);
	if (d) {
		f = gCreateFile(d, gName(s), FALSE);
		strcpy(outPath, gPath(f, NIL));
		if (ISNULL(f))  {
			errorPrintf("unable to create output file for '%s'", gPath(s, NIL));
			return(FALSE);
		}
		if (!gOpenFile(f, omode)) {
			errorPrintf("unable to open '%s' for output", gPath(f, NIL));
			return(FALSE);
		}
		move(24, 40); addstr("  -> "); addstr(gPath(f, NIL)); refresh();

		len = s -> dir.size;
		gOpenFile(s, imode);
	 	gSeek(dInFile, 0L);
		for (l = 1; l && len; len -= l) {
			l = gReadFile(s, buf, CHUNK);
			if (l && l > gWriteFile(f, buf, l)) {
				errorPrintf("error: %s writing %s", 
							strerror(errno),  s -> dir.name);
				gCloseFile(s);	/* close the input file */
				gCloseFile(f);	/* close the output file */
				return(FALSE);
			}
		}
		if (s -> dir.time != 0 && s -> dir.time != -1) 
			f -> dir.time = s -> dir.time;
		gCloseFile(s);	/* close the input file */
		gCloseFile(f);	/* close the output file */
	} else {
		ff = cvOpenDosFile(s -> dir.name, (char *) NULL, omode);
		if (!ff) {
			errorPrintf("unable to open '%s' for output", outPath);
			return(FALSE);
		}
		move(24, 40); addstr("  -> "); addstr(outPath); refresh();

		len = s -> dir.size;
		gOpenFile(s, "rb");
	 	gSeek(dInFile, 0L);
		for (l = 1; l && len; len -= l) {
			l = gReadFile(s, buf, CHUNK);
			if (l && l > fwrite(buf, 1, l, ff)) {
				errorPrintf("error: %s writing %s", 
							strerror(errno),  d -> dir.name);
				fclose(ff);
				gCloseFile(s);	/* close the input file */
				return(FALSE);
			}
		}
		gCloseFile(s);	/* close the input file */
		cvCloseDosFile();
		fixtime(s);
	}
	return(TRUE);
}


/*********************************************************************\
**
** cvCopyInit(s, imode, d, ext, omode)
**
**		Like dCopy but merely opens the files, calling cvInitRead
**		to start reading s, and returning a FILE* for writing d.
**		d must be a DOS file.
**
**		ext, if non-null, is the filename extension for d.
**
** cvCopyClose()
**
**		cleans up after a copy started with dCopyInit.
**
\*********************************************************************/

static Dir cvS, cvD;
static FILE *cvDF;

static FILE *cvCopyInit(s, imode, d, ext, omode)
	Dir s, d;
	char *imode, *ext, *omode;
{
	cvS = s;
	if (dEachNewDir) (*dEachNewDir)(s);
	cvInitRead(s, imode);
	if (d) {
		dFileExt = ext;
		cvD = gCreateFile(d, gName(s), FALSE);
		strcpy(outPath, gPath(cvD, NIL));
		if (ISNULL(cvD))  {
			errorPrintf("unable to create output file for '%s'", gPath(s, NIL));
			return(FALSE);
		}
		if (!gOpenFile(cvD, omode)) {
			errorPrintf("unable to open '%s' for output", gPath(cvD, NIL));
			return(FALSE);
		}
		cvDF = fdopen(cvD -> dir.file, omode);
	} else {
		cvD = NIL;
		cvDF = cvOpenDosFile(s -> dir.name, ext, omode);
		if (!cvDF) {
			errorPrintf("unable to open '%s' for output", outPath);
			return(FALSE);
		}
	}
	move(24, 40); addstr("  -> "); addstr(outPath); refresh();
	return(cvDF);
}

void cvCopyClose()
{
	if (cvD) {
		if (cvS -> dir.time != 0 && cvS -> dir.time != -1) 
			cvD -> dir.time = cvS -> dir.time;
		gCloseFile(cvS);	/* close the input File */
		fclose(cvDF);		/* close the output stream */
		gCloseFile(cvD);	/* close the output File */
	} else {
		gCloseFile(cvS);	/* close the input File */
		cvCloseDosFile();	/* close the output stream */
		fixtime(cvS);
	}
}


/*********************************************************************\
**
** cvCopyJcamp(s, d)
** cvCopySpectrum(s, d)
**
**		if d is null, write an ordinary DOS file whose name
**		is derived from that of s; otherwise create a new Dir node
**		in directory d.
**
\*********************************************************************/

#ifdef V_SP
extern char *dJcampHdr;
extern int graph3sp();

void cvViewSpectrum(s)
	Dir s;
{
	FILE *f;
	
	cvInitRead(s, "rb");
	getdate(s,date);
	oldnew(buf, &skrec,date);
	dSeekChar((long)sizeof(OKREC));
	graph3sp(gName(s), &skrec);
	gCloseFile(s);
}

static int cvCopyJcamp(s, d)
	Dir s, d;
{
	FILE *f;
	FILE *j;
	char title[16];

	if (!dJcampHdr || !*dJcampHdr) {
		errorPrintf("No JCAMP header file specified");
		return(FALSE);
	}
	j = fopen(dJcampHdr, "r");
	if (!j) {
		errorPrintf("Cannot open JCAMP header file");
		return(FALSE);
	}

	f = cvCopyInit(s, "rb", d, "DX", "wt");
	if (!f) return(FALSE);

	getdate(s,date);
	oldnew(buf, &skrec,date);
	dSeekChar((long)sizeof(OKREC));
	jcamp3(s -> dir.name, f, j, &skrec);

	fclose(j);
	cvCopyClose();
	return(TRUE);
}


static int cvCopyCss(s, d)
	Dir s, d;
{
	FILE *f;

	f = cvCopyInit(s, "rb", d, "SP", "wb");
	if (!f) return(FALSE);

	getdate(s,date);
	oldnew(buf, &skrec,date);
	dSeekChar((long)sizeof(OKREC));

	uv3do(f, &skrec, s -> dir.size);

	cvCopyClose();
	return(TRUE);
}

void dm3do(FILE *, KREC far *, LONG);
static int cvCopySpectrum(s, d)
	Dir s, d;
{
	FILE *f;

	f = cvCopyInit(s, "rb", d, "SP", "wb");
	if (!f) return(FALSE);

	getdate(s,date);
	oldnew(buf, &skrec,date);
	dSeekChar((long)sizeof(OKREC));

	dm3do(f, &skrec, s -> dir.size);

	cvCopyClose();
	return(TRUE);
}

#endif /* V_SP */

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
		dWriteStart(n);
	}

	switch (n -> dir.ftype) {
	 case ascii:
		ok = cvCopy(n, "rt", outDir, "wt");
		break;
	 case binary:
		ok = cvCopy(n, "rb", outDir, "wb");
		break;
#ifdef V_SP
	 case css:
	 	ok = cvCopyCss(n, outDir);
		break;
	 case jcamp:
		ok = cvCopyJcamp(n, outDir);
		break;
	 case spectrum:
		ok = cvCopySpectrum(n, outDir);
		break;
#endif
	 default:
		ok = cvCopy(n, "rb", outDir, "wb");
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
		dWriteStart(n);
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
		} else if (outDir && p -> dir.isDir) {
			/* === can eliminate outPrefix eventually === */
			/* === provided we always have an outDir  === */
			dd = outDir;					/* save dest. Dir node */
			d = strlen(outPrefix);			/* save prefix length */
			if (ISNULL(outDir = gCreateFile(outDir, gName(p), TRUE)))
				break;
			strcat(outPrefix, gName(outDir));
			if (outPrefix[strlen(outPrefix) - 1] != '\\')
				strcat(outPrefix, "\\");
			ok = dCopyDirs (p, FALSE);		/* copy the contents */
			outPrefix[d] = 0;				/* truncate prefix */
			outDir = dd;					/* restore outDir */
		} else if (p -> dir.isDir) {
			d = strlen(outPrefix);			/* save prefix length */
			cfOpenDosDir(p -> dir.name);	/* tack on dir. name */
			ok = dCopyDirs (p, FALSE);		/* copy the contents */
			outPrefix[d] = 0;				/* truncate prefix */
		} else {
			ok = dCopyFile(p, FALSE);
		}
	}

	if (init) {
		dFinishWrite();
	}
	return(ok);
}

void getdate(Dir s, char *date)
{
	struct tm *tm;
	XDir x;
	int i;

	i = (int) s -> dir.time;
	if (i > 0) {
		tm = gmtime((time_t*) &s -> dir.time);
		sprintf(date,"%02d/%02d/%02d",
			tm -> tm_year, tm -> tm_mon + 1, tm -> tm_mday);
	} else if (fClass(s) == clXDir) {
		x = (XDir) s;
		sprintf(date, "  /  /  ");
		strncpy(date, x -> x_dir.mdate, 2);
		strncpy(date + 3, x -> x_dir.mdate + 2, 2);
		strncpy(date + 6, x -> x_dir.mdate + 4, 2);
	} else
		strnset(date,' ',8);
}


