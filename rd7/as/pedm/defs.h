/************************************************************************/
/* This file contains the basic definitions needed to compile		*/
/* the Benchtop files.							*/
/************************************************************************/

#define	NUM_WIND	6		/* Maximum number of windows	*/
#define	NUM_BUTMSG	15		/* Maximum number of button msg	*/

#define	BEG_UPDATE	1		/* Grab control of screen	*/
#define	END_UPDATE	0		/* Release control of screen	*/
#define	BEG_MCTRL	3		/* Grab control of mouse	*/
#define	END_MCTRL	2		/* Release control of mouse	*/

#define	CR		0x0D		/* Enter key			*/
#define	PAGE_DOWN	0x5100		/* Page down key		*/

#define	DESK		0		/* Desktop window handle num	*/
#define	EXTERNAL	1		/* External Item flag		*/
#define	INTERNAL	2		/* Internal Item flag		*/
#define DIRECTORY	16		/* FOA attribute directory bit	*/

#define FUNC_LO		0x3B		/* First function key		*/
#define FUNC_HI		0x71		/* Last function key		*/
#define	FORMSIZE	256		/* Size of format buffer	*/
#define	LINESIZE	82		/* Size of line input buffer	*/
#define	VNAME		8		/* Length of variable name	*/

#define	ICON_W		80		/* Width in pixels for icon	*/
#define	ICON_H		46		/* Height in pixels for icon	*/

#define	FIRST_FIT	0		/* DOS mem alloc first fit	*/
#define	BEST_FIT	1		/* DOS mem alloc best fit	*/
#define	LAST_FIT	2		/* DOS mem alloc last fit	*/

#define	BUT_NONE	0		/* Error alert box no button	*/
#define	NOTE		1		/* Error alert box NOTE icon	*/
#define	WAIT		2		/* Error alert box WAIT icon	*/
#define	STOP		3		/* Error alert box STOP icon	*/
#define	CANCEL		1		/* Error box CANCEL button	*/
#define	CONTINUE	2		/* Error box CONTINUE button	*/
#define	ABORT		3		/* Error box ABORT button	*/
#define	OK		4		/* Error box OK button		*/
#define	CORRECT		5		/* Error box CORRECT button	*/
#define	INFORMATION	6		/* Error box INFORMATION button	*/
#define	RESTORE		7		/* Error box RESTORE button	*/
#define	RENAME		8		/* Error box RENAME button	*/
#define	OVERWRITE	9		/* Error box OVERWRITE button	*/
#define	IYES		10		/* Inquire box YES button	*/
#define	INO		11		/* Inquire box NO button	*/
#define	RETRY		12		/* Error box RETRY button	*/

#define	SHIFT_ON	0x0003		/* Shift keys on		*/
#define	CNTRL_ON	0x0004		/* Control key on		*/
#define	ALT_ON		0x0008		/* Alt key on			*/

#define	MU_ENTER	0x0000		/* Mouse rectangle event enter	*/
#define	MU_EXIT		0x0001		/* Mouse rectangle event exit	*/

#define	MEN_FULL	0		/* Full menu config file	*/
#define	MEN_REPLACE	1		/* Partial menu config file	*/
#define	POS_FREE	0xFFFF		/* Free MOA slot value		*/
#define	MEN_LINECHAR	0x13		/* Menu item separator char	*/
#define	ALL_OVERLAYS	0x8000		/* Code to free all overlays	*/

#define	RSC_LOAD	1		/* RSC.SLM resource load code	*/
#define	RSC_FREE	2		/* RSC.SLM resource free code	*/

#define	VWMAXCHAN	15		/* Maximum number of View chan	*/
#define	NUMMENUTITLESTR	12		/* Number of menu title strings	*/

/************************************************************************/
/* Defines for mouse cursor types.					*/
/************************************************************************/

