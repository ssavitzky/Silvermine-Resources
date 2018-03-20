/*    jchdr2 - spectrum specific items

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
#include "spec18.h"             /* 1800 KREC reqd for special treatment*/

#define MAXSTR 60L
#define NULL (char *)0L

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

void do17(KREC*);
LOCAL void do18(KREC*);

VOID jchdr2(
    SPECLOC *psploc,
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

    /*1800 is special case */
    if (j== 1800)
        do18(pkrec);
    if (j/100 == 17)
        do17(pkrec);

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

/*************************************do18*************************/
/* NOTE--------------------------------------------------
    This routine outputs "INSTRUMENT PARAMETERS" and RESOLUTION
        entered only if krec.inst is 1800

    if other instrument KRECS must be accommodated, separate
        files will be required for each due to common use
        of name KREC in each. LOCAL routines and buffers will
        have to be duplicated.

--------------------------------------------------*/
LOCAL TEXT ss[] = "INSTRUMENT PARAMETERS= ";
LOCAL TEXT mdname[]= "MODENAME=";
LOCAL TEXT modeid[] = "ID=";
LOCAL TEXT rsln[] = "RESOLUTION=";
LOCAL TEXT detect[] = "DETECTOR=";
LOCAL TEXT *det[8] = {
    "??",
    "DTGS/CsI",
    "DTGS/polys",
    "MCT",
    "InSb",
    "Photoacoustic",
    "Microscope",
	 "??"
    };

LOCAL LONG detmask = 0xfL;

LOCAL TEXT apd[] = "APODIZATION= ";
LOCAL TEXT *apod[7] = {
    "NONE",
    "WEAK",
    "MEDIUM",
    "STRONG",
    "TRIANGULAR",
    "COSINE",
    "USERDEF"
    };

LOCAL VOID do18(pkrec)
    KREC *pkrec;
    {
    short i,j,k;
    DOUBLE dbl;

	/**/
    curptr = cpystr(buffer,hash,ss,(char *)0);

    /* detector */
    j =pkrec->indetect;        /* Pri, sec,tert */
    k = swaps(pkrec->odetect); /* not handled in krecsw1 */
    switch(j)
        {
			case 2:
            k = k >> 4;
   	 	case 1:
            k = k >> 4;
			case 0:
         default:
	    	k = k & detmask;
         }
    curptr = cpystr(curptr,detect,det[k],semi,newline,NULL);
    chkline(2L);


    /* if a standard mode (0-5) just output name */
    /*  else output name and ident */
    i = pkrec->num;
    if ( i >= 0 && i < 6)
        {       /* a standard mode */
        curptr = cpystr(curptr,mdname,&pkrec->mdname[0],NULL);
        rmblanks((LONG)sizeof(pkrec->mdname));
        cpystr(curptr,semi,newline,NULL);
        chkline(2L);
        }
    else
        {      /* user or temporary mode */
        /* chkline or linewrt will add newline */
        curptr = cpystr(curptr,mdname,NULL);
        rmblanks((LONG)sizeof(pkrec->mdname));
        curptr = cpystr(curptr,semi,modeid,&pkrec->mdident[0],NULL);
        rmblanks((LONG)sizeof(pkrec->mdident));
        chkline(2L);
        }

    /* apodization */
    k = pkrec->apod;
    cpystr(curptr,apd,apod[k],semi,newline,NULL);
    chkline(2L);

/* that's all for now under settings, can expand more if really needed */

/*--------------------------resolution-----------------------*/
/* this is out of McDonald's sequence, probably OK, but if not,
        just make it a separate call at proper time */
    dbl = (float)pkrec->rsln/100.;
    i = dtof(valbufr,dbl,(BYTES)5,(BYTES)2);
    valbufr[i] = '\0';
    cpystr(buffer,hash,rsln,valbufr,newline,NULL);
    chkline(2L);

    return;

    }
