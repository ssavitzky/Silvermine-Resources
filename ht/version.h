/*********************************************************************\
**
**	rd3.h -- 	Version header file	and shipper info
**
**	870823 SS	split off from main program
**
\*********************************************************************/

#ifndef VERSION
#define VERSION "0.01"
#define PROGRAM "HyperTree"
#define ENV_VAR "HT"
#endif

#define COPYRIGHT "Copyright (c) 1990 HyperSpace Express."

#define SERNUM		"<SERIAL NUMBER>"
#define CUSTNAME	\
"<------SILVERMINE RESOURCES (WORKING VERSION) SILVERMINE RESOURCES------>"

#define PFXCHAR '<'		/* the character that starts SERNUM and CUSTNAME */
#define MAXPAT 80

#define global extern
#define IFDEBUG	if (debugf) {
#define DEBENDIF }
global int debugf;

/*
** Macro to build xor pattern in buffer
**		Wants to be semirandom, and has to be the same in
**		both the product and its shipper program.
*/
#define XORPAT(buf, len) { int i; unsigned long x;	\
	for (i = 0, x = 65559; i < len; ++i, x *= 65559) buf[i] = x >> 7; }


