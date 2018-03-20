/*
  oldkrc.h - 3600 krec header
  programmer : A.Savitzky, S.Tole
  date/rev.  : 83-07-04 R02
*/

typedef struct
    {
    TEXT  name[8];
    COUNT npts;
    COUNT nstrt;
    COUNT nfin;
    COUNT ndel;
    COUNT nflags;
    COUNT miny;
    COUNT maxy;
    COUNT instno;
    UTINY nsmth;
    UTINY naccs;
    UTINY mode;
    UTINY n580;
    float absex;
    COUNT tdint;
    COUNT ordexp;
    COUNT ostrt;
    COUNT ofin;
    TEXT ident[64];
    TEXT xx[32];
    }  OKREC;
