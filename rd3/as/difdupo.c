/*
    difdupo - diff/dup data output routine, includes writing ##END


*/
#include <std.h>
#include "..\..\as\spec.h"
#include <stdarg.h>
static void nxtpt(void);

#define MAXCHARS 70

IMPORT TEXT hash[];
IMPORT TEXT newline[];
IMPORT TEXT semi[];

IMPORT TEXT buffer[];
IMPORT TEXT valbufr[];
IMPORT TEXT *curptr;

/* no. decimal places are settable in .hdr */
IMPORT LONG ndecpl;

LOCAL DOUBLE f100 = 100.0;      /* useful constant */
LOCAL DOUBLE f10 = 10.0;
LOCAL TEXT csign[] = "@ABCDEFGHIabcdefghi";
LOCAL TEXT cdup[] = "?STUVWXYZs";
LOCAL TEXT cdiff[] = "%JKLMNOPQRjklmnopqr";
LOCAL LONG k = 0;              /* no. chars in output line */
LOCAL DOUBLE round = 0.5;

LOCAL LONG BUGCOUNT = 0;
LOCAL LONG *dataptr = 0;   /* pointer to current data point */
LOCAL LONG idata = 0;         /* current data point after scaling */
LOCAL DOUBLE fscale = 0.0;    /* scale factor */
LOCAL LONG curwn = 0;         /* current waveno. val */
LOCAL LONG idel = 0;
LOCAL LONG point = 0;         /* current count of data point */


/********************** nxtpt *****************************/
static void nxtpt()
{
    FAST LONG temp;
	 double dbl;

    temp = getNext3sl();
	 dbl = (DOUBLE)temp;
    if (temp >= 0)
        idata = (LONG)(dbl * fscale + round);
    else
        idata = (LONG)(dbl * fscale - round);
    curwn += idel;
    point += 1;
    return;
    }

/************* dxout **************************************/
/*  converts to ASCII, adjusts first char and increments ptrs */
VOID static dxout(
    LONG number,
    TEXT string[])
    {
    LONG l,firstd;

    l = itob(curptr,number,10);
    firstd = *curptr - '0';
    *curptr = string[firstd];
/*
errorPrintf("%l:%b ",number,curptr,1);
if (!(BUGCOUNT++ % 10))
 errorPrintf("\n");
*/
    curptr += l;
    k += l;
    *curptr = '\0';
    return;
    }

/**************************** difdupo *************************/
VOID difdupo(
    LONG *spdata,
    KREC *pkrec)
    {
    LONG i,j,l,m,n;
    double dbl;             /* temp float numbers */
    ULONG flags;
    LONG taflag;
    LONG sign;
    FAST TEXT *s;
    LONG npoints;
    LONG firstk;            /* to check if any items on last line */
    LONG pdata;             /* previous ordinate */
    LONG pdiff;             /* previous difference */
    LONG idiff;             /* current difference */
    LONG firstd;
    LONG equals;            /* flag */
    LONG nequals;           /* no. items that are equal to prev */
    LONG duplast;           /* = YES if last item on line is a dup */


BUGCOUNT = 0;
    /* initializers */
    dataptr = spdata;

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

    npoints = pkrec->npts;

    pdata = idata = (LONG)(*dataptr++ * fscale + round);
    point = 1;
    duplast = NO;
    /******* output loop */
    while (point <= npoints)
        {                       /* come here after line written */
        /* handle beginning of line */
        k = itob(buffer,curwn,0);
        curptr = &buffer[k];
        *curptr = '\0';
        firstk = k;
        /* get curwn back in sync for next **
        curwn += idel;                       */

        /* first ordinate on line is same as last on prev line */
        /* so use pdata */
        sign = pdata >= 0 ? 0:9;
          dxout((pdata>=0?pdata:-pdata),&csign[sign]);

        /* this could be last point */
        if (point == npoints)
            break;

        duplast = NO;
        /* no, so back up for idata *???????????????????
        dataptr--;
        point--;                ??? May not need to */
        /* it may be that logically can't have dup at this point, but!*/
        /* are next points dups? */
        nequals = 0;
        equals = YES;
        while (equals && (point <= npoints))
            {
            nxtpt();            /* get idiff = next point & housekeeping */
            if (idiff = idata - pdata)      /* if differ, idiff not = 0 */
                equals = NO;
            else
                nequals += 1;
            }

        /* now have no. equals */
        if (nequals)    /* logic requires, 2 or greater */
            {                           /* > 0 */
            nequals++;   /* to count first one */
            dxout(nequals,cdup);
            }

        /* could have last point */
        if (point == npoints)
            break;

        /* now have an idata != pdata and difference is in idiff */

        /* next point must be valid difference */
        while((point <= npoints) && (k < MAXCHARS))
            {
            /* have new difference so output it */
            sign = idiff >= 0 ? 0:9;
            dxout((idiff>=0?idiff:-idiff),&cdiff[sign]);

        /* still have room to check for more = diffs */
            pdiff = idiff;
            pdata = idata;
            duplast = NO;
         /* but may be full */
            if (k >= MAXCHARS || (point == npoints))
                break;
        /* are next points dups? */
            nequals = 0;
            equals = YES;
            while (equals && (point <= npoints))
                {
                nxtpt();
                idiff = idata - pdata;
                if (idiff != pdiff)     /* if differ, idiff not = 0 */
                    equals = NO;
                else
                    {
                    nequals += 1;
                    pdata = idata;
                    }
                }
            /* now have no. equals */
            if (nequals)
                {                           /* > 0 */
                nequals++;      /* to count first one */
                dxout(nequals,cdup);
                duplast = YES;
                }

            /* have a new difference, so go around again */
            }

        /* full line, so write */
        linewrt(buffer);
        /* if last was a dup, went too far, must back up */
        if (duplast)
            {
            dataptr--;
            curwn -= idel;
            point--;
            }
        }

    /* get here only when points == npoints */
    /* ----------------I HOPE ------------- */
    linewrt(buffer);
    return;
    }

