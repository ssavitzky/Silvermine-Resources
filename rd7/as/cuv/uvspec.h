/*
  spec.h - spectrum header for ultra violet
  programmer : SATole
  date/rev.  : 83-07-25 R02
	Revised for rd7 : 88/12/13  AS
*/
typedef struct skrec7         /* total length = 256 bytes */
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
    UTINY dspare;            /* spare */
    COUNT scnspd;            /* scan speed * 10, range (0) 19 - 9600 */
    UTINY rspnse;            /* response * 10, range 2 - 100 */
    UTINY nirsens;           /* near infrared sensitivity range 1-8 */
    COUNT uvslit;            /* slit, stored as mms * 100, range 5 - 2000 */
    COUNT uvgain;            /* uv gain, stored * 10, range 1 - 250 */
    COUNT detswitch;         /* detector switch * 10, range 8200 - 9000 */
    COUNT uspare2[9];        /* spares */
    UCOUNT cyctm;            /*AS*/
    UCOUNT ncycl;            /*AS*/
    COUNT instno;            /* instrument model number */
    UTINY ordmode;           /* ordinate mode, range 0 - 7 */
    UTINY cellno;            /* cell number, range 1 - 6 */
    LONG  mono1;             /* monochromator position if a tdrive spectrum */
    TEXT  SPARES1[16];       /* spares */
    TEXT  QUANT[32];         /* reserved for Quant info */
    COUNT drvdel;            /* derivative interval, range 0 - 10 */
    UTINY drvord;     /** added by AS for rd7 !! */
    TEXT  USPARES2[8];      /* spares */ /*adjusted, AS */
    TEXT  ordid[4];          /* AS */
    TEXT  abscid[6];         /* AS */
    TEXT  date[8];           /* yy/mm/dd ASCII  */
    TEXT  time[12];          /* HH:MM:ss.ss , ASCII, note trailing blank */
    TEXT  ident[72];         /* identification line */
    BITS  eflag;             /* krec extension flag */
    COUNT ebytes;            /* number of bytes in extension */
    TEXT  *eptr;             /* pointer to extension in memory */
    } KREC7;


