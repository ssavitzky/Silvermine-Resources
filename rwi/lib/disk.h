/*** headerID = "disk.h 1.0 (c) 1989 S. Savitzky ***/

/*********************************************************************\
**
**	Disk -- 	Header file for X Disk utilities
**
**		Note the use of X in place of something specialized like
**		Idris (as in RD7) or iRMX
**
**	890619	SS	create 
**
\*********************************************************************/

/*********************************************************************\
**
** Variables
**
\*********************************************************************/

#ifndef MAXDRIVES
#define MAXDRIVES 16
#endif

global unsigned dMaxDrives;			/* max # of drives */

global int	dDrive;					/* current drive number (default B)	*/
global int	dDriveType[MAXDRIVES];	/* drive type flags */
global int  dMediaType[MAXDRIVES];	/* media type flags */
global int	dPhysDrive[MAXDRIVES];	/* physical drive numbers */
global Class dFileSys[MAXDRIVES];	/* file system */

global int  dErrors;				/* # of disk errors */
global int  dErrorsExpected;		/* don't complain about errors */

/*
** Drive type flags
*/
#define DRV_96TPI	1			/* drive is 96-TPI (AT) */
#define DRV_HIDENS	2			/* drive can do high density */
#define DRV_COMPAT	4			/* drive has CompatiCard driver */
#define DRV_SS		8			/* single-sided */
#define DRV_SD		16			/* single-density */
#define DRV_HARD	32			/* drive is a hard disk */

#define DRV_AT	(DRV_96TPI | DRV_HIDENS)
#define DRV_PC	0

/*
** Media types
*/
#define MT_96TPI	1			/* recorded at 96tpi */
#define MT_HIDENS	2			/* recorded at high density (15 sectors) */
#define MT_SS		8			/* single-sided */
#define MT_SD		16			/* single-density */

#define MT_DD		0
#define MT_MD		MT_96TPI
#define MT_HD		(MT_96TPI | MT_HIDENS)


/*********************************************************************\
**
** Exported routines: disk I/O
**
\*********************************************************************/

global void dInitDrive();		/* (drv, seclen, spt, dens)	*/
global void dCloseDisk();		/* clean up after initializing drives */
global int dRdSector();			/* (bufr, track, side, sector, count) */
global int dWrSector();			/* (bufr, track, side, sector, count) */


/*********************************************************************\
**
** XDir Class
**
**		This is the "external" file system class; the structure
**		itself is hidden.
**
\*********************************************************************/

global Class clXDir;

/*********************************************************************\
**
** Drive Viewer
**
\*********************************************************************/

typedef struct _DriveVrPart {
	int			cur;			/* index of current line */
	int			lim;			/* index of last line */
} DriveVrPart;

typedef struct _DriveVrRec {
	ObjectPart 	object;
	DriveVrPart	driveVr;
} DriveVrRec, *DriveVr;

global Class clDriveVr;


