/*********************************************************************\
**
** SHIPPER		RDi disk duplication program
**
**		This program reads a file of serial numbers and customer names
**		and creates a disk containing a customized copy of RD7 for
**		each one.
**
**		Specifically, the input file contains multi-line entries
**		of the form:
**
**			line  1		serial number
**			line  2		customer name
**			lines 3-n	ignored
**			blank line	separates entries.
**
**		This way, the input file can contain mailing list and other 
**		information.  Lines 1 can contain other information
**		separated by commas; everything from the first comma to the
**		end of the line is skipped.
**
**		COMMAND-LINE OPTIONS:                                   (Default):
**            +-----------note required space
**			  V
**			-d dir		directory containing product to ship 	(RD7)
**			-p file		product to customize					(RD7.EXE)
**			-o drive	drive to output to						(A:)
**			file		input file name
**
**		The serial-number, customer-name, and Xor strings are
**		taken from #define's in the include file "rd7.h".
**
**		RESTRICTIONS:
**
**		The product name must be uppercase because the match algorithm
**		is stupid.
**
** 	880530 SS create
**	880822 SS speed up by sucking the whole disk image into memory
**
\*********************************************************************/

static char *pgmid="shipper 1.0\
Copyright (c) 1988 Silvermine Resources, Inc.";

#include <stdio.h>
#include <memory.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <malloc.h>
#include <dos.h>
#include "version.h"


/*********************************************************************\
**
** Variables
**
\*********************************************************************/

#define MAXFNAME 256			/* size of a pathname */
#define MAXLINE 1024			/* size of an input file line */
#define BUFSIZE 4096			/* how much to read in one gulp */
#define MAXFILES 100			/* # of files on the disk */


typedef struct filedsc {
	char name [MAXFNAME];
	long len;
	char huge *buf;
} FileDsc;

FileDsc files[MAXFILES];		/* The file names and contents */
int nfiles = 0;					/* How many there are */

char huge *nloc;				/* serial-number location */
char huge *cloc;				/* customer-name location */

char npat[] = SERNUM;			/* serial-number pattern */
int  nlen;						/* its length */
char cpat[] = CUSTNAME;			/* customer-name pattern */
int  clen;						/* its length */
char xpat[MAXPAT];				/* XOR pattern */

char *dirName  = "rwi";			/* product directory name */
char *prodName = "RWI.EXE";		/* product to customize MUST BE UPPERCASE */
char *outDisk  = "B:";			/* output disk drive */

FILE *infile;					/* input database file */
char *infileName;				/* its name */

char nbuf[MAXPAT];				/* serial-number buffer */
char cbuf[MAXPAT];				/* customer-name buffer */
char ebuf[MAXPAT];				/* encrypted customer-name buffer */

static char buf[4096];			/* buffer for file transfers */


/*********************************************************************\
**
** customize()
**
**		customize THE file
**
\*********************************************************************/


void customize()
{
	if (cloc) (memcpy)(cloc, ebuf, clen);
	if (nloc) (memcpy)(nloc, nbuf, nlen);
}


/*********************************************************************\
**
** findGoodies()
**
**		find customization points in the most-recently read file
**
\*********************************************************************/


void findGoodies()
{
	register char huge *cp;
	long i;

	for (cp = files[nfiles].buf, i = 0; i < files[nfiles].len; ++i, ++cp) {
		if (*cp == cpat[0] && !(memcmp)(cp, cpat, clen)) cloc = cp;
		if (*cp == npat[0] && !(memcmp)(cp, npat, nlen)) nloc = cp;
	}

	if (cloc == 0L || nloc == 0L) {
		fprintf(stderr, "Unable to locate customization points\n");
		exit(1);
	}
}


/*********************************************************************\
**
** shipDisk()
**
**		Copy the (buffered) product directory to the output drive
**
\*********************************************************************/

void shipDisk()
{
	char path[MAXFNAME];
	int i;
	FILE *f;
	long len;
	char huge *cp;

	fprintf(stderr, 
	        "Place a formatted disk in drive %s; hit any key when ready\n",
			outDisk
		   );
	getch();

	customize();

	fprintf(stderr, "Writing disk... "); fflush(stderr);

	for (i = 0; i < nfiles; ++i) {
		sprintf(path, "%s\\%s", outDisk, files[i].name);
		fprintf(stderr, "%s ", path); fflush(stderr);
		f = fopen(path, "wb");
		if (!f) {
			fprintf(stderr, "Error opening '%s'\n", files[i].name);
			exit(1);
		}
		for (len = files[i].len, cp = files[i].buf; 
			 len > 0;
			 len -= BUFSIZE, cp += BUFSIZE) {
			 	fwrite(cp, 1, (len > BUFSIZE? BUFSIZE : (size_t) len), f);
			 }
		fclose(f);
	}

	fprintf(stderr, "Write complete.\n");
}


/*********************************************************************\
**
** readFile(path, name)
**
**		Read the file into memory, putting it in files[nfiles]
**		If it is THE file, locate the customer name and serial number.
**
\*********************************************************************/

