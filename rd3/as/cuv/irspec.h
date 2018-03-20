/* -DATE-05/28/82            */
/* mod for rd3 90-06-13  AS */
typedef struct skrec         /* total length = 256 bytes */
    {
    TEXT  spname[8];
    UTINY stype;             /* spectrum type, 0=IR,1=UV,....       */
    UTINY dtype;             /* Data type = 2, 3, or 4 byte */
    LONG  npts;
    LONG  istart;            /* IR = starting wavenumber * 100  */
    LONG  ifin;              /* IR = ending wavenumber * 100 */
    COUNT ndel;              /* IR = wavenumber * 100 */
    ULONG flags;             /* NOTE- 32 flags available */
    LONG  scale;             /* data scale, for T, = 100% */
    LONG  miny;
    LONG  maxy;
    COUNT naccs;             /* number of accumulations */
    float absex;             /* absorbance expansion factor */
    UTINY nsmth;
    TEXT  dspare;            /* data spare */
	COUNT filter;			/* 983 filter n5 after ST*/
	COUNT beamod;				/* 983 beam mode  n2 after ST SC */
    COUNT dspare2[14];       /* data spares */
    COUNT instno;            /* Instrument model number */
    UTINY mode;              /* instrument specific */
	TEXT ispare;
	LONG mono1;				/* monochromator postion in time drive */
    TEXT  SPARES1[16];
    TEXT  QUANT[32];         /* reserved for Quant info */
    TEXT  SPARES2[20];       /* spares maintain total block = 256 bytes */
    TEXT  date[8];           /* yy/mm/dd ASCII  */
    TEXT  time[12];          /* HH:MM:ss.ss , ASCII, note trailing blank */
    TEXT  ident[72];
    BITS  eflag;             /* krec extension flag */
    COUNT ebytes;            /* number of bytes in extension */
    TEXT  *eptr;             /* pointer to extension in memory */
    } KREC;
