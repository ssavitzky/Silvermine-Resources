/************************************************************************/
/* 				Spectrum Header				*/
/************************************************************************/

/*--------------- removed from dmspec.h------------------*/

typedef struct		/* Total length = 512 bytes */
{
	/****************************************************************/
	/* 		Spectroscopy specific information.		*/
	/****************************************************************/

	CHAR    spname[14];	/* byte = 0; spectrum name with ext. NULL term. */
	UBYTE   stype;		/* byte = 14; spectrum type, 0=IR,1=UV,2=LS; TD=10+ */
	UBYTE   dtype;		/* byte = 15; data type = 2, 3, or 4 bytes */
	UBYTE	ordtype;	/* byte = 16; ordinate type see below */
	UBYTE	flat;		/* byte = 17; order of flat command */
	WORD	naccs;		/* byte = 18; number of accumulations (averages) */
	LONG	ndel;		/* byte = 20; interval for stored data * 100 (nm or secs) */
	LONG    npts;		/* byte = 24; number of data points in spectrum */
	LONG    istart;		/* byte = 28; wave start * 100 (nm or secs) */
	LONG    ifin;		/* byte = 32; wave finish * 100 (nm or secs) */
	LONG    miny;		/* byte = 36; ordinate minimum */
	LONG    maxy;		/* byte = 40; ordinate maximum */
	DOUBLE  scale;		/* byte = 44; data scale */
	DOUBLE  absex;		/* byte = 52; absorbance expansion factor */
	DOUBLE	absex_offset;	/* byte = 60; absex factor offset */
	CHAR	ordid[4];	/* byte = 68; ordinate label */
	CHAR	abscid[6];	/* byte = 72; abscissa label */
	CHAR    date[8];	/* byte = 78; yy/mm/dd in ASCII */
	CHAR    time[12];	/* byte = 86; HH:MM:ss.ss ,ASCII, note trailing blank */
	CHAR	m_date[8];	/* byte = 98; Modification date */
	CHAR	m_time[12];	/* byte = 106; Modification time */
	CHAR	analyst[21];	/* byte = 118; Analyst ident. NULL terminated */
	CHAR    info[51];	/* byte = 139; information line NULL terminated */
	UWORD	manip;		/* byte = 190; manipulation flags (see below) */
	DOUBLE	diffactor;	/* byte = 192; diff factor */
	DOUBLE	dffct_offset;	/* byte = 200; diff factor offset */
	DOUBLE	enhwdth;	/* byte = 208; enhance width */
	DOUBLE	enhfact;	/* byte = 216; enhance factor */
	DOUBLE	onedef;		/* byte = 224; definition of 1 unit (proposed) */
	DOUBLE	realdel;	/* byte = 232; proposed floating delta */
	LONG	norm_wave;	/* byte = 240; wave length * 100 for normal */
	LONG	norm_offset;	/* byte = 244; normalization offset */
	DOUBLE	norm_factor;	/* byte = 248; normalization factor */
	CHAR	*eptr;		/* byte = 256; Pointer to extension in memory */
				/*             Used only if KREC > 512 bytes  */
	WORD	ebytes;		/* byte = 260; number of bytes in extension */
	BYTE	smth_code;	/* byte = 262; smoothing code (see below) */
	BYTE	drvord;		/* byte = 263; derivative order (see below) */
	DOUBLE	smth_width;	/* byte = 264; Smooth width 1-7 */
	DOUBLE	drv_width;	/* byte = 272; Derivative width * 100 */
	UBYTE	gform;		/* byte = 280; globally defined header format */
	UBYTE	lform;		/* byte = 281; locally defined header format */
	ULONG cantmanip;	/* byte 282; can't do manipulation flags(see below) */
	CHAR	specfill[4];	/* byte = 286; 4 extra bytes */

	/****************************************************************/
	/*	             UV specific information			*/
	/****************************************************************/

	UWORD	instno;		/* byte = 290; instrument model number */
	ULONG	fixwave;	/* byte = 292; fixed wave * 100nm */
	CHAR	subtech;	/* byte = 296; sub-technique to be defined */
	CHAR	method;		/* byte = 297; method (see below) */
	UWORD	cycles;		/* byte = 298; number of cycle in total */
	UWORD	totcycs;	/* byte = 300; total number of cycles   */
	UBYTE	response;	/* byte = 302; response seconds * 10 */
	UBYTE	integ;		/* byte = 303; integration time  in sec */
	LONG	cyctime;	/* byte = 304; cycle time (sec * 10)    */
	UWORD	scnspd;		/* byte = 308; scanning speed * 10 nm/sec (0 for diode array)*/
	UWORD	slit;		/* byte = 310; slit size * 100nm */
	ULONG	accessory;	/* byte = 312; accessories (see below) */
	UBYTE	lamps;		/* byte = 316; Lamps ON/OFF, 1-UV, 2-VIS */
	CHAR	uvfiller[1];	/* byte = 317; filler space for discipline */

	/****************************************************************/
	/*          Instrument specific information (Lambda 4)		*/
	/****************************************************************/
					     
	CHAR	corr_typ;	/* byte = 318; correction type (p) partial, etc. */
	CHAR	drv_ord;	/* byte = 319; derivative order 1-4 (real-time) */
	UWORD	drv_pts;	/* byte = 320; derivative lambda (real-time) */
	UWORD	idno;		/* byte = 322; id number */
	LONG	minutes;	/* byte = 324; cyc time or td time in mins * 100 */
	LONG	delminute;	/* byte = 328; td delta time in minutes * 100 */
	LONG	baselo;		/* byte = 332; baseline start wave * 100 */
	LONG	basehi;		/* byte = 336; baseline stop wave * 100  */
	UBYTE	ser_over;	/* byte = 340; serial = 0; overlay = 1   */
	UBYTE	output;		/* byte = 341; See note below            */
	UBYTE	grid;		/* byte = 342; Grid pattern (x, y, xy)   */
	UBYTE	timeunits;	/* byte = 343; time sec=0, min=1, auto=2  */
	UWORD	instdel;	/* byte = 344; delta for instrument data * 100 */
	UWORD	dispdel;	/* byte = 346; delta for displayed data * 100 */
	LONG	ordmax;		/* byte = 348; user ordinate max	   */
	LONG	ordmin;		/* byte = 352; user oridnate min	   */
	ULONG	wave1;		/* byte = 356; 1st wavelength * 100 (WLPG) */
	ULONG	wave2;		/* byte = 360  2nd     "      "  "     "   */
	ULONG	wave3;		/* byte = 364  3rd     "      "  "     "   */
	ULONG	wave4;		/* byte = 368  4th     "      "  "     "   */
	ULONG	wave5;		/* byte = 372  5th     "      "  "     "   */
	ULONG	wave6;		/* byte = 376  6th     "      "  "     "   */
	ULONG	wave7;		/* byte = 380  7th     "      "  "     "   */
	ULONG   wave8;		/* byte = 384  8th     "      "  "     "   */
	ULONG	fact1;		/* byte = 388; Factor for 1st wavelength   */
	ULONG	fact2;		/* byte = 392;    "    "  2nd     "        */
	ULONG	fact3;		/* byte = 396;    "    "  3rd     "        */
	ULONG	fact4;		/* byte = 400;    "    "  4th     "        */
	ULONG	fact5;		/* byte = 404;    "    "  5th     "        */
	ULONG	fact6;		/* byte = 408;    "    "  6th     "        */
	ULONG	fact7;		/* byte = 412;    "    "  7th     "        */
	ULONG	fact8;		/* byte = 416;    "    "  8th     "        */
	CHAR	ext_name[8];	/* byte = 420; name of external file       */
	union	ACC	acc;	/* byte = 428; see above definition */
	CHAR	lam4fill[34];	/* byte = 478; extra space for inst. spec. */
}KREC;


