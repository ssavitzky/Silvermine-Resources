/************************************************************************/
/* 		Desk accesory union structures.				*/
/************************************************************************/

typedef struct cell
{
	LONG	cellmin1;	/* byte = 428; Ordinate min for cell 1	*/
	LONG	cellmax1;	/* byte = 432; Ordinate max for cell 1	*/
	LONG	cellmin2;	/* byte = 436; Ordinate min for cell 2	*/
	LONG	cellmax2;	/* byte = 440; Ordinate max for cell 2	*/
	LONG	cellmin3;	/* byte = 444; Ordinate min for cell 3	*/
	LONG	cellmax3;	/* byte = 448; Ordinate max for cell 3	*/
	LONG	cellmin4;	/* byte = 452; Ordinate min for cell 4	*/
	LONG	cellmax4;	/* byte = 456; Ordinate max for cell 4	*/
	LONG	cellmin5;	/* byte = 460; Ordinate min for cell 5	*/
	LONG	cellmax5;	/* byte = 464; Ordinate max for cell 5	*/
	LONG	cellmin6;	/* byte = 468; Ordinate min for cell 6	*/
	LONG	cellmax6;	/* byte = 472; Ordinate max for cell 6	*/
	WORD	cellmap;	/* byte = 476; Bit map of active cells	*/
};

typedef struct accrec
{
	WORD	samples;	/* byte = 428; # samples for multi-sampler */
 	WORD	stndards;	/* byte = 430; # of auto standards */
	WORD	volume;		/* byte = 432; Sipper volume (sample) */
	WORD	forward;	/* byte = 434; Sipper formard (air) */
	WORD	position;	/* byte = 436; sipper position of sample */
	WORD	pdelay;		/* byte = 438; Temperature delay time */
	WORD	temp1;		/* byte = 440; Temp of Temp controller */
	WORD	temp2;		/* byte = 442; Used for high performance */
	WORD	temp3;		/* byte = 444; Used for high performance */
	WORD	temp4;		/* byte = 446; Used for high performance */
	WORD	ramp12;		/* byte = 448; Used for high performance */
	WORD	ramp23;		/* byte = 450; Used for high performance */
	WORD	ramp34;		/* byte = 452; Used for high performance */
};

union ACC
{
	struct cell	cells;
	struct accrec	accsry;
};

/************************************************************************/
/* 				Spectrum Header				*/
/************************************************************************/
/*--------------------- removed to dmspec.h----------------------*/

/************************************************************************/
/*		Spectrum Locator Linked List Structure			*/
/************************************************************************/
/*-------------------------------------------------------AS
typedef struct specloc
{
	CHAR	filid[13];	/* Spectrum filid name			**
	BYTE	status;		/* Primarily for scan, plot, get, etc.	**
	UWORD	specid;		/* Spectrum id number			**
	LONG	size;		/* Size of KREC + DATA area, in bytes	**
	KREC	*kptr;		/* Pointer to start of KREC		**
	LONG	*sptr;		/* Pointer to start of DATA area	**
	CHAR	*p1;		/* Reserved pointer for EMS		**
	CHAR	*p2;		/* Reserved pointer for EMS		**
	CHAR	*p3;		/* Reserved pointer for EMS		**
	LONG	rt_npts;	/* Number of real-time points		**
	struct	specloc *link;	/* Pointer to next list node		**

} SPECLOC;
__________________________________________________________*/
/************************************************************************/
/* 		Data Region Memory Management Structure			*/
/************************************************************************/

typedef struct mempool
{
	LONG	total_bytes;	/* Maximum size of Memory Pool in bytes	*/
	LONG	free_bytes;	/* Number of free bytes in Memory Pool	*/
	CHAR	*strt_ptr;	/* Pointer to start of Memory Pool	*/
	CHAR	*free_ptr;	/* Pointer to next free byte		*/
	UWORD	next_specid;	/* Next assigned spectrum id number	*/

} MEMPOOL;