#define	ARROW		0
#define	TEXT_CURSOR	1
#define	HOUR_GLASS	2
#define	POINT_FINGER	3
#define	FLAT_HAND	4
#define	THIN_CROSS	5
#define	THICK_CROSS	6
#define	OUTLINE_CROSS	7

#define	MOUSE_ON	257
#define	MOUSE_OFF	256

#define	ACT_PO		1		/* Primary Object Action obj	*/
#define	ACT_WIND	2		/* Window action object		*/
#define	COPY_FAIL	(-1)		/* Copy fail status code	*/
#define	COPY_ABORT	0		/* Copy user abort status code	*/
#define	COPY_SUCCESS	1		/* Copy success status code	*/
#define	TYP_SAVE	0		/* SAVEDS.SLM save code		*/
#define	TYP_SAVEAS	1		/* SAVEDS.SLM save as code	*/

/************************************************************************/
/* 		Primary Object / Window Type Codes.			*/
/************************************************************************/

#define	TYP_DEFMASK	0x00FF		/* Mask for code		*/
#define	TYP_NOWIND	0x0000		/* No window			*/
#define	TYP_DISK	0x0001		/* Disk/File Window		*/
#define	TYP_VIEW	0x0002		/* View Window			*/
#define	TYP_PRINT	0x0003		/* Print Window			*/
#define	TYP_DATAREG	0x0004		/* Data Region Window		*/
#define	TYP_TRASH	0x0005		/* Trash Can Window		*/
#define	TYP_INST	0x0006		/* Instrument Window		*/
#define	TYP_SCAN	0x0007		/* Scan Window			*/
#define	TYP_TIMEDR	0x0008		/* Time Drive Window      	*/
#define	TYP_WLPG	0x0009		/* WLPG Window			*/
#define	TYP_TW		0x000A		/* Two WLPG Window		*/
#define	TYP_CONC	0x000B		/* Concentration Window		*/
#define	TYP_QUANT	0x000C		/* Quant Window			*/
#define	TYP_KINS	0x000D		/* Kinetics Window		*/
#define	TYP_ACCSRY	0x000E		/* Accessory Window (Lambda 4)	*/
#define	TYP_ACC  	0x0010		/* Accessory Icon Window	*/
#define	TYP_PLOT	0x0011		/* Plot Window			*/
#define	TYP_PEAK	0x0012		/* Peak Window			*/
#define	TYP_TYPE	0x0013		/* Typed command window		*/
#define	TYP_RTVIEW	0x0014		/* Real-time View Window	*/
#define	TYP_MACRO	0x0015		/* Macro language window	*/
#define	TYP_DISKSUBDIR	0x0016		/* Disk subdirectory.		*/
#define	TYP_NOTFOUND	0x0017		/* Not found.			*/
#define	TYP_TABLE	0x0019		/* Table Window			*/
#define	TYP_SCTABLE	0x0020		/* Scan Table Window		*/
#define	TYP_TDTABLE	0x0021		/* Time Drive Table Window	*/
#define	TYP_WPTABLE	0x0022		/* Wlpg Table Window		*/
#define TYP_CWPTABLE	0x0023		/* Cell Wlpg Table Window	*/
#define TYP_TWTABLE	0x0024		/* Two Wave Table Window	*/
#define TYP_CTWTABLE	0x0025		/* Two Wave Cell Table Window	*/
#define TYP_CNTABLE	0x0026		/* Two Wave Cell Table Window	*/
#define	TYP_CTDTABLE	0x0027		/* Time Drive Cell Table Window	*/

/************************************************************************/
/* Icon Display Options.						*/
/************************************************************************/

#define	SORT_TYPE	0		/* Sort FOA by file type	*/
#define	SORT_SIZE	1		/* Sort FOA by file size	*/
#define	SORT_DATE	2		/* Sort FOA by file date	*/
#define	SORT_NAME	3		/* Sort FOA by file name	*/

#define	SHOW_ICON	0		/* Display DOT as icons		*/
#define	SHOW_TEXT	1		/* Display DOT as text		*/

