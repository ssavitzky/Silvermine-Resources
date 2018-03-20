/***
*dostypes.h - defines DOS packed date and time types
*
*   Copyright (c) 1987, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*   This file defines the DOS packed date and time types.
*
*******************************************************************************/


#define MASK4   0xf     /* 4 bit mask */
#define MASK5   0x1f    /* 5 bit mask */
#define MASK6   0x3f    /* 6 bit mask */
#define MASK7   0x7f    /* 7 bit mask */

#define DAYLOC      0   /* day value starts in bit 0 */
#define MONTHLOC    5   /* month value starts in bit 5 */
#define YEARLOC     9   /* year value starts in bit 9 */

#define SECLOC      0   /* seconds value starts in bit 0 */
#define MINLOC      5   /* minutes value starts in bit 5 */
#define HOURLOC     11  /* hours value starts in bit 11 */

#define SET_DOS_DAY(dword, xday)    dword |= (((xday) & MASK5) << DAYLOC)
#define SET_DOS_MONTH(dword, xmon)  dword |= (((xmon) & MASK4) << MONTHLOC)
#define SET_DOS_YEAR(dword, xyr)    dword |= (((xyr) & MASK7) << YEARLOC)

#define SET_DOS_HOUR(tword, xhr)    tword |= (((xhr) & MASK5) << HOURLOC)
#define SET_DOS_MIN(tword, xmin)    tword |= (((xmin) & MASK6) << MINLOC)
#define SET_DOS_SEC(tword, xsec)    tword |= (((xsec) & MASK5) << SECLOC)

#define DOS_DAY(dword)      (((dword) >> DAYLOC) & MASK5)
#define DOS_MONTH(dword)    (((dword) >> MONTHLOC) & MASK4)
#define DOS_YEAR(dword)     (((dword) >> YEARLOC) & MASK7)

#define DOS_HOUR(tword) (((tword) >> HOURLOC) & MASK5)
#define DOS_MIN(tword)  (((tword) >> MINLOC) & MASK6)
#define DOS_SEC(tword)  (((tword) >> SECLOC) & MASK5)

extern time_t _dtoxtime();
#define XTIME(d,t) _dtoxtime(DOS_YEAR(d),DOS_MONTH(d),DOS_DAY(d),DOS_HOUR(t),\
     DOS_MIN(t),DOS_SEC(t)*2)
