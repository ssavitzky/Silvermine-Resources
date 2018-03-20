/* pctherm - translates binary thermal file to ascii with JCAMP-like header
					and graphs data */
/* File has been tranferred from 7000 disk to PC using rd7ab

	errors go to stdout
	output file, same name, goes to subdir pc
	syntax:
		pctherm filename
	  additional flags:
	   -h  eliminates header
		-e  eliminates ##END=
		-d eliminates output file
		-g eliminates graphing

********************************************************************/
char copyright[]="PCTHERM: Copyright (c) 1989 Silvermine Resources, Inc.";

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <std.h>
#include <io.h>
#include <stdio.h>
#include "pctherm.h"

void errstrg(char*);

VCTRL far *vc = NULL;
extern SHEAD far *sh = NULL;
extern struct disk_dcmeth  far *dcm = NULL;
extern DMETH far *dm = NULL;
extern DHEAD far *dh = NULL;
extern HCMETH far *hcm = NULL;

LOCAL int hdrflg = 1;       /* all flag defaults = YES */
LOCAL int endflg = 1; 
LOCAL int dataflg = 1;
LOCAL int grflg = 1;
LOCAL char fname[32];
LOCAL char outfil[32];
LOCAL char subdir[] = "pc\\";
void far *inalloc;
float far *dataptr;
char far *supbufr;
FILE *infile;
FILE *outf;
/*long getfirst( FILE * , DTHEAD * , long *);*/
int graphth(FILE *, FILE *);
int doOption(char,char *);
void pHeader(FILE *, FILE *);
void pData(FILE far *,FILE far *);

main (int argc,
	char** argv)
	{
	int inhandle;
	int outhandle;
	long splength;		/* file length */
	long i,k;
	void far *bufloc;
	char far *suplbufr;
	long firsty;
	char opts[256], far *opt;
	char c;

	*fname = NULL;    /* for check that name was entered */
	/*
	** Process environment variable
	*/
	if (opt = getenv("PCTHERM")) strcpy(opts, opt);
	else    opts[0] = 0;

	for (opt = strtok(opts, " \t"); opt; opt = strtok(NULL, " \t")) 
		{
		if (*opt == '-') 
			{
			c = opt[1];
			if (!doOption(c, opt)) 
				{
			 	printf("PCTHERM: unknown environment option '-%c %s'\n", c, opt);
				exit(1);
				}
			} 
		else 
			{
			printf("PCTHERM: unknown environment option '%s'\n", opt);
			exit(1);
			}
		}

	/*
	** Process command line arguments
	*/
	for ( ; ++argv, --argc; ) 
		{
		if (**argv == '-') 
			{
			c = (*argv)[1];
			if (!doOption(c, *argv)) 
				{
			 	printf("PCTHERM: unknown option '-%c %s'\n", c, *argv);
				exit(1);
				}
			} 
		else 
			strcpy(fname,*argv);
		}


	if (!*fname)
		{
		printf("Proper command is:\n     pctherm filename  or\n\
     pctherm -h -e filename\n");
		exit(1);
		}
	/* set up output file in subdir \PC */
	strcpy(outfil,subdir);
	strcat(outfil,fname);
	
	if ((infile = fopen(fname,"rb")) == NULL)
		errstrg("couldn't open input file\n");
	if ((inhandle = fileno(infile)) == -1 )
		errstrg("no file handle\n");
	if ((splength = filelength(inhandle)) == -1L)
		errstrg("filelength error\n");

	inalloc = (void *)malloc(i = HEADERBUF); /* holds header only */
	if (inalloc == NULL)
		{
		printf("insufficient memory, malloc \n");
		exit(1);
		}

	bufloc = inalloc;
	if ((read(inhandle,(char far *)bufloc,i))   == -1)
			errstrg("read error\n");

	/**** READ in HEADERS & FLIP ***********/

	thermhdr(inalloc);    /* header only, flips and */
                         /* points to vc,sh,dm,dcm,dh */

	/******************************** GRAPH ************************/
	if (grflg)
		if (graphth(infile,outf))
			dataflg = hdrflg = endflg = NO; /* don't make file if q or n */

/************** OPEN output FILE ******************************/
	if (dataflg)
		if ((outf = fopen(outfil,"wt")) <= (FILE far*)0)
			errstrg("couldn't open output file\n");

/**************************************************************/


	/**** WRITE Header *************/
   if (hdrflg)
		pHeader(infile,outf);

	/***** WRITE data **************/
	if (dataflg)
		pData(infile,outf); /* allocates data area<s> */

	if (endflg)
		fprintf(outf,"##END=\n");

	fclose(infile);
	fclose(outf);
	printf(copyright);

	}
			
void errstrg(char* s)
	{
	puts(s);
	exit(2);
	}

int doOption(char opt, char far *strg)
	{
	switch(opt)
		{
		case 'h':
		case 'H':
			hdrflg = NO;
			return(YES);
		case 'e':
		case 'E':
			endflg = NO;
			return(YES);
		case 'd':
		case 'D':
			dataflg = hdrflg = endflg = NO;
			return(YES);
		case 'g':
		case 'G':
			grflg = NO;
			return(YES);
		default:
/*		 if (isdigit(opt))
				{
				step = atol(&strg[1]);
				return(YES);
				}
*/
			break;
		}
/*	printf("Item is not -h, -e, or -stepvalue\n");  */
	printf("Command line or environment flag is not -h, -e, -d or -g\n");
	return(NO);
	}


