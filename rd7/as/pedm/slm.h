/*	slm.h - header for SLM usage (shared-code and overlays)
	Last updated    5/26/87   [RNoble]  */


#define EXEHDR struct exehdr
EXEHDR {
	unsigned signatur;		/*	.EXE signature, should be 0x5A4D  */
	unsigned imglenlo;		/*	length of image mod 512  */
	unsigned filsizhi;		/*	size of file in 512-byte increments  */
	unsigned relitems;		/*	number of relocation items  */
	unsigned hdrsize;		/*	size of header in 16-byte paragraphs  */
	unsigned minalloc;		/*	minimum additional paragraphs required  */
	unsigned maxalloc;		/*	maximum additional paragraphs required  */
	unsigned dispstk;		/*	displacement in paragraphs of stack seg  */
	unsigned spstart;		/*	initial value for SP upon startup  */
	unsigned checksum;		/*	checksum for entire file  */
	unsigned ipstart;		/*	initial value for IP upon startup  */
	unsigned dispcode;		/*	displacement in paragraphs of code seg  */
	unsigned disprel;		/*	displacement in bytes 1st relocation item  */
	unsigned ovlnum;		/*	overlay number  */
	};


/*	the XCINFO structure is not for general use  */

#define XCINFO struct xcinfo
XCINFO {
	unsigned	*xcbeg;
	unsigned	*xcend;
	unsigned	xcsiz;
	unsigned	*bcbeg;
	unsigned	*bcptr;
	unsigned	*stkend;
	unsigned	*cursp;
	};


#define SLMINFO struct slminfo
SLMINFO {
	int		(far *slmentry) ();	/*	ptr to overlay entry function  */
	int		(far *slmexit) ();	/*	ptr to slm exit function  */
	long	slmsiz;				/*	number of bytes slm requires  */
	EXEHDR	eh;					/*	EXEHDR for SLM  */
	XCINFO	xc;					/*	intercall stack information  */
	};


#define MAXPOOLS	4			/*	maximum number of SLM memory pools  */

/*	the following are memory pool types  */

#define	PT_NONE		0			/*	unused entry  */
#define	PT_USER		1			/*	user-allocated memory pool  */
#define	PT_SYS		2			/*	system memory pool (halloc'd)  */
#define PT_EMS		3			/*	EMS memory pool  */


typedef int		MHANDLE;		/*	slm memory block handle  */
typedef int		SHANDLE;		/*	autoload slm handle  */


/*	function prototypes  */


int em_alloc(int);

int em_exist();

unsigned int em_frame();

int em_free(int);

int em_map(int, int, int);

int em_pages(int *);

int em_stat();

int load_slm(char *, char **, SLMINFO *);

SLMINFO *slm_lock(SHANDLE);

MHANDLE slm_mem(int, void *, long);

SHANDLE slm_reg(char *, MHANDLE);

int slm_setup(int);

void slm_term();

void slm_ulock(SHANDLE);

void slm_uload(SHANDLE);

