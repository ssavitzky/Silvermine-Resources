/*** headerID = "convert.h 1.0 (c) 1988 S. Savitzky ***/

/*********************************************************************\
**
**	Convert -- 	Header file for Conversion
**
**	890622	SS	iRMX86 version
**	880509	SS	create 
**
\*********************************************************************/

/*********************************************************************\
**
** State
**
\*********************************************************************/

global unsigned	dInputDrive;		/* input drive + 1 */

global unsigned dOutputDrive;		/* drive + 1 */
global char     dOutputPath[256];	/* buffer for output pathname */
global Dir	    dOutputDir;			/* corresponding dir node */


/*********************************************************************\
**
** Routines
**
\*********************************************************************/

global int		dCopyDirs();	/* (n, init) 	copy tagged files under n */
global int		dCopyFile();	/* (n, init)	copy a single file		*/

global unsigned	dValidateDest();	/* ()		validate dest. dir.		*/

