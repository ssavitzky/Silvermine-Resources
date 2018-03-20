/*
    jchdr3 - data specific header items


*/
#include <stdlib.h>

#include <std.h>
#include "..\..\as\spec.h"

#define FIELD 8
#define MAXLABELS 10
IMPORT TEXT hash[];
IMPORT TEXT newline[];
IMPORT TEXT semi[];

IMPORT TEXT buffer[];
IMPORT TEXT valbufr[];
IMPORT TEXT *curptr;

IMPORT LONG ndecpl;
IMPORT LONG xydata;         /*put out in X Y columns if yes*/

LOCAL TEXT *labels[MAXLABELS] = {
    "XFACTOR=",
    "FIRSTX=",
    "LASTX=",
    "DELTAX=",
    "NPOINTS=",                   /* the only integer */
    "YFACTOR=",
    "MINY=",
    "MAXY=",
    "FIRSTY=",
    "XYDATA= (X++(Y..Y))"    /* does not take a value */
    };
LOCAL TEXT xylabel[] = "XYDATA= (X Y)"; /* XY pairs */

LOCAL TINY fmt[MAXLABELS][2] = {        /* formats for above items */
        2,2,
        6,2,
        6,2,
        4,2,
        -1,-1,                          /* integer special case */
        6,4,
	0,0,                             /* filled in dep on t/a */
        0,0,
        0,0,
        -1,-1,                          /* no value */
	};

LOCAL DOUBLE f100 = 100.0;      /* useful constant */
LOCAL DOUBLE f10 = 10.0;


VOID jc3hdr3(
    LONG firstpoint,
    KREC *pkrec)
    {
    LONG i,j,k,l;
    double dbl;         /* temp float numbers */
    ULONG flags;
    LONG taflag;
     LONG lblno;            /* current label */
     TEXT *lblptr;
     LONG *dataptr;     /* start of data area */
     DOUBLE fscale;     /* scale factor */
     DOUBLE ymin,ymax,firsty;


    /* initializers */

    /* pertinent flags */
    fscale = (DOUBLE)pkrec->scale;
    ymin = pkrec->miny/fscale;
    ymax = pkrec->maxy/fscale;
    firsty = firstpoint/fscale;

    flags = pkrec->flags;
    taflag = flags & (FLG_TA | FLG_LOG);

    for (i = 6; i < 9;i++)
        {
	fmt[i][0] = (TINY)(FIELD - ndecpl);
	fmt[i][1] = (TINY)ndecpl;
        }

    /*only way to do this seems just go down lblnos */
    for (lblno = 0; lblno < MAXLABELS ; lblno++)
        {
		  if ((lblno == 9) && xydata)
		  	lblptr = xylabel;
		  else
        	lblptr = labels[lblno];
	switch ((int)lblno)
            {
        case 0:
            /* xfactor depends on deltax */
            l = pkrec->ndel;
                if (l%100)              /* fractional interval */
                    {
                    if (l%10)
                        dbl = 0.01;
                    else
                        dbl = 0.1;
                    }
                  else
                    dbl = 1.0;
            break;
        case 1:
            dbl = pkrec->istart/f100;
            break;
         case 2:
            dbl = pkrec->ifin/f100;
            break;
        case 3:
            dbl = (FLOAT)pkrec->ndel/f100;
            break;
        case 4:
            k = pkrec->npts;
            j = itob(valbufr,k,10);
            valbufr[j]='\0';
            break;
        case 5:                     /* yfactor depends on absorb or trans */
            if (taflag < 2 )
                {
                dbl = 1.0;
                for(i = 0;i < ndecpl;i++)
                    dbl /= f10;
		fmt[lblno][0] = 4, fmt[lblno][1] = (TINY)ndecpl;
                }
            else                    /* LOG */
                {
                dbl = 1.0;
                ndecpl = 2;
		fmt[lblno][0] = 4, fmt[lblno][1] = (TINY)ndecpl;
                }
            break;
        case 6:
            dbl = ymin;
            break;
        case 7:
            dbl = ymax;
            break;
        case 8:
            dbl = firsty;
            break;
        case 9:
        default:
            break;
            }

        /* start line */
        curptr = cpystr(buffer,hash,lblptr,NULL);
        *curptr = '\0';

        if (lblno<9)        /* no value for 9 */
            {
            if (lblno == 4)     /* npoints */
                cpystr(curptr,valbufr,newline,NULL);
            else
                {
            i = dtof(valbufr,dbl,(BYTES)fmt[lblno][0],(BYTES)fmt[lblno][1]);
                valbufr[i] = '\0';
                cpystr(curptr,valbufr,newline,NULL);
                }
            }
        linewrt(buffer);
        }       /* end of lblno loop */

    return;
    }
