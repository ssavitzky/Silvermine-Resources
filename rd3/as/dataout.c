/*
    dataout - data output routine, packed form only


*/
#include <std.h>
#include "..\..\as\spec.h"
#include <stdlib.h>
#include <stddef.h>

#define MAXCHARS 76

IMPORT TEXT hash[];
IMPORT TEXT newline[];
IMPORT TEXT semi[];

IMPORT TEXT buffer[];
IMPORT TEXT valbufr[];
IMPORT TEXT *curptr;

/* no. decimal places are settable in .hdr */
IMPORT LONG ndecpl;
IMPORT LONG xydata;         /*put out in X Y columns if yes*/
LONG getNext3sl(void);

LOCAL DOUBLE f100 = 100.0;      /* useful constant */
LOCAL DOUBLE f10 = 10.0;
LOCAL DOUBLE round = 0.5;

VOID dataout(
    KREC *pkrec)
    {
    LONG i,j,m,n;
    int k,l;
    DOUBLE dbl;         /* temp float numbers */
    ULONG flags;
    LONG taflag;
    LONG idata;        /* current data point after scaling */
    DOUBLE fscale;     /* scale factor */
    LONG curwn;        /* current waveno. val */
    LONG idel;
    FAST TEXT *s;
    FAST LONG temp;
    DOUBLE dtemp;
    LONG npoints;
    int firstk;       /* to check if any items on last line */
	 int maxchars = MAXCHARS;

    /* initializers */
	/* getNext3sl is initialized */
    /* note no exponentiation operator */
    for (fscale=1.0,i=0;i < ndecpl; i++)
        fscale *= f10;
    fscale /= (DOUBLE)pkrec->scale;

    idel = pkrec->ndel;
    curwn = pkrec->istart;
    if (idel % 100)
        if (idel % 10)
            ;       /* hundredth of a wn */
        else
            {
            curwn /= 10;        /* tenths */
            idel /= 10;
            }
    else
        {
        idel /= 100;            /* interval 1 wn or > */
        curwn /= 100;
        }

    k = itob(buffer,curwn,0);      /* initialize first data line */
    buffer[k++] = ' ';
    curptr = &buffer[k];
    *curptr = '\0';
    firstk = k;
	 if (xydata)
	 	maxchars = firstk;
    curwn -= idel;              /* to handle first point properly */

    npoints = pkrec->npts;

    /******* output loop */
    for (j = 0; j < npoints;j++)
        {
	temp = getNext3sl();
	dtemp = (DOUBLE)temp;
        if(temp >= 0)
	    idata = (LONG)(dtemp * fscale + round);
        else
	    idata = (LONG)(dtemp * fscale - round);
        curwn += idel;
    /* number conversion requires the following in order to force a  */
    /* leading +, else could use base = 0 in following itob()        */
        if (idata >= 0)
            valbufr[0] = '+';
        else
            {
            valbufr[0] = '-';
            idata = -idata;
            }
        l = itob(&valbufr[1],idata,10);
        valbufr[++l]='\0';
        if (j && (k+l) > maxchars)
            {
            linewrt(buffer);
            k = itob(buffer,curwn,0);
            buffer[k++] = ' ';
            firstk = k;
            curptr = &buffer[k];
            *curptr = '\0';
            }

        for (s = valbufr; *s;)      /* add next point to line */
            *curptr++ = *s++;
        *curptr = '\0';
        k += l;                         /* update line length */

        }

        if (k > firstk)
            linewrt(buffer);

    return;
    }
