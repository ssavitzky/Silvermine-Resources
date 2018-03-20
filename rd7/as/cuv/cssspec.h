/*
 *
 * spec.h - spectrum header
 *
 * PECSS    Lambda 5/7/9
 *
 *      Note:  byte - refers to byte offset in variable table
 *
 *
 *  PSollboehmer
 *  Bodenseewerk Perkin-Elmer
 *  7770 Ueberlingen
 *
 */


typedef struct skrec {      /* total length = 512 bytes */

    TEXT    spname[8];      /* byte = 0; spectrum name */
    UTINY   stype;          /* byte = 8; spectrum type,0=IR,1=UV,2=LS; TD=10+*/
    TINY    dtype;          /* byte = 9; data type = 2, 3, or 4 bytes */
    LONG    npts;           /* byte = 10; number of data points in spectrum */
    LONG    istart;         /* byte = 14; wave finish * 100 (nm or secs) */
    LONG    ifin;           /* byte = 18; wave start * 100 (nm or secs) */
    COUNT   ndel;           /* byte = 22; interval * 100 (nm or secs) */
    UTINY   manip;          /* byte = 24; spectrum manipulation flags */
    UTINY   ordtype;        /* byte = 25; ordinate type 1=A, 2=%T, 3=I */
    COUNT   filler;         /* byte = 26; RESERVED, Do not use */
    LONG    scale;          /* byte = 28; int. data scale, see scaldef */
    LONG    miny;           /* byte = 32; ordinate minimum */
    LONG    maxy;           /* byte = 36; ordinate maximum */
    COUNT   naccs;          /* byte = 40; number of accumulations (averages) */
    FLOAT   absex;          /* byte = 42; absorbance expansion factor */
    UTINY   nsmth;          /* byte = 46; smoothing code 1-7 */
    UTINY   method;         /* byte = 47; method (1 - 5) survey,detail,etc. */
    UTINY   accsry;    /* byte = 48; accessory (1 - 3) manual,sipper,cellpro */
    UTINY   ordmode;        /* byte = 49; ord mode format, (0-7)  */
    UCOUNT  cyclnum;        /* byte = 50; cycle number for this spectrum */
    UCOUNT  ncycles;        /* byte = 52; number of cycles */
    ULONG   cycltm;         /* byte = 54; cycle time * 100 */
    COUNT scnspd;           /*  58: scan speed * 10, range (0) 19 - 9600 */
    UTINY rspnse;           /*  60: response * 10, range 2 - 100 */
    UTINY nirsens;          /*  61: near infrared sensitivity range 1-8 */
    COUNT uvslit;           /*  62: slit, stored as mms * 100, range 5 - 500*/
    COUNT uvgain;           /*  64: uv gain, stored * 10, range 1 - 250 */
    COUNT detswitch;        /*  66: detector switch * 10, range 8200 - 9000 */
    UTINY cellno;           /*  68: cell number (cell changer) */
    TINY  dummy[5];         /*  69: filler */
    COUNT   res;            /* byte = 74; resolution */
    LONG    tdwave;         /* byte = 76; time drive wavelength value */
    UCOUNT  instno;         /* byte = 80; instrument model number */
    TEXT    srmeth[9];      /* byte = 82; STORE/RESTORE method name */
    TEXT    filler2[44];    /* byte = 91  */
    TEXT    scan;           /* byte = 135; SCAN prompt value */
    COUNT   drvdel;         /* byte = 136; derivative interval 0 - 10 */
    UTINY   drvord;         /* byte = 138; derivative order 0-4 */
    TEXT    hscan;          /* byte = 139; HSCAN prompt value */
    UCOUNT  nsamples;       /* byte = 140; no. of samples for multi-sampler */
    UCOUNT  enhwdth;        /* byte = 142; enhance width * 100 */
    UCOUNT  enhfact;        /* byte = 144; enhance factor * 100 */
    TEXT    ordid[4];       /* byte = 146; ordinate label */
    TEXT    abscid[6];      /* byte = 150; abscissa label */
    TEXT    date[8];        /* byte = 156; yy/mm/dd in ASCII */
    TEXT    time[12];       /* byte = 164; HH:MM:ss.ss ,ASCII, trailing blank*/
    TEXT    ident[72];      /* byte = 176; identification line */
    UTINY   gform;          /* byte = 248; globally defined header format */
    UTINY   lform;          /* byte = 249; locally defined header format */
    COUNT   ebytes;         /* byte = 250; number of bytes in extension */
    ULONG   scaldef;        /* byte = 252; Scale defintion * 100 */
    TEXT    extens[44];     /* byte = 256; reserved */
    ULONG   enzfact;        /* byte = 300; ZKINS enzyme factor */
    TEXT    blank;          /* byte = 304; blank field for prompt (menu.c) */
    TEXT    byte;           /* byte = 305; for prompt/input of small integer */
    TEXT    ordstr[3];      /* byte = 306; ordmode prompt value */
    TEXT    rdyln;          /* byte = 309; value for ready (y/n/b) */
    TEXT    ytype;          /* byte = 310; ord value type A(ute),D,V */
    LONG    ymin;           /* byte = 311; ordmin used in view and ready(y/n)*/
    LONG    ymax;           /* byte = 315; ordmax used in view & ready(y/n)*/
    TEXT   region;          /* byte = 319; region X, Y, Z */
    TEXT   chart;           /*  320; recorder status 0=OFF, 1=Dash,2=CONT*/
    TEXT   asave;           /*  321; autosave Y/N */
    TEXT   aprint;          /*  322; autoprint Y/N */
    TEXT   ncells;          /*  323; number of cells for CPRG */
    LONG   cofact;          /*  324; concentration factor for CONC */
    TEXT   azbg;            /*  328; autozero/background value (A, B) */
    COUNT  cssmeth;         /*  329; PECSS method number */
    TEXT   azero;           /*  331; Autozero on RESTORE (Y/N) */
    TEXT   reserved[15];    /*  332 */

    /* conc/calib data */
    TEXT  co_meth[9];       /*  347; conc method name */
    TEXT  co_smpid[9];      /*  356; sample identification */
    TINY  co_rflag;         /*  365; flag for repetitive output */
    TINY  co_arith;         /*  366; flag indicating arithmetics was done */
    TINY  co_nwave;         /*  367; number of wavelengths */
    LONG  co_wave[3];       /*  368; measurement wavelengths */
    COUNT co_curve;         /*  380; calibration curve function 1 ..  4 */
    FLOAT co_calp[3];       /*  381; parameters of calibration curve */
    FLOAT co_ordv;          /*  394; calculated ordinate value */
    FLOAT co_concf;         /*  398; calculated/given sample concentration */
    LONG  co_concl;         /*  402; same in long type */
    FLOAT co_lolim;         /*  406; low limit of calibration ordinate range */
    FLOAT co_hilim;         /*  410; high limit of calibration ordinate range*/
    TEXT  co_rsign;         /*  414; character indicating inside/out cal.rang*/

    TEXT  reserv2[4];       /*  415; */

    /* wavprog items must appear in the same order as in concrec structure */
    TEXT   wp_meth[9];      /*  419; wavprog method name */
    TEXT   wp_smpid[9];     /*  428; wavprog sample identification */
    TINY   wp_rflag;        /*  437; flag for repetitive output */
    TINY   wp_arith;        /*  438; flag indicating arithmetics was done */
    TINY   wp_nwave;        /*  439; number of wavelengths */
    LONG   wp_wave[8];      /*  440; wavprog wavelengths */

    TEXT    xrec[40];       /*  472; krec extension, instrument specific*/
} KREC;



