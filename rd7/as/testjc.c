/* testjc - tests jcamp conversion routines */
/* reads .sp file from pc directory, argument is input filename WITHOUT
	extension
	input file is  fname.sp
	output file is fname.dx
	errors go to stdout

	if no name, uses DFLTNAME
********************************************************************/
#include <stdlib.h>
#include <string.h>
#include <std.h>
#include "spec.h"
#include <io.h>
#include <stdio.h>

void errstrg(char*);
char *cpystr(char *,...);
/*#define DFLTNAME "comp1"*/
#define DFLTNAME "polys1"
#define MAXFILE 32000

LOCAL char dfltname[] = {DFLTNAME};
LOCAL char fname[32];
LOCAL char spfl[32];
LOCAL char dxfl[32];
LOCAL char sp[] = {".SP"};
LOCAL char dx[] = {".DX"};
LOCAL char hdrfl[] = {"jcamp.hdr"};	/* default for now */

char *inalloc;
FILE far *spfile;
FILE far *dxfile;
FILE far *hdrfile;
LOCAL KREC skrec;

/*input filename,dx,hdr,data buffer,datalength*/
void jcampdo(char*,FILE*,FILE*,char*,LONG); 
FILE far *openDX(char*);
FILE far *openHDR(char*);

main (int argc,
	char** argv)
	{
	int sphandle;
	int dxhandle;
	int hdrhandle;
	long splength;		/* file length */
	long *spdata;
	KREC far *pkrec;
	int i;

	if (argc == 1)
		strcpy(fname,dfltname);
	else
		strcpy(fname,argv[1]);

	cpystr(spfl,fname,sp,NULL);
	cpystr(dxfl,fname,dx,NULL);
	inalloc = (char *)malloc(MAXFILE);

	if (inalloc == NULL)
		{
		printf("insufficient memory, malloc \n");
		exit(1);
		}

	
	if ((spfile = fopen(spfl,"rb")) == NULL)
		errstrg("couldn't open spfl\n");
	if ((sphandle = fileno(spfile)) == -1 )
		errstrg("no file handle\n");
	if ((splength = filelength(sphandle)) == -1L)
		errstrg("filelength error\n");
	 if (splength >= (long)MAXFILE)
		errstrg("file > MAXFILE\n");
	if ((read(sphandle,inalloc,(unsigned int)splength)) == -1)
		errstrg("read error\n");

/************** OPEN DX FILE, HEADER FILE ******************************/
	if ((dxfile = openDX(dxfl)) <= (FILE far*)0)
		errstrg("couldn't open DX file\n");
	/************* for now, need a header file, even if only dummy */
	if ((hdrfile = openHDR(hdrfl)) <= NULL)
		errstrg("couldn't open header file\n");

/********************************* JCAMPDO *********************/
	jcampdo(fname,dxfile,hdrfile,inalloc,splength);

	errstrg("testjc complete \n");
	}

void errstrg(char* s)
	{
	puts(s);
	exit(2);
	}

void errorPrintf(char* s)
	{
	puts(s);
	}
