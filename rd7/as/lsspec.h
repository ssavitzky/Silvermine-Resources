/*
  spec.h - spectrum header for luminescence
  programmer : VJMehta, SACale
  date/rev.  : 83-04-22 R07
*/
typedef struct skrec         /* total length = 256 bytes */
    {
    TEXT  spname[8];         /* spectrum name */
    UTINY stype;             /* spectrum type, 0=IR,1=UV,2=LS; TD=10+  */
    UTINY dtype;             /* Data type = 2, 3, or 4 byte */
    LONG  npts;              /* number of data points in spectrum */
    LONG  istart;            /* start * 100 (nm or secs) */
    LONG  ifin;              /* finish * 100 (nm or secs) */
    COUNT ndel;              /* interval * 100 (nm or secs) */
    ULONG flags;             /* NOTE- 32 flags available */
    LONG  scale;             /* int. data scale;100%T,1A or 1 lum. unit */
    LONG  miny;              /* minimum intensity */
    LONG  maxy;              /* maximum intensity */
    COUNT naccs;             /* number of accumulations */
    FLOAT absex;             /* absorbance expansion factor */
    UTINY nsmth;             /* smoothing code, range 1-7 */
    UTINY mono;              /* monochromator; EX=0, EM=1, SY=2 */
    COUNT scnspd;            /* scan speed * 10 */
    UTINY rspnse;            /* response */
    UTINY ordfmt;            /* ord. mode format, A/R=0,F/S=1,A/C=2,A/Z=4 */
    COUNT exslit;            /* excitation slit * 10 */
    COUNT emslit;            /* emission slit * 10 */
    COUNT pdelay;            /* phosphorescence delay time * 100 */
    COUNT pgate;             /* phosphorescence gate time * 100 */
    COUNT cycltm;            /* cycle time * 10 */
    UTINY exshtr;            /* ex. shutter,0=open,1=closed,2=locked */
    UTINY emfltr;            /* emission filter 0-7 */
    TEXT  cornam[8];         /* file from which correction factors read */
    FLOAT bckgfc;            /* background subtraction factor */
    FLOAT concfc;            /* concentration factor */
    COUNT instno;            /* instrument model number */
    TEXT  mode;              /* instrument mode - F, P, I, X, L */
    TINY  volt;              /* voltage/ratio 0-100 or -1 */
    LONG  mono1;             /* { EM * 100 if scan EX or SY, EX * 100 if */
    LONG  mono2;             /* scan EM, EX and EM fixed if Tdrive } */
    FLOAT ordexp;            /* ordinate expansion factor */
    BOOL  scanup;            /* YES if scan low to high, else NO */
    COUNT iphvlt;            /* voltage for integrated phosphorescence */
    UTINY iphrsp;            /* filter response time for integrated phos. */
    TEXT  dspare;            /* spare to maintain total block=256 bytes */
    TEXT  QUANT[32];         /* reserved for Quant info */
    COUNT drvdel;            /* derivative interval * 100 */
    UTINY drvord;            /* derivative order 0-4 */
    TEXT  MPF66[17];         /* spares reserved for LS accessories */
    TEXT  date[8];           /* yy/mm/dd ASCII  */
    TEXT  time[12];          /* HH:MM:ss.ss , ASCII, note trailing blank */
    TEXT  ident[72];         /* identification line */
    BITS  eflag;             /* krec extension flag */
    COUNT ebytes;            /* number of bytes in extension */
    TEXT  *eptr;             /* pointer to extension in memory */
    } KREC;


typedef struct specl         /* spectrum locator */
    {
    struct specl  *flp;           /* forward link pointer to next item */
    ULONG    xyzid;          /* 4byte LONG =  X,Y,Z, etc. trailing blanks */
    LONG     size;           /* max size of DATA area, in bytes */
    KREC     *kptr;          /* points to start of KREC */
    LONG     *sptr;          /* points to start of data area */
    TINY status;             /* primarily for scan, plot, get, etc. */
    TINY spare;
    } SPECLOC;
