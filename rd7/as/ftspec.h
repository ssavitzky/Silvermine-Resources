/*
    ftspec.h    - spectrum header for the FT-IR
    Programmer : SATole/ESSparks
    date/rev : 84-10-22 R06
*/



typedef struct skrec         /* total length = 512 bytes */
    {                   /* byte number: */
    TEXT  spname[8];    /*   0: spectrum name                      */
    UTINY stype;        /*   8: spectrum type, 0=IR,1=UV,3 = IG, 10 = time */
    UTINY dtype;        /*   9: Data type = 2, 3, or 4 byte */
                                 /* if negative, data is in float form */
    LONG  npts;         /*  10: Actual number of data points */
    LONG  istart;       /*  14: IR = starting wavenumber * 100  */
    LONG  ifin;         /*  18: IR = ending wavenumber * 100 */
    COUNT ndel;         /*  22: IR = wavenumber * 100 */
    ULONG flags;        /*  24: NOTE- 32 flags available */
    LONG  scale;        /*  28: data scale, for T, = 100% */
    LONG  miny;         /*  32: */
    LONG  maxy;         /*  36: */
    COUNT naccs;        /*  40: number of accumulations */
    float absex;        /*  42: absorbance expansion factor */
    UTINY nsmth;        /*  46: smoothing code 1-7 */
    UTINY beam;         /*  47: beam value 0 -26 */
    LONG  rsln;         /*  48: resolution 0 -3200 */
    COUNT cycs;         /*  52: cycles 1-32767 */
    BOOL  rtio;         /*  54: ratio (0) or non-ratio (1) mode */
    UTINY apod;         /*  58: apodisation 0-4 */
    UTINY imag;         /*  59: transform : real = 0, imag = 1, magn = 2 */
    UTINY dctr;         /*  60: detector : 0 = TGS, 1 = MCT */
    UTINY phase;        /*  61: phase correction for spectrum */
    UTINY gain;         /*  62: 1700 gain setting */
    UTINY accy;         /*  63: aqcuire mode */
    UTINY jacq;         /*  64: jaqcuinot stop setting */
    TBOOL mvflag;       /*  65: set if spectrum was obtained via a MOVE */
    UCOUNT opdv;        /*  66: scan speed when spectrum acquired */
    COUNT dspare2[6];   /*  68: data spares */
    UCOUNT instno;      /*  80: Instrument model number */
                        /* >>>>> if 0xffff, file cannot be saved */
    UTINY mode;         /*  82: instrument specific */
    TBOOL gtflag;       /*  83: set if spectrum obtained via a GET */
    COUNT is1;          /*  84: instrument specific, 1500 gain, etc. */
    COUNT is2;          /*  86: */
    TEXT  SPARES1[16];  /*  88: more spares */
    TEXT  QUANT[32];    /* 104: quant info */
    TEXT  SPARES2[20];  /* 136: spares maintain total block 512 bytes */
    TEXT  date[8];      /* 156: yy/mm/dd ASCII  */
    TEXT  time[12];     /* 164:  HH:MM:ss.ss, ASCII, NOTE LEADING blank!! */
    TEXT  ident[72];    /* 176: field should be blank filled */
    BITS  eflag;        /* 177: krec extension flag */
    COUNT ebytes;       /* 250: number of bytes in this extension, 0 or 256 */
    TEXT  *eptr;        /* 252: RESERVED, normally NULL */
    TEXT  xrec[200];    /* 256: krec extension, Instrument Specific */
    TEXT  savxrec[50];  /* 456: RESERVED, and in use */
    COUNT ebyte2;       /* 506: no. of additional extension bytes */
    TEXT  *eptr2;       /* 508: RESERVED, extension in memory */
                        /* 512: total bytes in block */
    } KREC;


typedef struct specl         /* spectrum locator */
    {
    struct specl *flp;       /* forward link pointer to next item */
    ULONG    xyzid;          /* 4byte LONG =  X,Y,Z, etc. trailing blanks */
    LONG     size;           /* max size of DATA area, in bytes */
    KREC     *kptr;          /* points to start of KREC */
    LONG     *sptr;          /* points to start of data area */
    TINY status;             /* primarily for scan, plot, get, etc. */
    TINY spare;
    } SPECLOC;

/* bit definitions: SPECLOC.status */
#define BUSY   1            /* Region Busy */
#define RBUSY  7            /* error returned by getsploc if region busy */

/* bit definitions: KREC.flags      */
#define FLG_TA      1       /* Set if absorbance, else 0 */
#define FLG_LOG     2       /* set by LOG conversion */
#define FLG_DIFF    4       /* set by diff */
#define FLG_FLAT    8       /* set by flat */
#define FLG_MERGE   16      /* set by merge */
#define FLG_ARITH   32      /* set by add,sub,mult,div */
    /*              64       * to be defined */
#define FLG_MOD     128     /* set if modified: shift,change,etc. */

/* NOTE: in at least three cases, flags are implied by existence of
    values in specific variables:
        naccs >1    implies average
        nsmth != 0  implies smooth
        absex != 0  implies abex
        enhwdth && enhfact != 0  implies enhance
*/

/* SPECIAL NOTE:
    KREC.instno = 2**16 - 1 (-1) is used to indicate a copyrighted
                                 and non-saveable spectrum
    In this case, the copyright notce is a null-terminated string
        in the area following instno
 */

/* mode bits for move krec */
#define CNAME     1     /* If set, clear spname area to blanks */
#define CIDENT    2     /* If set, clear ident area to blanks */
#define CEXTEND   4     /* if set, clear ebytes */
#define DONAMEID  8     /* Put name1, op, name2 into ident */
#define NDATE    16     /* Don't update date/time */

