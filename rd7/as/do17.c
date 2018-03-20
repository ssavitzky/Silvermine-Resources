/*    do17 - 1700 series specific spectrometer settings

   NOTE--------------------------------------------------
    This routine outputs "INSTRUMENT PARAMETERS" and RESOLUTION
        for 1700 series only, based on ftspec.h 84-10-22 R06

*/
#include <std.h>
#include "ftspec.h"             /* 1700 KREC reqd for special treatment*/

IMPORT TEXT hash[];
IMPORT TEXT newline[];
IMPORT TEXT semi[];

IMPORT TEXT buffer[];
IMPORT TEXT valbufr[];
IMPORT TEXT *curptr;

LOCAL TEXT ss[] = "INSTRUMENT PARAMETERS=";
LOCAL TEXT rsln[] = "RESOLUTION=";
LOCAL TEXT detect[] = "DETECTOR=";
LOCAL TEXT *det[2] = {
    "TGS",
    "MCT"
    };

LOCAL TEXT apd[] = "APODIZATION= ";
LOCAL TEXT *apod[5] = {
    "NONE",
    "TRIANGULAR",
    "WEAK",
    "MEDIUM",
    "STRONG"
    };

char *cpystr(char*,...);
int dtof(char*,double,int,int);
void linewrt(char*);

VOID do17(KREC *pkrec)
    {
    LONG i,j,k;
    DOUBLE dbl;

    curptr = cpystr(buffer,hash,ss,0L);

    /* detector */
    k = pkrec->dctr;
    cpystr(curptr,detect,det[k],semi,newline,0L);
    chkline(2L);

    /* apodization */
    k = pkrec->apod;
    cpystr(curptr,apd,apod[k],semi,newline,0L);
    chkline(2L);

/* that's all for now under settings, can expand more if really needed */

/*--------------------------resolution-----------------------*/
/* this is out of McDonald's sequence, probably OK, but if not,
        just make it a separate call at proper time */
    dbl = (float)pkrec->rsln/100.;            /*??????????????????*/
    i = dtof(valbufr,dbl,(BYTES)5,(BYTES)2);
    valbufr[i] = '\0';
    cpystr(buffer,hash,rsln,valbufr,newline,0L);
	 chkline(2L);

    return;
    }