void readFile(path, name)
	char *path;
	char *name;
{
	register int i;
	register char huge *cp;
	FILE *f;
	char fbuf[MAXFNAME];
	struct stat sbuf;

	strcpy(fbuf, path);
	strcat(fbuf, "\\");
	strcat(fbuf, name);
	fprintf(stderr, "  %s", fbuf);
	fflush(stderr);

	f = fopen(fbuf, "rb");
	if (!f || fstat(fileno(f), &sbuf)) {
		fprintf(stderr, "\nError opening '%s'\n", fbuf);
		exit(1);
	}

	cp = halloc((long) sbuf.st_size, 1);
	if (cp == 0L) {
		fprintf(stderr, "\nError allocating buffer for 's'\n", fbuf);
		exit(1);
	}

	strcpy(files[nfiles].name, name);
	files[nfiles].buf = cp;
	files[nfiles].len = sbuf.st_size;

	/*
	** This relies on the buffer being EXACTLY big enough.
	** That means stat and fread have to agree on the file's size.
	** Should be safe...
	*/
	for ( ; i = fread(cp, 1, BUFSIZE, f); cp += i);

	fclose(f);
	
	if (!strcmp(name, prodName)) findGoodies();
}


/*********************************************************************\
**
** readDisk()
**
**		Read the product directory into memory
**
\*********************************************************************/

static struct find_t find_buf;

void readDisk()
{
	char path[MAXFNAME];

	fprintf(stderr, "Reading product directory %s.\n", dirName);

	strcpy(path, dirName);
	strcat(path, "\\*.*");
	if (_dos_findfirst(path, _A_SUBDIR, &find_buf))
		return;
	do {
		if (!strcmp(find_buf.name, ".") || !strcmp(find_buf.name, ".."))
			continue;
		if (find_buf.attrib & _A_SUBDIR) {	 
			/* === ignore subdirectories for now === */
		} else {		/* is a file */
			readFile(dirName, find_buf.name);
			++nfiles;
		}
	} while (! _dos_findnext(&find_buf));


	fprintf(stderr, "\nProduct directory read.\n");
}


/*********************************************************************\
**
** int readEntry()
**
**		read the next entry from the input file.
**		Return 0 if no next entry exists.
**
\*********************************************************************/

int readEntry()
{
	register char *p;
	register int i;
	char buf[MAXLINE];

	/*
	** Skip blank lines, also looking for EOF
	*/
	if (! fgets(buf, sizeof(buf), infile)) return(0);
	while (buf[0] == '\n') if (! fgets(buf, sizeof(buf), infile)) return(0);

	/*
	** Extract the serial number
	*/
	if (p = strchr(buf, ',')) *p = 0;
	if (p = strchr(buf, '\n')) *p = 0;
	strcpy(nbuf, buf);

	/*
	** Extract the customer name
	**		center it in ebuf
	**		xor it with the xor pattern.
	*/
	if (! fgets(buf, sizeof(buf), infile)) return(0);
	if (p = strchr(buf, '\n')) *p = 0;
	strcpy(cbuf, buf);

	for (i = 0; i < clen - 1; ++i) ebuf[i] = ' ';
	ebuf[clen-1] = 0;
	strncpy(ebuf + (clen - 1 - strlen(cbuf)) / 2, cbuf, strlen(cbuf));
	for (i = 0; i < clen; ++i) ebuf[i] ^= xpat[i];

	/*
	** Skip anything else and return success.
	*/
	while (fgets(buf, sizeof(buf), infile) && buf[0] != '\n') ;
	fprintf(stderr, "\nDisk s/n %s  for %s\n", nbuf, cbuf);
	return (1);
}



/*********************************************************************\
**
** main(argc, argv)
**
\*********************************************************************/

main(argc, argv)
	int argc;
	char **argv;
{
	for (++argv; --argc; ++argv) {
		if (**argv == '-')
			switch ((*argv)[1]) {
			 case 'd': case 'D':
			 	dirName = *++argv;
				--argc;
				break;
			 case 'p': case 'P':
			 	prodName = *++argv;
				--argc;
				break;
			 case 'o': case 'O':
			 	outDisk = *++argv;
				--argc;
				break;
			 default:
			 	fprintf(stderr, "Unrecognized option '%s'\n", *argv);
				exit(1);
			}
		else {
			if (infileName) {
				fprintf(stderr, "Only one filename permitted\n");
				exit(1);
			}
			infileName = *argv;
		}
	}
	if (infileName) {
		infile = fopen(infileName, "r");
		if (!infile) {
			fprintf(stderr, "Cannot open input file '%s'\n", infileName);
			exit(1);
		}
	} else {
		fprintf(stderr, "No input file name given\n");
		exit(1);
	}

	/*
	** Initialize the xor pattern and pattern length
	*/
	clen = sizeof (cpat);
	nlen = sizeof (npat);
	XORPAT(xpat, clen);

	/*
	** Read the disk image
	*/
 	readDisk();

	/*
	** Do the transfer for each entry.
	*/
	while (readEntry()) shipDisk();
	exit(0);
}
