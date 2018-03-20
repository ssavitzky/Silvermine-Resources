/************************************************************************/
/* 			SLIM File Header Structure			*/
/************************************************************************/

typedef struct slimhdr
{
	USHORT	signature;	/* Overlay signature mark (0x4D5A)	*/
	USHORT	img_len;	/* Length of image mod 512		*/
	USHORT	img_size;	/* Image size in 512 byte pages		*/
	USHORT	num_rel;	/* Number of relocation table items	*/
	USHORT	hdr_size;	/* Size of header in paragraphs		*/
	USHORT	min_para;	/* Minimum number of paragraphs		*/
	USHORT	max_para;	/* Maximum number of paragraphs		*/
	USHORT	stack_seg;	/* Displacement of stack seg (ss reg)	*/
	USHORT	stack_off;	/* Offset of stack (i.e. size)		*/
	USHORT	checksum;	/* File checksum			*/
	USHORT	code_off;	/* Offset of code (ip reg)		*/
	USHORT	code_seg;	/* Displacement of code seg (cs reg)	*/
	USHORT	first_rel;	/* First relocation table item		*/
	USHORT	ovl_num;	/* DOS overlay number 			*/

} SLIMHDR;


/************************************************************************/
/*		SLIM Information Control Structure			*/
/************************************************************************/

typedef struct slim
{
	SLIMHDR	sl_slimhdr;		/* SLIM file header buffer	*/
	LONG	(*sl_slim_enter)();	/* SLIM entry function pointer	*/
	WORD	sl_cinit;		/* Perform cinit() if TRUE	*/
	CHAR	sl_slim_name[13];	/* SLIM filename string		*/
	CHAR	sl_filler;		/* Reserved CHAR		*/
	USHORT	sl_stack;		/* Start of stack segment addr	*/
	USHORT	sl_c_etext;		/* End of code segment address	*/
	ULONG	sl_code_off;		/* Offset of code area		*/
	ULONG	sl_code_size;		/* Size of code area		*/
	ULONG	sl_data_off;		/* Offset of data area		*/
	ULONG	sl_data_size;		/* Size of data area		*/
	CHAR	**sl_environ;		/* Root process environment	*/
	CHAR	*sl_code_ptr;		/* Code area pointer 		*/
	CHAR	*sl_data_ptr;		/* Data area pointer		*/
	WORD	sl_dataload;		/* Load data segment if TRUE	*/

} SLIM;


/************************************************************************/
/* 			SLIM Relocation Structure			*/
/************************************************************************/

typedef struct slimrel
{
	USHORT	offset;			/* Relocation address offset	*/
	USHORT	segment;		/* Relocation address segment	*/

} SLIMREL;

/************************************************************************/
/*			SLIM Return Codes				*/
/************************************************************************/

#define	SLIMOK		(0)		/* SLIM function success	*/
#define	SLIMERRFILOPEN	(-1)		/* File open error		*/
#define	SLIMERRFILREAD	(-2)		/* File read error		*/
#define	SLIMERRSIG	(-3)		/* Illegal SLIM signature 	*/

#define	SLIMBLKSIZE	32000		/* Default SLIM i/o block size	*/
#define	RELOCSIZE	64		/* Relocation item array size	*/

/************************************************************************/
/*			SLIM/SLM Information union.			*/
/************************************************************************/

typedef union ovlinfo
{
	SLMINFO	slm;			/* SLM information block	*/
	SLIM	slim;			/* SLIM information block	*/

} OVLINFO;


/************************************************************************/
/*			Overlay Manager Information Structure		*/
/************************************************************************/

typedef struct ovl
{
	CHAR	*pslmblk;		/* SLM block memory pointer	*/
	OVLINFO	info;			/* Overlay infomation block	*/
	WORD	handle;			/* Overlay handle number	*/
	UWORD	start_blk;		/* Starting block in memory	*/
	UWORD	num_blks;		/* Number of blocks required	*/
	CHAR	ovl_name[13];		/* Name of overlay		*/
	UBYTE	active;			/* TRUE if overlay is active	*/
	UWORD	ovl_mode;		/* Overlay mode byte		*/

} OVL;

#define	OVL_BLKSIZE	256		/* Size of an overlay block	*/

#define	OVLERRNOHAND	(-1)		/* No more handles available	*/
#define	OVLERRMEM	(-2)		/* Not enough blks to load ovl	*/
#define	OVLERRLOAD	(-3)		/* Overlay load error		*/
#define	OVLERRILLHAND	(-4)		/* Illegal handle number	*/
#define	OVLERRSCMEXEC	(-5)		/* Illegal SCM execution	*/
#define	OVLERREMSEXEC	(-6)		/* More than one active EMS ovl	*/
#define	OVLERRMODE	(-7)		/* Illegal overlay mode mask	*/
#define	OVLERRSTKFULL	(-8)		/* Overlay hndl stack is full	*/
#define	OVLERRSTKEMPTY	(-9)		/* Overlay hndl stack is empty	*/

#define	OVL_NOSWAP	0x0000		/* Non-swapable overlay		*/
#define	OVL_SWAP	0x0001		/* Swapable overlay		*/
#define	OVL_EMS		0x0002		/* Can run in expanded memory	*/
#define	OVL_SCM		0x0004		/* Share-code overlay		*/

#define	OVL_FREEBLK	0		/* Free overlay block		*/
#define	OVL_SWAPBLK	1		/* Swapable overlay block	*/
#define	OVL_NOSWAPBLK	2		/* Non-swapable overlay block	*/

#define	OVL_MODEAVAIL	0xFFFA		/* OVL_NOSWAP, OVL_SWAP, and	*/
					/* OVL_SCM currently supported	*/

/************************************************************************/
/*			Argument Checking				*/
/************************************************************************/

extern	int	ovl_load(char *, unsigned int);
/*extern	int	ovl_exec(int, long, long, long, long, long, long, long, long);
*/
extern	int	ovl_exec();
extern	int	ovl_free(int);
extern	OVL	*get_ovl_ptr(int);
extern	OVL	*get_ovl_ptr_name(char *);
