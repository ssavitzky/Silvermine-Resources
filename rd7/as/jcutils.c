/*
    jcutils - contains global buffers, pointers and small
             utilities, primarily for jchdr*

    also contains rdline, linewrt and initializers

    rdline - reads line at a time from a single fd setup in rdopen
    rdinit - must be called before rdline for initialize fio

    linewrt - writes the output line to file fdo
                and displays on screen
    wrtInit - initializer must be called first to set fdo

*/
#include <stdlib.h>
#include <std.h>
#include <stdio.h>
#include <string.h>
#include <io.h>

GLOBAL TEXT hash[] = "##";
GLOBAL TEXT newline[] = "\n";
GLOBAL TEXT semi[] = ";";

GLOBAL TEXT buffer[128] = {NULL};
GLOBAL TEXT valbufr[32] = {NULL};
GLOBAL TEXT *curptr = {NULL};

LOCAL FILE *dxfile = NULL;
LOCAL FILE *hdrfile = NULL;
LOCAL int fd = NULL;
/* finit proto till I decide what to do ******************/
int finit(FILE*,int,int);


/****************************chkline **********************/
/* checks whether line long enuf to output */
VOID chkline(
    LONG maxstr)
    {
    FAST LONG l;         /* line length */

    l = strlen(buffer);
    if (l > maxstr)
        {
        linewrt(buffer);    /* linewrt tacks on newline if needed */
        zbufr(buffer,128L);
        curptr=cpystr(buffer,"  ",NULL);        /* 2 leading blanks */
        }

    return;
    }


/********************************addint************************/
/* translates an integer to text, adds to a string, and checks length */
VOID addint(strg,ival,maxstr)
    TEXT *strg;
    LONG ival;
    LONG maxstr;
    {
    LONG i;

    i = itob(valbufr,ival,10);
    valbufr[i] = '\0';
    curptr = cpystr(curptr,strg,valbufr,semi,NULL);
    chkline(maxstr);

    return;
    }


/**************** remove trailing blanks ****************/
VOID rmblanks(
    LONG size)
    {
    size--;                 /* backup from null */
    for(;size > 0 && (curptr[-1] == ' '); size--,curptr--)
        ;
    return;
    }

/*************************rdinit *******************************/
VOID rdinit(FILE *fdr)
   {
   hdrfile = fdr;
   return;
   }
/****************************************************************/
/**************************rdline ***************************/
LONG rdline(s,n)
    TEXT *s;
    LONG n;
    {
	char* strg;
	
   strg = fgets(s,(int)n,hdrfile);
	if (strg <= 0)
		return(0L);

    return((LONG)strlen(strg));
    }

/********************************write**************************/

VOID wrtInit(
    FILE *fdo)
    {
    dxfile = fdo;
	fd = fileno(dxfile);
    return;
    }

VOID linewrt(line)
    TEXT *line;           /* must be NULL terminated */
    {
    int length;

    length = strlen(line);
    if (line[length-1] != '\n')
        {
        line[length++]= '\n';
        line[length] = '\0';
        }

    write(fd,line,length);
    return;
    }
