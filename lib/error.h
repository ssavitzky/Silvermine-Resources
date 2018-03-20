/*** Header: "ERROR.H 1.0 copyright 1987 S. Savitzky" ***/

/*********************************************************************\
**
**	ERROR.H -- error handler header file
**	
**	870106 SS	create PC version.
**
\*********************************************************************/


/*********************************************************************\
**
**	C O N S T A N T S
**	
\*********************************************************************/

#define INFORMATIONAL	0		/* not an error at all, just a message */
#define WARNING			1		/* warning; may be disregarded			*/
#define ERROR				2		/* error											*/
#define FATAL				3		/* fatal error: exit immediately			*/
#define INTERNAL			4		/* internal (fatal) error					*/
#define ASSERTION			5		/* assertion failure (fatal)				*/

/*********************************************************************\
**
**	E R R O R   H A N D L I N G
**	
\*********************************************************************/

extern int (*errorHook)();	/* (sev, msg) -> 1 if error handled				*/

extern void error();			/* (sev, msg)	 output an error message		*/
extern void errorPrintf();	/*	(sev, fmt, ...) printf an error message	*/

#ifndef NOASSERT
#define ASSERT(a, msg) if (!(a)) error(ASSERTION, msg);
#else
#define ASSERT(a, msg) /* empty */
#endif



