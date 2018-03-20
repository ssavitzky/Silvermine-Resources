/* KREC extended, and spectral definition */
/* -date- 84/03/28          Savitzky */
/* -date- 84/04/10  Rev. 1  V. Mehta */
/* -date- 84/04/11  Rev. 2  Savitzky */
/* -date- 84/06/04  Rev. 3  R. Basztura */
/* -date- 84/12/14  Rev. 4  R. Basztura */
typedef struct skrec         /* total length = 512 bytes */
    {                   /* byte number: */
    TEXT  spname[8];    /*   0: spectrum name                      */
    UTINY stype;        /*   8: spectrum type, 0=IR,1=UV,..+10 = time */
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
    TEXT  dspare;       /*  47: data spare */
    COUNT dspare2[16];  /*  48: Instrument specific */
/*  1800   optn.h      */
    UCOUNT    instno;                /* instrument number (1800)            */
    ULONG    serial;                /* instrument serial number            */
    TEXT    software[4];        /* software package name (FTIR)        */
    UCOUNT    revision;            /* software revision number * 100    */
    UCOUNT    odetect;                /* installed detector options        */
                                /* bits 0-3 :    primary detector
                                   bits 4-7 :    secondary detector
                                   bits 8-11:    tertiary detector
                                   codings  :
                                    0    no installation
                                    1    DTGS with CsI window
                                    2    DTGS with polystyrene window
                                        (formerly FIR)
                                    3    MCT
                                    4    InSb
                                    5    photoacoustic detector
                                    6    microscope
                                */
    TINY    source;                /* installed source options            */
                                /* bits 0-3 :    primary source
                                   bits 4-7 :    external source
                                   codings  :
                                    0    no installation
                                    1    standard hot wire
                                    2    globar
                                    3    high pressure mercury
                                    4    quartz-halogen lamp
                                */
    TINY    ondf;                /* neutral density filter            */
                                /* value indicates number of avail-    */
                                /*  able filters                    */
    TINY    oof;                    /* optical filter                    */
                                /* value indicates number of avail-    */
                                /*  able filters                    */
    TINY    pol;                /* polarizer options                */
                                /* bits 0-3 :    first location
                                   bits 4-7 :    second location
                                   codings  :
                                    0    not installed
                                    1    AgBr
                                    2    polystyrene
                                */
    TINY    bspl;                /* beamsplitter options                */
                                /* bits 0-3 :    primary location
                                   bits 4-7 :    secondary location
                                   codings  :
                                    0    not installed
                                    1    KBr
                                    2    Mylar
                                    3    CsI
                                    4    Si02
                                */
    TINY    gcir;                /* reserved for gcir                */
    TINY    opspare[2];            /* spares set to 0                    */

  /*  1800    inst.h     */
/*   1800   inst.h    */
    UTINY    indetect;                /* selected detector                */
                                /* coding:
                                    0    primary location (usu. DTGS)
                                    1    secondary location (usu. MCT)
                                    2    tertiary (auxiliary) location
                                   NOTE:  the actual  detector type
                                          should be determined from
                                          the appropriate  entry in
                                          the OPTN structure
                                */
    UTINY    insource;                /* selected source                    */
                                /* bit 0:
                                    0    internal source off
                                    1    internal source on
                                   bit 1:
                                    0    internal source selected
                                    1    external source selected
                                */
    TINY    insb;                    /* sample beam selected                */
                                /* 0    normal (front beam)
                                  -1    reverse (back beam)
                                */
    UTINY    inndf;                /* selected neutral density filter    */
                                /* 0    no filter
                                   n    number of filter
                                   NOTE: the maximum number of filters
                                         is determined by the entry in
                                         the OPTN structure
                                */
    UTINY    inof;                    /* selected optical filter            */
                                /* coding as for ndf above            */
                                  /* polarizer selected                */
        TINY    ininout;            /* 0    polarizer out                */
                                /* 1    polarizer in                */
        TINY    inangle;            /* polarizer angle (-50 to +95)        */
    TEXT    inspare[5];            /* spares set to 0                    */
    TEXT  SPARES1[22];  /* 114: instrument specific */
	COUNT drvdel;       /* 136: derivative width (1 to 8) * 100 */
	UTINY drvord;       /* 138: derivative order (0 to 4)       */
	TEXT SPARES2[3];    /* 139: instrument specific             */
    UCOUNT enhwdth;     /* 142: enhancement width */
    UCOUNT enhfact;     /* 144: enhancement factor */
    TEXT  ordid[4];     /* 146: ordinate label     */
    TEXT  abscid[6];    /* 150: abscissa label     */
    TEXT  date[8];      /* 156: yy/mm/dd ASCII  */
    TEXT  time[12];     /* 164:  HH:MM:ss.ss, ASCII, NOTE LEADING blank!! */
    TEXT  ident[72];    /* 176: field should be blank filled */
    UTINY gform;        /* 248: global header format indicator */
    UTINY lform;        /* 249: local header format indicator */
    COUNT ebytes;       /* 250: number of bytes in this extension, 0 or 256 */
    TEXT  *eptr;        /* 252: RESERVED, normally NULL */
    /*   1800 mode.h    */
    TEXT   mdname[20];            /* mode name, no intrinsic signifi- */
                                /*  cance                            */
    TEXT    mdident[64];            /* mode identifier string            */
    TINY    num;                /* mode number                        */
                                /* -1    temporary mode
                                    0    survey scan
                            values below have no special significance
                                    1    low energy
                                    2    quantitative
                                    3    qualitive
                                    4    medium resolution
                                    5    high resolution
                                    6    user
                                */
    TINY    detect;                /* reserved for detector type        */
    TEXT    spare1[4];            /* spares, set to 0                    */
    UCOUNT    rsln;                /* resolution * 100                    */
    LONG    ncycles;            /* default number of cycles            */
                                /* On return  in spstat,  this will */
                                /*  contain the  actual  number  of */
                                /*  cycles scanned.                    */
    UCOUNT cycle[9];            /* scan cycle, consisting of:        */
                                /*  [0] switching delay, seconds * 10
                                   first beamsplitter
                                    [1]    first number of sample scans
                                    [2]    number of reference scans
                                    [3]    second number of sample scans
                                    [4]    minor cycle repeat count
                                   second beamsplitter
                                    [5]    first number of sample scans
                                    [6]    number of reference scans
                                    [7]    second number of sample scans
                                    [8]    minor cycle repeat count
                                */
    TEXT    spare2[2];            /* spares, set to 0                    */
    TINY    jacq;                /* jacquinot stop                    */
                                /*  0    AUTO - 1800 selected
                                    1-6    normal range
                                    7    USER
                                */
    TINY    jacq1;                /* reserved for reference beam        */
    TINY    jacq2;                /* reserved for second beamsplitter */
    TINY    jacq3;                /* reserved for second beamsplitter */
    COUNT    opdv[4];            /* opd velocities * 100                */
                                /* in order:
                                    first bspl  - sample beam
                                    first bspl  - reference beam
                                    second bspl - sample beam
                                    second bspl - reference beam
                                */
    TINY    gain;                /* gain                                */
                                /*  0        :    AUTO
                                    1, 2    :    TGS
                                    1,2,4,8    :    MCT
                                */
    TINY    gain1;                /* reserved for reference beam        */
    TINY    gain2;                /* reserved for second beamsplitter */
    TINY    gain3;                /* reserved for second beamsplitter */
    TINY    ndf[4];                /* reserved for the neutral density */
                                /*  filters                            */
    TINY    of[4];                /* reserved for optical filters        */
    TEXT    spare3[4];            /* spares, set to 0                    */
    /* 1800   rcal.h    */
    UCOUNT     hi;                    /* optimum wavenumber range hi        */
    UCOUNT    lo;                    /* optimum wavenumber range lo        */
                                /* These parameters control the de- */
                                /*  cisions  made on  the automatic */
                                /*  selection  of  parameters  (for */
                                /*  example,  jacquinot  stop)  and */
                                /*  control the beamsplitter choice */
                                /*  for recalculating a spectrum.   */
                                /* They do NOT  imply any limits on */
                                /*  the available range for a spec- */
                                /*  trum.                           */
    TEXT    rspare1[2];            /* spares, set to 0                    */
    TINY    acqm;                /* acquire mode                        */
                                /*    0    double beam
                                    1    single ratio
                                    2    single beam
                                    3    single flat
                                    4    single sample
                                    5    single reference
                                    6    single sample flat
                                    7    single reference flat
                                */
    TINY    bidir;                /* bidirectional scan flag            */
    TINY    igram;                /* interferogram type                */
                                /* -1    reserved for right singlesided
                                    0    doublesided
                                    1    left singlesided
                                  NOTE: left singlesided  implies the
                                        bulk of the data is collected
                                        before (left  of) the central
                                        maximum.
                                */
    TINY    rtype;                /* reserved for ramp type            */
    UCOUNT    ramp;                /* semi-ramp length, # of points    */
    UCOUNT    ramp2;                /* reserved for second beamsplitter */
    TINY    apod;                /* apodisation type                    */
                                /*    0    none
                                    1    weak  (Norton-Beer)
                                    2    medium
                                    3    strong
                                    4    triangular
                                    5    raised cosine
                                    6    user-defined
                                */
    TEXT    rspare2;                /* spare, set to 0                    */
    UCOUNT    amult;                /* apodisation linewidth multiplier */
                                /*  (value * 100)                    */
    TINY    spect;                /* spectrum type                    */
                                /*    0    real
                                    1    imaginary
                                    2    magnitude
                                    3    phase
                                */
    TINY    phtyp;                /* source of phase correction        */
                                /*    0    self
                                    1    reference beam
                                    2    previous
                                */
    COUNT    phase;                /* # of phase correction points        */
                                /* -1    AUTO
                                    0    no phase correction applied
                                    n    number of points
                                */
    UCOUNT    phase2;                /* reserved for second beamsplitter */
    TEXT    rspare3[4];            /* spares, set to 0                    */
    /*  1800   rcal.h  again to make 200 bytes  */
    UCOUNT     r2hi;                    /*  optimum wavenumber range hi        */
    UCOUNT    r2lo;                    /* optimum wavenumber range lo        */
                                /* These parameters control the de- */
                                /*  cisions  made on  the automatic */
                                /*  selection  of  parameters  (for */
                                /*  example,  jacquinot  stop)  and */
                                /*  control the beamsplitter choice */
                                /*  for recalculating a spectrum.   */
                                /* They do NOT  imply any limits on */
                                /*  the available range for a spec- */
                                /*  trum.                           */
    TEXT    r2spare1[2];            /* spares, set to 0                    */
    TINY    r2acqm;                /* acquire mode                        */
                                /*    0    double beam
                                    1    single ratio
                                    2    single beam
                                    3    single flat
                                    4    single sample
                                    5    single reference
                                    6    single sample flat
                                    7    single reference flat
                                */
    TINY    r2bidir;                /* bidirectional scan flag            */
    TINY    r2igram;                /* interferogram type                */
                                /* -1    reserved for right singlesided
                                    0    doublesided
                                    1    left singlesided
                                  NOTE: left singlesided  implies the
                                        bulk of the data is collected
                                        before (left  of) the central
                                        maximum.
                                */
    TINY    r2rtype;                /* reserved for ramp type            */
    UCOUNT    r2ramp;                /* semi-ramp length, # of points    */
    UCOUNT    r2ramp2;                /* reserved for second beamsplitter */
    TINY    r2apod;                /* apodisation type                    */
                                /*    0    none
                                    1    weak  (Norton-Beer)
                                    2    medium
                                    3    strong
                                    4    triangular
                                    5    raised cosine
                                    6    user-defined
                                */
    TEXT    r2spare2;                /* spare, set to 0                    */
    UCOUNT    r2amult;                /* apodisation linewidth multiplier */
                                /*  (value * 100)                    */
    TINY    r2spect;                /* spectrum type                    */
                                /*    0    real
                                    1    imaginary
                                    2    magnitude
                                    3    phase
                                */
    TINY    r2phtyp;                /* source of phase correction        */
                                /*    0    self
                                    1    reference beam
                                    2    previous
                                */
    COUNT    r2phase;                /* # of phase correction points        */
                                /* -1    AUTO
                                    0    no phase correction applied
                                    n    number of points
                                */
    UCOUNT    r2phase2;                /* reserved for second beamsplitter */
    TEXT    r2spare3[4];            /* spares, set to 0                    */
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

