/* KREC extended and spectral definition */
/* -date- 84/03/28          Savitsky */
/* -date- 84/04/10  Rev.1   V. Mehta */
/* -date- 84/04/11  Rev.2   Savitzky */
/* -date- 84/12/14  Rev.3   Basztura */

typedef struct skrec        /* total length = 512 bytes             */
    {               /* byte number: */
    TEXT    spname[8];      /*   0: spectrum name                   */
    UTINY   stype;          /*   8: spectrum type                   */
                                /*  0   IR
                                    1   UV
                                 + 10   time
                                */
    UTINY   dtype;          /*   9: data type = 2, 3 or 4 byte      */
                            /*      if negative, data is FLOAT      */
    LONG    npts;           /*  10: actual number of data points    */
    LONG    istart;         /*  14: IR = starting wavenumber * 100  */
    LONG    ifin;           /*  18: IR = ending wavenumber * 100    */
    COUNT   ndel;           /*  22: IR = wavenumber * 100           */
    ULONG   flags;          /*  24: NOTE - 32 flags available       */
    LONG    scale;          /*  28: data scale, for T, = 100%       */
    LONG    miny;           /*  32:                                 */
    LONG    maxy;           /*  36:                                 */
    COUNT   naccs;          /*  40: number of accumulations         */
    FLOAT   absex;          /*  42: absorbance expansion factor     */
    UTINY   nsmth;          /*  46: smoothing code, 1 - 7           */
    TEXT    dspare;         /*  47: data spare                      */
    COUNT   dspare2[16];    /*  48: instrument specific             */
    UCOUNT  instno;         /*  80: instrument model number         */
                            /*      if 0xffff, file cannot be saved */
    UTINY   mode;           /*  82: instrument specific             */
    TEXT    ispare;         /*  83: instrument specific             */
    COUNT   is1;            /*  84: instrument specific, 1500 gain, */
                            /*      etc.                            */
    COUNT   is2;            /*  86:                                 */
    TEXT    SPARES1[48];    /*  88: instrument specific             */
    COUNT   drvdel;         /* 136: derivative width (1 to 8) * 100 */
    UTINY   drvord;         /* 138: derivative order (0 to 4)       */
    TEXT    SPARES2[3];     /* 139: instrument specific             */
    UCOUNT  enhwdth;        /* 142: enhancement width               */
    UCOUNT  enhfact;        /* 144: enhancement factor              */
    TEXT    ordid[4];       /* 146: ordinate label                  */
    TEXT    abscid[6];      /* 150: abscissa label                  */
    TEXT    date[8];        /* 156: yy/mm/dd  (ASCII)               */
    TEXT    time[12];       /* 164:  HH:MM:ss.ss  (ASCII)           */
                            /*      NOTE:  LEADING BLANK            */
    TEXT    ident[72];      /* 176: field should be blank-filled    */
    UTINY   gform;          /* 248: global header format indicator  */
    UTINY   lform;          /* 249: local header format indicator   */
    COUNT   ebytes;         /* 250: number of bytes in extension, 0 */
                            /*      or 256                          */
    TEXT   *eptr;           /* 252: RESERVED, normally NULL         */
    TEXT    xrec[200];      /* 256: KREC extension                  */
    TEXT    savxrec[50];    /* 456: RESERVED, and in use            */
    COUNT   ebyte2;         /* 506: # of additional extension bytes */
    TEXT   *eptr2;          /* 508: RESERVED, extension in memory   */
    } KREC;                 /* 512: total bytes in block            */


typedef struct specl        /* spectrum locator                     */
    {
    struct specl *flp;      /* forward link pointer to next item    */
    ULONG   xyzid;          /* 4-byte LONG = X, Y or Z and trailing */
                            /*  blanks                              */
    LONG    size;           /* maximum size of DATA area, in bytes  */
    KREC   *kptr;           /* points to start of KREC              */
    LONG   *sptr;           /* points to start of data area         */
    TINY    status;         /* primairily for scan, plot, set, etc. */
    TINY    spare;
    } SPECLOC;

    /************************************/
    /* bit definitions:  SPECLOC.status */
    /************************************/
#define BUSY          1     /* Region busy                          */
#define RBUSY         7     /* returned by "getsploc"  if region is */
                            /*  busy                                */
    /************************************/
    /* bit definitions:  KREC.flags     */
    /************************************/
#define FLG_TA        1     /* set if absorbance, else 0            */
#define FLG_LOG       2     /* set by LOG conversion                */
#define FLG_DIFF      4     /* set by DIFF                          */
#define FLG_FLAT      8     /* set by FLAT                          */
#define FLG_MERGE    16     /* set by MERGE                         */
#define FLG_ARITH    32     /* set by ADD, SUB, MULT, DIV           */
    /*               64        TO BE DEFINED                        */
#define FLG_MOD     128     /* set if modified: shift, change, etc. */

    /* NOTE:  In at least three cases, flags are implied by existence
              of values in specific variables:
                naccs > 1   implies average
                nsmth != 0  implies smooth
                absex != 0  implies abex
                enhwdth && enhfact != 0 implies enhance
    */

    /* SPECIAL NOTE:   KREC.instno = 2**16 -1,  where (-1) is used to
                       indicate a copyrighted, non-saveable spectrum.
       In this case, the copyright notice is a null-terminated string
       in the area following KREC.instno.
    */
    /************************************************/
    /* bit definitions:  MODE in krec move routines */
    /************************************************/
#define CNAME         1     /* if set, blank KREC.spname area       */
#define CIDENT        2     /* if set, blank KREC.ident area        */
#define CEXTEND       4     /* if set, clear KREC.ebytes and the    */
                            /*  extension area, KREC.xrec           */
#define DONAMEID      8     /* put name1, op, name2 into ident      */
#define NDATE        16     /* do not update date/time              */
