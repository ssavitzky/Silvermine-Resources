/*** headerID = "convert.h 1.0 (c) 1988 S. Savitzky ***/

/*********************************************************************\
**
**	Convert -- 	Header file for Conversion
**
**	880509	SS	create 
**
\*********************************************************************/


/*********************************************************************\
**
** State
**
\*********************************************************************/

global char	  dJcampBuf[256];	/* buffer for Jcamp header pathname */
global String dJcampHdr;

global char     dOutputPath[256];	/* buffer for output pathname */
global unsigned dOutputDrive;		/* drive + 1 */
global Dir	    dOutputDir;			/* corresponding dir node */

/*********************************************************************\
**
** Routines
**
\*********************************************************************/

global int		dCopyDirs();	/* (n, init) 	copy tagged files under n */
global int		dCopyFile();	/* (n, init)	copy a single file		*/

global int		dCopyDosDirs();	/* (n, init) 	copy tagged files under n */
global int		dCopyDosFile();	/* (n, init)	copy a single file		*/

global unsigned	dValidateDest();	/* ()		validate dest. dir.		*/
global unsigned dValidateDos();	 	/* (dir)	validate source drive	*/
global unsigned dValidateIdris();	/* (dir)	validate source drive	*/