typedef struct specloc {        /* spectrum locator */
    struct specloc *flp;    /* forward link pointer */
    ULONG   xyzid;          /* 4byte LONG = X,Y,Z, etc. trailing blank */
    LONG    size;       /* max. size of DATA area, in bytes */
    KREC    *kptr;      /* points to start of krec */
    LONG    *sptr;      /* points to start of data area */
    TINY    status;     /* primarily for scan, plot, get, etc. */
    TINY    spare;      /* extra location */
} SPECLOC;

/* flag masks for krec MANIP variable */

#define FLG_LOG         1   /* set by LOG conversion */
#define FLG_DIFF        2   /* set by diff */
#define FLG_FLAT        4   /* set by flag */
#define FLG_MERGE       8   /* set by merge */
#define FLG_ARITH       16  /* set by arith */
#define FLG_MOD         32  /* set if modified - shift,change,etc */
#define FLG_AVE         64


/* the following variables also have meanings similar to flags
    naccs > 1 implies average
    nsmth !=0 implies smooth
    absex !=0 implies abex
    enhwdth && enhfact !=0 implies enhance
*/

/* stype definitions */

#define IR              0
#define UV              1
#define FL              2
#define INTERF          3
#define NEUT            9
/* Time drive = stype + 10 */
/* Rep scan = stype + 20 */


/* krec ORDTYPE definitions */

#define TYP_ABS     1       /* Ordinate absorbance */
#define TYP_TRA     2       /* Ordinate transmittance */
#define TYP_INT     3       /* Ordinate intensity */

/* accessories */
#define MANUAL 1
#define SIPPER 2
#define CELLPRG 3
