/************************************************************************/
/*				Data Types				*/
/************************************************************************/

#define	CHAR	char			/* Use CHAR for ASCII items	*/
#define	RCHAR	register char
#define	BYTE	char			/* Use BYTE for numeric items	*/
#define	TEXT	char			/* Compatibility		*/
#define	UBYTE	unsigned char
#define	HCHAR	char huge
#define	WORD	int
#define	UWORD	unsigned int
#define	SHORT	short int
#define	USHORT	unsigned short int
#define	RWORD	register int
#define	RUWORD	register unsigned int
#define	HWORD	int huge
#define	BITS	unsigned int
#define	LONG	long int
#define	ULONG	unsigned long
#define	HLONG	long huge
#define	HULONG	unsigned long huge
#define	FLOAT	float
#define	DOUBLE	double
#define	EXTERN	extern
#define	MLOCAL	static
#define	LOCAL	static
#define	STATIC	static
#define	GLOBAL	/**/
#define	VOID	void
#define	FAR	far
#define	NEAR	near
#define	HUGE	huge
#define	PASCAL	pascal
#define	DEFFUN	/**/

typedef	unsigned bool;			/* For View			*/

/************************************************************************/
/*				Everybody's Favorites			*/
/************************************************************************/

#define	YES	1
#define	NO	0
#define	TRUE	1
#define	FALSE	0

#ifndef NULL
#define NULL	0
#endif

#ifndef	EOF
#define	EOF	(-1)
#endif

#define NULLPTR (char *)0
#define	FOREVER	while(1)

/* Use MAX, MIN, and ABS rather that max, min, and abs for new code.	*/

#define max(a,b)	(((a) > (b)) ? (a) : (b))
#define min(a,b)	(((a) < (b)) ? (a) : (b))
#define abs(a)		(((a) < 0) ? -(a) : (a))

#define MAX(a,b)	(((a) > (b)) ? (a) : (b))
#define MIN(a,b)	(((a) < (b)) ? (a) : (b))
#define ABS(a)		(((a) < 0) ? -(a) : (a))
