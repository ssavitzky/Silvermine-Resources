/*
 * spec.h - spectrum header for luminescence
 * VJMehta, SACale
 * date/rev 83-04-22 R07
 * rev: 10-2-84  modified skrec
 */

typedef struct skrec7 {          /* total length = 256 bytes */
    TEXT    spname[8];  /* spectrum name */
    UTINY   stype;      /* spectrum type, 0=IR,1=UV,2=LS; TD=10+ */
    UTINY   dtype;      /* data type = 2, 3, or 4 bytes */
    LONG    npts;       /* number of data points in spectrum */
    LONG    start;         /* start * 100 (nm or secs) */
    LONG    stop;         /* finish * 100 (nm or secs) */
    COUNT   ndel;       /* interval * 100 (nm or secs) */
    ULONG   flags;      /* NOTE 32 flags available */
    LONG    scale;      /* int. data scale;100%T, 1A or 1 lum */
    LONG    miny;       /* minimum intensity */
    LONG    maxy;       /* maximum intensity */
    COUNT   naccs;      /* number of accumulations */
    FLOAT   absex;      /* absorbance expansion factor */
    UTINY   nsmth;      /* smoothing code 1-7 */
    UTINY   nodule;         /* module no. */
    UTINY   method;         /* method code */
    UTINY   mode;           /* ordinate mode 1-7 */
    UTINY   inttm;            /* integration period */
    UTINY   smooth;         /* data smoothing code or int code */
    UCOUNT  deltm;          /* time interval for data * 100 */
    UCOUNT  ncal;           /* number of calculation */
    UCOUNT  idno;           /* id number */
    UCOUNT  scnspd;         /* scanning speed */
    ULONG   fixwave;        /* fixed wavelength */
    LONG    ordmax;         /* ordinate max * n */
    LONG    ordmin;         /* ordinate min * n */
    ULONG   wave1;          /* 1st wavelength  */
    ULONG   wave2;
    ULONG   wave3;
    ULONG   wave4;
    ULONG   wave5;
    ULONG   wave6;
    ULONG   fact1;          /* factor for 1st wavelength */
    ULONG   fact2;
    ULONG   fact3;
    ULONG   fact4;
    ULONG   fact5;
    ULONG   fact6;
    UTINY   plot;           /* plot on/off */
    UTINY   axis;           /* axis on/off */
    UTINY   serial;         /* serial/overlay */
    UTINY   disk;           /* disk on/off */
    UTINY   table;          /* tabe on/off */
    UTINY   spec;           /* reagent/product */
    UCOUNT  slit;           /* slit size * 100 */
    UCOUNT  cyctm;          /* cycle time */
    UCOUNT  ncycl;          /* no. of cycle */
    COUNT   instno;         /* instrument number */
    TEXT    spare1;      /* 1 bytes spare */
    TEXT    corr_typ;
    COUNT   drvdel;         /* derivative interval * 100 */
    UTINY   drvord;     /* derivative order 0-4 */
    UTINY   curfit;            /* 1 byte spare */
    ULONG   wavmax;         /* start wavelength */
    ULONG   wavmin;         /* stop wavelength */
    UCOUNT sp1;
    UCOUNT  concsc;
    UCOUNT  mincyc;     /* Min cycle time */
    UCOUNT  wlpgsc;
    TEXT    date[8];    /* yy/mm/dd in ASCII */
    TEXT    time[12];   /* HH:MM:ss.ss ,ASCII, note trailing blank */
    TEXT    ident[72];  /* identification line */
    BITS    eflag;      /* krec extension flag */
    COUNT   ebytes;     /* number of bytes in extension */
    TEXT    *eptr;      /* pointer to extension in memory */
    TEXT    extra[256];
} KREC7;