/* flag masks for krec MANIP variable */

/*#define FLG_LOG         1	/* set by LOG conversion */
/*#define FLG_DIFF        2	/* set by diff */
/*#define FLG_FLAT        4	/* set by flag */
/*#define FLG_MERGE       8 	/* set by merge */
#define FLG_ADD		32		/* 16 AS -set by arith */
#define	FLG_SUB		32		/* set by arith */
#define	FLG_MULT		32		/*AS-64	/* set by arith */
#define	FLG_DIV		32	/*128	/* set by arith */
/************************************************ AS ?????
#define	FLG_SMOOTH	256	/* set by smooth *
#define	FLG_ABEX	512	/* set by abex *
#define	FLG_NORM	1024	/* set by normalize *
#define	FLG_ENHANCE	2048	/* set by enhance *
#define	FLG_DERIV	4096	/* set by deriv *
#define FLG_MOD         8192	/* set if modified - shift,change,etc *
#define FLG_AVE		16384	/* set if average taken *
#define FLG_TAAT		0x00010000
#define FLG_PEAK		0x00020000

#define	INST_ERROR	-1	/* IEEE Communications Error	*/

/* stype definitions */

#define IR              0
#define UV              1
#define FL              2
#define INTERF          3
#define NEUT            9
/* Time drive = stype + 10 */
/* Rep scan = stype + 20 */
#define	VIS	2			/* Visible Lamp			*/

/* krec ORDTYPE definitions */

#define	TYP_ABS		1		/* Ordinate absorbance */
#define	TYP_TRA		2		/* Ordinate transmittance */
#define	TYP_D1		3		/* Ordinate derivative 1 */
#define	TYP_D2		4		/* Ordinate derivative 2 */
#define	TYP_D3		5		/* Ordinate derivative 3 */
#define	TYP_D4		6		/* Ordinate derivative 4 */
#define	TYP_EGY		7		/* Ordinate energy */
#define	TYP_RFL		8		/* Ordinate reflectance */
#define	TYP_ABRFL	9		/* Ordinate absolute reflectance */
#define	TYP_INT		10		/* Ordinate intensity */
#define	TYP_EM		11		/* Ordinate emittance */
#define	TYP_PAS		12		/* Ordinate Photoacoustic units */
#define	TYP_KM		13		/* Ordinate Kubelka-Munk units */

/* Base size of KREC */

#define	KREC_BASE	512		/* Base size of KREC */

/* Method type definitions used in runcmd() */

#define	SCAN	1
#define	TIMEDR	2
#define WLPG	4
#define	WAVE_2	8
#define	QUANT	16
#define	CONC	32
#define	KINS	64

/****************************************/
/* Bit pattern to test if a specific	*/
/* accessory is present or not.		*/
/****************************************/

#define	MANUAL		0
#define	SIPPER		1
#define	MULTISAMP	2
#define	CELLPROG	4
#define	TCA		8
#define HTA		16

/* Ouput mask definitions */

#define	PLOT		1		/* Output bit mask		*/
#define	TABLE		2		/* Output bit mask		*/
#define	GRID		4		/* Output bit mask		*/
#define	INCREMENT	8		/* Output bit mask		*/
#define	DISKCPY		16		/* Output bit mask		*/
#define	SAFE_ALT	32		/* Output Safe/Alterable	*/

#define	GRID_X	1			/* Grid direction X		*/
#define	GRID_Y	2			/* Grid direction Y		*/
#define	GRID_XY	3			/* Grid direction both X & Y	*/

/* Smoothing codes */

#define	SAVIT_GOLA	1		/* Savitzky-Golay Smooth 	*/
#define	TRIXIE		2		/* Trixie Smooth algorithm	*/

/* Derivative order codes */
/* Derivative order will be set to 1 thru 4 if a deriv was done with 	*/
/* PEMSB.  If a deriv was done on TRIXIE it will be 256.		*/

/* Display types */

#define	SERIAL		0
#define	OVERLAY		1
