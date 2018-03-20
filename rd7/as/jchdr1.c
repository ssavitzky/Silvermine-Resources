/*    jchdr1 - output initial portion of header

    items output are:
        TITLE       as generated
        JCAMP-DX    internal to this routine
        DATA-TYPE   from KREC
        ORIGIN      from header file
        OWNER           "
        sample info     "
        DATE, TIME  from KREC

*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <std.h>
#include "spec.h"
static void slashes(char*);
IMPORT TEXT hash[];
IMPORT TEXT newline[];
IMPORT TEXT semi[];

IMPORT TEXT buffer[];
IMPORT TEXT valbufr[];
IMPORT TEXT *curptr;

/*#define NULL (char *)0*/
/* default no. decimal places */
#define DEFDEC 4
GLOBAL LONG ndecpl = DEFDEC;
GLOBAL LONG difdup = NO;         /*DIFF/DUP mode flag */

LOCAL TEXT *dtype[] =
        {
        "INFRARED ",
        "ULTRAVIOLET ",
        "LUMINESCENCE ",
        "UNDEFINED "
        };

/************************version change requires recompilation*******/
LOCAL TEXT version[] = "##JCAMP-DX= 4.24\n";
LOCAL TEXT ttitle[] = "##TITLE= ";

VOID jchdr1(
    TEXT *title,
    KREC *pkrec)
    {
    TEXT outline[82];
    TEXT temp[82];
    LONG tmpsize;
    LONG stype,sstype;          /* for spectrum type */
    TEXT *stype2;                /* SPECTRUM or TIME DRIVE */
    LONG i,j,k;
    FAST TEXT *si,*so;
    LONG first;

    tmpsize = sizeof(temp);

    /* set // defaults */
    ndecpl = DEFDEC;
    difdup = NO;

    /* output title */
    cpystr(outline,ttitle,title,newline,0L);
    linewrt(outline);

    /* output version */
    linewrt(version);

    /* spectrum type */
    stype = pkrec->stype;
    sstype = stype - 10;           /* time drive is >0 */
    if (stype >= 10)
        stype =  stype % 10;
    i = min (stype,4);      /* change if find more stypes */
    if (sstype < 0)
        stype2 = "SPECTRUM";
    else
        stype2 = "TIME DRIVE";

    cpystr(outline,hash,"DATA TYPE=",dtype[i],stype2,newline,0L);
    linewrt(outline);

    /* now collect from header file */
    /* first line may be ##TITLE, to be ignored here */
    /* else write input line */
    first = YES;
    zbufr(temp,tmpsize);
    while(rdline(temp,tmpsize)> 0)
        {                   /* zero is EOF, <0 is ?? */
        if (first)
            {
	    if (strnicmp(temp,ttitle,8))  /*if its not TITLE*/
                first = NO;       /* doesn't skip write */
            }
        if (first)
            first = NO;             /* and skips write */
        else
            {
            if (!strnicmp(temp,"//",2))
                slashes(&temp[2]);
            else
                linewrt(temp);
            }
        zbufr(temp,tmpsize);
        }

    /* have read header file, now other items */

    /* date */
    si = pkrec->date;
    so = temp;
    j = sizeof(pkrec->date);
    for (i=0;i<j;i++)
        *so++ = *si++;
    *so = '\0';
    cpystr(outline,hash,"DATE= ",temp,newline,NULL);
    linewrt(outline);

    /* time */
    si = pkrec->time;
    so = temp;
    j = sizeof(pkrec->time);
    for (i=0;i < j;i++)
        *so++ = *si++;
    *so = '\0';
    cpystr(outline,hash,"TIME= ",temp,newline,NULL);
    linewrt(outline);

    /* enuf for now */
    return;
    }

/*********************slashes**********************************/
/* handles lines beginning with //                            */
/* labels must be in upper case                               */
LOCAL TEXT decimals[] = "DECIMAL";      /* handles decimal<S> */
LOCAL TEXT dfdp[] = "DIF/DUP";
LOCAL TEXT dffdp[] = "DIFF/DUP";
LOCAL int ldec = sizeof(decimals)-1; /* size includes NULL! */
LOCAL int ldf = sizeof(dfdp)-1;
LOCAL int ldff = sizeof(dffdp)-1;

LOCAL VOID slashes(
    TEXT *line)
    {
    FAST char *s;
    LONG k;
    if ((!strnicmp(line,dfdp,ldf))\
             ||(!strnicmp(line,dffdp,ldff)))
        {
        difdup = YES;
        return;
        }
    if (!strnicmp(line,decimals,ldec))
        {
        s = strchr(line,'=');
        s++;            /* get past the = */
        k = atol(s);
        if (k < 1 || k > 10 )
            errorPrintf("Invalid decimals, default set to 4 \n");
        else
            ndecpl  = k;
        return;
        }

    errorPrintf("Label not recognized\n");
    return;
    }
