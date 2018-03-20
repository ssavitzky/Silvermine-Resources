/*********************************************************************\
**
**	dd.h -- 	Header file for Data Display program
**
**	870823 SS	split off from main program
**
\*********************************************************************/

#define VERSION "1.01"
#define COPYRIGHT "Copyright (c) 1987, 1988, 1989 Silvermine Resources, Inc."

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
**		both RD7 and its shipper program.
*/
#define XORPAT(buf, len) { int i; unsigned long x;	\
	for (i = 0, x = 65559; i < len; ++i, x *= 65559) buf[i] = x >> 7; }


