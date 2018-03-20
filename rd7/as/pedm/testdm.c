/* testdm - tests data manager conversion routines */
/* reads .spi file from pc directory, argument is input filename WITHOUT
	extension
	input file is  fname.spi
	output file is fname.sp
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
#define DFLTNAME "comp1"
#define MAXFILE 32000

LOCAL char dfltname[] = {DFLTNAME};
LOCAL char fname[32];
LOCAL char spifl[32];
LOCAL char spfl[32];
LOCAL char spi[] = {".SPI"};		  /*sp idris mode */
LOCAL char sp[] = {".SP"};

char *inalloc;
FILE far *spifile;
FILE far *spfile;
LOCAL KREC skrec;

/*input filename,sp,hdr,data buffer,datalength*/
void dmdo(char*,int,char*,LONG); 
FILE far *openDM(char*);

main (int argc,
	char** argv)
	{
	int spihandle;
	int sphandle;
	int hdrhandle;
	long spilength;		/* file length */
	long *spidata;
	KREC far *pkrec;
	int i;

	if (argc == 1)
		strcpy(fname,dfltname);
	else
		strcpy(fname,argv[1]);

	cpystr(spifl,fname,spi,NULL);
	cpystr(spfl,fname,sp,NULL);
	inalloc = (char *)malloc(MAXFILE);

	if (inalloc == NULL)
		{
		printf("insufficient memory, malloc \n");
		exit(1);
		}

	
	if ((spifile = fopen(spifl,"rb")) == NULL)
		errstrg("couldn't open spifl\n");
	if ((spihandle = fileno(spifile)) == -1 )
		errstrg("no file handle\n");
	if ((spilength = filelength(spihandle)) == -1L)
		errstrg("filelength error\n");
	 if (spilength >= (long)MAXFILE)
		errstrg("file > MAXFILE\n");
	if ((read(spihandle,inalloc,(unsigned int)spilength)) == -1)
		errstrg("read error\n");

/************** OPEN DM FILE, ******************************/
	if ((spfile = openDM(spfl)) <= (FILE far*)0)
		errstrg("couldn't open SP file\n");
	if ((sphandle = fileno(spfile)) == -1 )
		errstrg("no file handle for output\n");

/********************************* JCAMPDO *********************/
	dmdo(fname,sphandle,inalloc,spilength);

	errstrg("testdm complete \n");
	}

void errstrg(char* s)
	{
	puts(s);
	exit(2);
	}
