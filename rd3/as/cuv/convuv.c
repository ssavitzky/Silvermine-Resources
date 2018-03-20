/*
  convuv.c - convert 3600 UV spectrum to 7000 format
  programmer : SATole
  date/rev.  : 83-07-28 R04

  Revised for rd3 90-06-13 AS

  This function converts a 3600 Ultraviolet spectrum into the
  format required on the 7000.

  The information stored in the krec of a 3600 spectrum is
  converted into the much expanded krec of the 7000.

  It thus takes care of the different data size and how
  various values are stored, e.g. mode, filter, etc.

*/

#include <std.h>
#include "uvspec.h"        /* spectrum header for UV */
#include "okrec.h"

#define SLTSHF 8            /* shift for slit value */
#define RSPSHF 12           /* shift for response value */
#define SPDSHF 8            /* shift for speed value */
#define ORDSHF 4            /* shift for ordinate value */
#define DELMSK 0x000F       /* mask for delta wavelength value */
#define RSPMSK 0xF000       /* mask for response value */
#define SPDMSK 0x0F00       /* mask for scan speed value */
#define ORDMSK 0x00F0       /* mask for ordinate mode value */
#define TDMASK 0x0010       /* mask for tdrive flag in 'nflags' */
#define TSCALE 4194304
#define ASCALE 8388608
#define TFACTOR (TSCALE/20000L)
#define AFACTOR (ASCALE/30000L)

LOCAL COUNT oldspd[2][11] = {
    {0, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11},
    {0, 19, 38, 75, 150, 300, 600, 1200, 2400, 4800, 9600}
    };

LOCAL UTINY oldrsp[2][6] = {
    {4, 5, 6, 7, 8, 9},
    {2, 5, 10, 20, 50, 100}
    };

LOCAL COUNT olddel[2][8] = {
    {0, 1, 2, 3, 4, 5, 6, 7},
    {0, 1, 2, 3, 4, 6, 8, 10}
    };

VOID convuv(OKREC *oldkrc,          /* pointer to old 3600 krec */
    KREC7 *newkrc)           /* pointer to new krec */
    {
    COUNT tmpspd;           /* temporary variable for scan speed */
    UTINY tmprsp;           /* temporary variable for response */
    COUNT tmpdel;           /* temporary variable for delta wavelength */
	 int i;


    /* now set up the variables which are different for ordinary uv
       spectra and timedrive spectra, i.e. stype, start, fin, del */
    if ((newkrc->flags & TDMASK) != 0)
        {
        /* tdrive spectrum */
        newkrc->stype = 11;
        newkrc->istart = 0;
        newkrc->ifin = (newkrc->npts - 1) * swaps(oldkrc->ndel) * 100;
        newkrc->ndel = abs(swaps(oldkrc->ndel) * 100);

        /* set up the monochromator position */
        newkrc->mono1 = swaps(oldkrc->nstrt) * 100;
        }
    else
        {
        /* ordinary spectrum */
        newkrc->stype = 1;
        /* set up data interval for UV - the old krec value must be negated */
        newkrc->ndel = -swaps(oldkrc->ndel);
        }


    /* set up instrument specific values in new uv header */
    /* set up scan speed - previously encoded 0-11, but now store
    as actual scan speed * 10 */
    tmpspd = (swaps(oldkrc->ordexp) & SPDMSK) >> SPDSHF;
    for (i = 0; tmpspd != oldspd[0][i]; i++);
    newkrc->scnspd = oldspd[1][i];

    /* set up response - previously encoded 4-9, now store * 10
       to get range of 2 - 100 */
    tmprsp = (swaps(oldkrc->ordexp) & RSPMSK) >> RSPSHF;
    for (i = 0; tmprsp != oldrsp[0][i]; i++);
    newkrc->rspnse = oldrsp[1][i];

    /* set up uvslit */
    newkrc->uvslit = (oldkrc->mode << SLTSHF) + oldkrc->n580;

    /* set up ordinate mode */
    newkrc->ordmode = (swaps(oldkrc->ordexp) & ORDMSK) >> ORDSHF;

    /* set up delta wavelength */
    tmpdel = (swaps(oldkrc->ordexp) & DELMSK);
    for (i = 0; tmpdel != olddel[0][i]; i++);
    newkrc->drvdel = olddel[1][i];

    /* set remaining instrument values to zero until defaults are decided */
    newkrc->nirsens = 0;
    newkrc->uvgain = 0;
    newkrc->detswitch = 0;
    newkrc->cellno = 0;

    /* all variables are now set up in the new krec */

	 return;
    }


