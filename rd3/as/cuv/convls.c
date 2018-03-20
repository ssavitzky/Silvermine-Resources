/*
  convls.c - convert 3600 LS spectrum to 7000 format
  programmer : SATole
  date/rev.  : 83-07-28 R04

  Revised for rd3 90-06-13 AS

  This function converts a 3600 Luminescence spectrum into the
  format required on the 7000.

  The information stored in the krec of a 3600 spectrum is
  converted into the much expanded krec of the 7000.

  It thus takes care of the different data size and how
  various values are stored, e.g. mode, response, etc.

*/

#include <std.h>
#include "lsspec.h"        /* spectrum header for LS */
#include "okrec.h"

#define LSCALE ((long)(8388608/2000))
#define MODMSK 0x60         /* mask for mode value */
#define MONMSK 0x03         /* mask for monochromator value */
#define SPDMSK 0x70         /* mask for speed value */
#define RSPMSK 0x07         /* mask for response value */
#define TDMASK 0x0010       /* mask for tdrive flag in 'nflags' */
#define MODSHF 5            /* shift for mode value */
#define SPDSHF 4            /* shift for speed value */

LOCAL TEXT modlst[] = {"FPIX"};

VOID convls(OKREC *oldkrc,          /* pointer to old 3600 krec */
    KREC *newkrc)           /* pointer to new krec */
    {
	 float datascl;			/*scaling value for minmax */

    /* check for tdrive spectrum, i.e. set spectrum type to 12 else 2 */
    if ((swaps(oldkrc->nflags) & TDMASK) != 0)
        newkrc->stype = 12;         /* tdrive spectrum */
    else
        newkrc->stype = 2;

    /* set up luminescence scale */
    newkrc->scale = LSCALE;

	 /*guarantee correct sign for delta */
	 newkrc->ndel = abs(newkrc->ndel);
	 newkrc->ndel = (newkrc->istart > newkrc->ifin)?-newkrc->ndel:newkrc->ndel;

	 /* set up instno, values for ex and em monos and ordinate expansion */
    newkrc->instno -= 3000;     /* as instno was 3003,4,5 */
    newkrc->mono1 = swaps(oldkrc->ostrt) * 100;
    newkrc->mono2 = swaps(oldkrc->ofin) * 100;
    newkrc->ordexp = swaps(oldkrc->ordexp);

    /* set up the mode and the monochromator, where mode is the top 3 bits
    in the old krec mode and mono is the bottom 2 bits,
    where mode: F=000,P=101,I=110,X=111, mono: EX=0,EM=1,SY=2 */
    newkrc->mono = oldkrc->mode & MONMSK;
    newkrc->mode = modlst[((oldkrc->mode & MODMSK) >> MODSHF)];

    /* set up the scan speed and the response, where scan speed is bits
       4-6 of the old n580 amd response if bits 0-2 */
    newkrc->scnspd = ((oldkrc->n580 & SPDMSK) >> SPDSHF) * 10;
    newkrc->rspnse = oldkrc->n580 & RSPMSK;

    /* all variables are now setup in the new krec */

    return;
    }


