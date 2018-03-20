/*
  convir.c - convert 3600 IR spectrum to 7000 format
  programmer : SATole
  date/rev.  : 83-07-28 R04

  This function converts a 3600 Infrared spectrum into the
  format required on the 7000.

  The information stored in the krec of a 3600 spectrum is
  converted into the much expanded krec of the 7000.

  It thus takes care of the different data size and how
  various values are stored, e.g. mode, filter, etc.

*/
/* revised for rd3 90-06-13 AS*/
	
#include <std.h>
#include "irspec.h"        /* spectrum header for IR */
#include "okrec.h"

#define FLTMSK 0x03         /* mask for filter */
#define MDSHF9 2            /* shift for 983 mode value*/
#define MDSHF7 2            /* shift for 780 mode value */
#define TDMASK 0x0010       /* mask for tdrive flag in 'nflags' */
#define TSCALE 4194304L
#define ASCALE 8388608L
#define TFACTOR (TSCALE/20000L)
#define AFACTOR (ASCALE/30000L)

VOID convir(OKREC *oldkrc,        /* pointer to old 3600 krec */
		    KREC *newkrc)           /* pointer to new krec */
    {
    FLOAT datscl;           /* scaling value for data */
    FAST COUNT ipts;        /* number of spectrum points */

    /* now set up the variables which are different for ordinary ir
       spectra and timedrive spectra, i.e. stype, start, fin, del */
    if ((newkrc->flags & TDMASK) != 0)
        {
        /* tdrive spectrum */
        newkrc->stype = 10;
        newkrc->istart = 0;
        newkrc->ifin = (newkrc->npts - 1) * swaps(oldkrc->ndel) * 100;
        newkrc->ndel = swaps(oldkrc->ndel) * 100;

        /* set up the monochromator position */
        newkrc->mono1 = swaps(oldkrc->nstrt) * 100L;
        }
    else
        /* ordinary spectrum */
        newkrc->stype = 0;

    /* variables are now set up which are common to 983 and 780 -
       now do the instrument specific ones */
    if (newkrc->instno == 983)
        {
        /* set up filter and mode */
        /* mode is the top 6 bits of n580 and filter (the bottom 2 bits) - 1
           this being the old 'mult' */
        newkrc->filter = (oldkrc->n580 & FLTMSK) + 1;
        newkrc->mode = oldkrc->n580 >> MDSHF9;

        /* set up beam mode, can be single beam, double beam or emission */
        switch(oldkrc->mode)
            {
            case 255:
                /* single beam transmittance */
                newkrc->beamod = 3;
                break;
            case 254:
                /* double beam transmittance */
                newkrc->beamod = 1;
                break;
            case 253:
                /* emission */
                newkrc->beamod = 5;
                break;
            }
        }
    else if (newkrc->instno == 780)
        {
        /* set up filter, mode and beam mode */
        /* mode is the top 6 bits of the old n580 and filter is the bottom
           2 bits, previously called 'mult'
           for the filter, the values stored as 0,1,2,3 represent 1,2,4,16
           which is the same as in 3600 krec.
           for the mode, the values stored as 1,2,3,4 represent 0-3 from
           the instrument, which is also the same as before.
           for the beam mode, 0=DBT and 1=SBT which is as before. */
        newkrc->filter = (oldkrc->n580 & FLTMSK);
        newkrc->mode = (oldkrc->n580 >> MDSHF7);
        newkrc->beamod = oldkrc->mode;
        }

    /* all variables are now setup in the new krec */

	 return;
    }