#define	SHOW_GRAPH	0		/* Display data as a graph	*/
#define	SHOW_TABLE	1		/* Display data as a table	*/

#define	PRI_OBJ		1		/* AOL Primary Object flag	*/
#define	SEC_OBJ		2		/* AOL Secondary Object flag	*/
#define	DUMMY_OBJ	3		/* AOL dummy object flag	*/

/************************************************************************/
/*		Definition of return types				*/
/************************************************************************/

#define RET_SUCCESS	1		/* successful return		*/
#define RET_ERROR	2		/* error return			*/
#define RET_NOTDONE	3		/* not done with input		*/
#define RET_DONE	4		/* done with input, execute	*/
#define RET_FAIL	5		/* fatal error return		*/
#define RET_CHANGE	6		/* input for current field	*/
#define RET_CDONE	7		/* DONE but with input for field*/
#define	RET_CORRECT	8		/* error return with correct	*/
#define	RET_CANCEL	9		/* error return with cancel	*/
#define	RET_FULL	10		/* error return with dr full	*/
#define	RET_EXISTED	11		/* error return, path exists	*/

/************************************************************************/
/* Definitions of variable types for struct var.v_type.			*/
/************************************************************************/

#define MAXBITS		16

#define TYP_NULL	0
#define TYP_CHAR	1
#define TYP_UCHAR	2
#define TYP_SHORT       3
#define TYP_USHORT	4
#define TYP_ULONG	5
#define TYP_LONG	6
#define TYP_FLOAT	7
#define TYP_DOUBLE	8
#define TYP_STRING	9
#define TYP_NAME	10
#define TYP_DATE	11

#define TYP_ISCALE	128	/* if set, "scale" is an INDEX to factor */
#define TYP_MASK	0xF	/* mask to get storage type from "type" */

/************************************************************************/
/* Definitions for use by scanarg.c when performing the parsing.	*/
/************************************************************************/

#define PAR_FLOAT	1			/* bit 0 set means FLOAT */ 
#define PAR_INT		2			/* bit 1 set means INTEGER */ 
#define PAR_REGION	3			/* bit 2 set means REGION */ 
#define PAR_OPER	4			/* bit 3 set means OPERATOR */ 
#define PAR_FILID	5			/* bit 4 set means FILE ID */
#define PAR_A		6			/* bit 5 set means "A" */
#define PAR_D		7			/* bit 6 set means "D" */
#define PAR_STRING	8			/* bit 7 set means STRING */
#define PAR_SOFTKEY	9			/* bit 8 set means SOFTKEY */
#define PAR_BLANK	10			/* bit 9 set means SPACE */
#define PAR_S		11			/* bit 10 set means 'S' */
#define PAR_UCHAR	12			/* bit 11 allows udef chars */
#define PAR_USTRING	13			/* bit 12 allows udef string */
#define PAR_DOUBLE	14			/* bit 13 set means DOUBLE */

/************************************************************************/
/* 		Macro Language EMS Share-Code Codes			*/
/************************************************************************/

#define	MACEMS_SCMLOADED	(1)	/* ML EMS SCM loaded		*/
#define	MACEMS_SCMNOTDEF	(-1)	/* ML EMS SCM file not defined	*/
#define	MACEMS_SCMNOTFOUND	(-2)	/* ML EMS SCM file not found	*/
#define	MACEMS_EMSNOEXISTS	(-3)	/* Expanded memory not found	*/
#define	MACEMS_EMSNOPAGES	(-4)	/* Not enough EMS pages		*/
#define	MACEMS_SLMSETUP		(-5)	/* ML EMS SCM SLM setup failed	*/
#define	MACEMS_SLMMEM		(-6)	/* ML EMS SCM MEM setup failed	*/
#define	MACEMS_SLMREG		(-7)	/* ML EMS SCM register failed	*/
#define	MACEMS_SLMLOCK		(-8)	/* ML EMS SCM lock failed	*/
