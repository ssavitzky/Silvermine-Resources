/*    jc3hdr2 - spectrum specific items for rd3

    outputs
        SPECTROMETER/DATA SYSTEM
        DATA PROCESSING
    YUNITS

    ??? XUNITS ???
    ??? RESOLUTION ???

*/
#include <stdlib.h>
#include <string.h>
#include <std.h>
#include "..\..\as\spec.h"			/* 3600's didn' know 1800 */
/*#include "spec18.h"             /* 1800 KREC reqd for special treatment*/

#define MAXSTR 60L

IMPORT TEXT hash[];
IMPORT TEXT newline[];
IMPORT TEXT semi[];

IMPORT TEXT buffer[];
IMPORT TEXT valbufr[];
IMPORT TEXT *curptr;


LOCAL TEXT sds[] = "SPECTROMETER/DATA SYSTEM= ";
LOCAL TEXT pkn[] = "PERKIN-ELMER ";
LOCAL TEXT dproc[] = "DATA PROCESSING= ";
LOCAL TEXT smth[] = "SMOOTHING= ";
LOCAL TEXT expnd[] = "EXPANSION= ";
LOCAL TEXT acc[] = "ACCUMULATIONS= ";
LOCAL TEXT enhw[] = "ENHANCE WIDTH= ";
LOCAL TEXT factor[] = "FACTOR= ";
LOCAL TEXT xunits[] = "##XUNITS= 1/CM\n";     /* always! for now */
LOCAL TEXT yunits[] = "YUNITS= ";
LOCAL TEXT *typ[4] = {                        /* flags & (FLG_TA | FLG_LOG) */
    "TRANSMITTANCE ",
    "ABSORBANCE ",
    "LOG TRANSMITTANCE ",
    "LOG ABSORBANCE "
    };

/* smoothing npts */
LOCAL COUNT pt[8] = {5,9,13,19,25,37,49,149};

/* no 1700 or 1800 in 3600cds
void do17(KREC*);
LOCAL void do18(KREC*);
*/

VOID jc3hdr2(
    KREC *pkrec)
    {
    FAST TEXT *si,*so,*s;
    LONG i,j,k;
    double dbl;         /* temp float numbers */
    ULONG flags;
    LONG stype;
    TEXT c;

    /* output data system */
    j = pkrec->instno;
    i = itob(valbufr,j,10);
    valbufr[i]='\0';
	/**/
    cpystr(buffer,hash,sds,pkn,valbufr,newline,(char*)0);
    linewrt(buffer);

    /*1800 is special case 
    if (j== 1800)
        do18(pkrec);
    if (j/100 == 17)
        do17(pkrec);
*/

    /* data processing items, output as strings > MAXLEN */
    curptr = cpystr(buffer,hash,dproc,NULL);
    /* now go down thru list */
    /* ??? background ?? */

    /* smooth */
    if (j = pkrec->nsmth)
        {                               /* not zero */
	i = strlen(ltoa((LONG)pt[j-1],valbufr,10));
        valbufr[i] = '\0';
        curptr = cpystr(curptr,smth,valbufr,semi,NULL);
        }
    else
        curptr = cpystr(curptr,smth,"none",semi,NULL);
    chkline(MAXSTR);

    /* expansion */
    if (dbl=pkrec->absex)
        {
        i = dtof(valbufr,dbl,(BYTES)5,(BYTES)5);
        valbufr[i] = '\0';
        curptr = cpystr(curptr,expnd,valbufr,semi,NULL);
        chkline(MAXSTR);
        }

    /* accumulations */
    if (j = pkrec->naccs)
        addint(acc,j,MAXSTR);

    /* enhancement */
    j = pkrec->enhwdth;
    k = pkrec->enhfact;
    if (j && k)
        {
        addint(enhw,j,64L);               /* 78 - 14 */
        addint(factor,k,MAXSTR);
        }

    /* pertinent flags */
    flags = pkrec->flags;

    if (flags & FLG_FLAT)
        {
        curptr = cpystr(curptr,"FLAT;",NULL);
        chkline(MAXSTR);
        }

    if (flags & FLG_DIFF)
        {
        curptr = cpystr(curptr,"DIFFERENCE;",NULL);
        chkline(MAXSTR);
        }

    if (flags & FLG_MERGE)
        {
        curptr = cpystr(curptr,"MERGE;",NULL);
        chkline(MAXSTR);
        }

    if (flags & FLG_ARITH)
        {
        curptr = cpystr(curptr,"ARITHMETIC;",NULL);
        chkline(MAXSTR);
        }

    if (flags & FLG_MOD)
        {
        curptr = cpystr(curptr,"DATA MODIFIED;",NULL);
        chkline(MAXSTR);
        }

    /* flush if > 2 = leading spaces */
	chkline(2L);

    /* xunits always cm-1::::::::::::::::::::::::::::::::::::::*/
    linewrt(xunits);

    /* now yunits */
    stype = pkrec->stype;
    if ((stype==0) || (stype==1))
        {                           /* IR or UV */
        i = flags & (FLG_TA | FLG_LOG);
        cpystr(buffer,hash,yunits,typ[i],newline,NULL);
        linewrt(buffer);
        }
    else
        {
        c = pkrec->ordid[0];
        if ((c != ' ') && (c != '\0'))
            {
	    strncpy(valbufr,&pkrec->ordid[0],4);
            valbufr[4]='\0';
            cpystr(buffer,hash,yunits,valbufr,newline,NULL);
                linewrt(buffer);
            }
         }
    /*?????????????? DONE ??????????????????????*/
    return;
    }

