/************************************************************************/
/*			Benchtop Error Message Codes			*/
/*			Codes 0-99 Reserved for Benchtop		*/
/************************************************************************/

#define	EAESERR		0		/* AES function call error	*/
#define	EVDIERR		1		/* VDI function call error	*/
#define	EFILOPEN	2		/* File open error		*/
#define	EFILREAD	3		/* File read error		*/
#define	EFILWRITE	4		/* File write error		*/
#define	EFILCLOSE	5		/* File close error		*/
#define	ERDONLY		6		/* Can not delete rd only	*/
#define	ENOSPACE	7		/* Out of space on disk		*/
#define	EWINDMAX	8		/* Too many windows open	*/
#define	EWINDHAND	9		/* Illegal window handle	*/
#define	ECOPYDIR	10		/* Copy to same directory error	*/
#define	ECOPYBENCH	11		/* Copy to Benchtop error	*/
#define	EBADDEF		12		/* Bad default in switch	*/
#define	ESYSERR		13		/* System error			*/
#define	ENOTFOUND	14		/* File not found		*/
#define	EILLDEFPATH	15		/* Illegal default path string	*/
#define	EDRFULL		16		/* Data region is full		*/
#define	ESPECID		17		/* Illegal spectrum id		*/
#define	ESPECDEL	18		/* Spectrum delete error	*/
#define	EITEMSELDIR	19		/* Illegal Item Selector dir	*/
#define	EITEMSELFIL	20		/* Illegal Item Selector file	*/
#define	EDROPEN		21		/* Data Region window open	*/
#define	ECOPYDR		22		/* Illegal copy to Data Region	*/
#define	EDRDEL		23		/* Error in Data Region delete	*/
#define	EOVLLOAD	24		/* Overlay load error		*/
#define	EINVDRI		25		/* Invalid drive specification	*/
#define	EFORMFAIL	26		/* Format Failure		*/
#define	EILLFORMAT	27		/* Illegal format		*/
#define	ERSCLOAD	28		/* Resource load error		*/
#define	ENOHANDLES	29		/* No more handles available	*/
#define	EILLFILENAME	30		/* Illegal filename specifier	*/
#define	EFOLDEMPTYCOPY	31		/* Empty folder copy		*/
#define	EFOLDEMPTYDEL	32		/* Empty folder delete		*/
#define	EMISSCMDPARM	33		/* Missing command parameter	*/
#define	EILLCMDPARM	34		/* Illegal command parameter	*/
#define	EPATHNOTFOUND	35		/* Path/folder not found	*/
#define	EWRRDONLY	36		/* Can not overwrite rd only	*/
#define	EFOLDCREATE	37		/* Folder create error		*/
#define	EILLHANDLE	38		/* Invalid file handle number	*/
#define	EDRNOTFOUND	39		/* Data Region file not found	*/
#define	EILLABSVAL	40		/* Illegal abscissa value	*/
#define	EILLABSORDER	41		/* Illegal abscissa order	*/
#define	EILLOBJDRAG	42		/* Illegal object drag		*/
#define	EILLDRSELNAME	43		/* Illegal Data Region Sel name	*/
#define	EILLORDVAL	44		/* Illegal ordinate value	*/
#define	EPRNWKSOPEN	45		/* Printer workstation open	*/
#define	EPLTWKSOPEN	46		/* Plotter workstation open	*/
#define	EINVFORM	47		/* Invalid format for commands	*/
#define	EOVLEXEC	48		/* Overlay exec error		*/
#define	EDRFILCREATE	49		/* Data Region create error	*/
#define	EDRFILOPEN	50		/* Data Region open error	*/
#define	EDRFILREAD	51		/* Data Region read error	*/
#define	EDRFILWRITE	52		/* Data Region write error	*/
#define	EDRFILCLOSE	53		/* Data Region close error	*/
#define	ECOPYRIGHT	54		/* Save of copyrighted data	*/
#define	EDRFILALLOC	55		/* Data Region allocate error	*/
#define	EUNDO		56		/* Undo Command error		*/
#define	EINCPATH	57		/* Incomplete path specified.	*/
#define	EINVICON	58		/* Invalid icon selected.	*/
#define	EILLWINACTION	59		/* Illegal wind action selected.*/
#define	ENOWINDOPEN	60		/* No windows open.		*/
#define	EILLWINCOORDS	61		/* Illegal window coordinates.	*/
#define	EILLSTATUSOBJ	62		/* Illegal status command.	*/
#define	EINVPREF	63		/* Invalid preference selected.	*/
#define	EILLOPENOBJ	64		/* Illegal OPEN object selected.*/
#define	EABORTRET	65		/* Error in retrieve, abort?	*/
#define	EIGNORPATH	66		/* Ignore path?			*/
#define	EAREYOUSURE	67		/* Are you sure? *.* delete.	*/
#define	EVWPLOTEXISTS	68		/* Plot already exists in VW	*/
#define	EILLCMD		69		/* Illegal TYPED COMMAND.	*/
#define	EILLDRFILE	70		/* Illegal Data Region file	*/
#define	EUNDEFDEFACT	71		/* Undefined default action	*/
#define	EVWILLFILE	72		/* Illegal View Window file	*/

/************************************************************************/
/*		Application Memory Resident Error Message Codes		*/
/*			Use codes between 100 and 199			*/
/************************************************************************/

#if	PEMSB
#define	ETAATDERV	100		/* Illegal TAAT on deriv spectra*/
#define	ETAATIRUV	101		/* Illegal TAAT on non IR/UV sp	*/
#define	ETAATORD	102		/* Illegal TAAT ordinate type	*/
#define	ESMLEVEL	103		/* Illegal SMOOTH level number	*/
#define	EDERVORDER	104		/* Illegal derivative order	*/
#define	EDERVLAMB	105		/* Illegal Delta Lambda		*/
#define	EABSRANGE	106		/* Abscissa value out of range	*/
#define	EABSNE		107		/* Illegal ARITH string abs	*/
#define	EINTVNE		108		/* Illegal wavelength interval	*/
#define	EINCOMP		109		/* Illegal ARITH spec type	*/
#define	EILLOPR		110		/* Illegal ARITH operator	*/
#define	EDIVZERO	111		/* Division by zero		*/
#define	EILLVAL		112		/* Illegal constant value	*/
#define	EINVSEL		113		/* Invalid box selections	*/
#define	ERAM		114		/* Not enough room in Data Reg	*/
#define	EOVFL		115		/* ARITH data overflow error	*/
#define	EORDER		116		/* Illegal Abscissa Order	*/
#define	EILLSNAME	117		/* Illegal Data Region name	*/
#define	EILLPEAK	118		/* Illegal Peak spectrum	*/
#define	EDIFFABS	119		/* Illegal Auto Diff abscissa	*/
#define	ETAATTD		120		/* Illegal TAAT on Time Drive	*/
#define EILLDNAME	121		/* Illegal Dest name		*/
#define	EILLS2NAME	122		/* Illegal Second src region	*/
#define EINVTHR		123		/* Illegal threshold value	*/
#define	EILLDERV	124		/* Illegal deriv spectrum	*/
#define	ENEGBASE	125		/* Negative baseline values	*/
#define	EFUNCTYP	126		/* Illegal function type	*/
#define	EBASEPT		127		/* Illegal baseline point	*/
#define	EWAVESNO	128		/* Illegal wave step number	*/
#define	ENONIR		129		/* Illegal spectrum for flat	*/
#define	EFLATORD	130		/* Illegal flat ordtype		*/
#define	EILLSTEP	131		/* Illegal step value for flat	*/
#define ENOPEAKS	132		/* No peaks were found		*/
#define	EDUPNAME	133		/* Duplicate Data Region Name	*/
#define    ENDSRCN      134     /*  No source filename entered */
#define    ENDSRC       135     /*  File not in data region  */
#define    EABSCIS      136     /*  Abscissa point out of range */
#define    EFROMTO      137     /*  From and to must be between Bases */
#define    EADATA       138     /*  Illegal format for area command */
#define    EFRORTO      139     /*  Illegal From or To value */
#define    ESTRTEND     140     /*  Limits out of range */
#define    EORDVAL      141     /*  Ordinate value illegal for given abscissa */
#define    EPTSLZERO    142        /* Points <= 0 in data file    */
#define    ENOTLOGDT    143        /* Not LOG data.        */
#define    EALREADYL    144        /* Already LOG data.        */
#define    ETAATSCALE   145        /* Illegal scale  for taat */
#define    EGFISEL      146       /* Gem file selector error   */
#define    EFIEXT       147       /* File extension must be .DX */
#define    EDXFIL       148      /*  Illegal Jcamp file. */
#define    ESPECWIDTH   149      /* spectral width to small for enhance */ 
#define    ESPECWIDNO   150      /* spectral width to small for enhance */ 
#define    ESPECFACT    151      /* spectral width to small for enhance */ 
#define    ERRUTIL      152      /* Invalid file conversion in utility */
#define    ERRUTIL      152      /* Invalid file conversion in utility */
#define    ESAMESRC     153      /* Both src names same in diff */
#define    EINTDIF      154      /* Interferograms not allowed in DIFF */ 
#endif

/************************************************************************/
/*		Application File Resident Error Message Codes		*/
/*			Use codes 200 and above				*/
/************************************************************************/

#if	I4444
#define	ENOINIT		200		/* Instrument is not initialized*/
#define	EACC_INACTIVE	201		/* Accessory is not active	*/
#define	EWDW_OPEN	202		/* CPW already open		*/
#define	EINV_ENTRY	203		/* Invalid entry to edit field	*/
#define	EORD_RNG	204		/* Ordinate min > max		*/
#define EORD_VAL	205		/* Ordinate value out of range	*/
#define	EABSC_RNG	206		/* Abscissa min > max		*/
#define	EABSC_VAL	207		/* Abscissa value out of range	*/
#define	EDLAMP		208		/* D2 lamp must be ON   	*/
#define	EWLAMP		209		/* Tungsten lamp must be ON   	*/
#define	ECYC_TIME	210		/* Invalid Cycle Time		*/
#define	EDATA_INT	211		/* Invalid Data Interval	*/
#define	EID_LENGTH	212		/* Id must be 4 chars in length	*/
#define	EID_ALPHA	213		/* Id must start with Alpha	*/
#define	EID_ILLEGAL	214		/* Illegal char in File Id	*/
#define	EID_NUMBER	215		/* File Id needs 3 Numeric	*/
#define	EID_ZEROS	216
#define	ETABLE_INT	217		/* Table Int. less than Data Int*/
#define	ETABLE_MLT	218		/* Table Int. not a multiple of Data Int*/
#define	EDERIV_PTS	219		/* Not enough pts for a deriv run*/
#define	ENO_CELLS	220		/* No Cell Programmer cells active*/
#define	ECHNG_WL 	221		/* Change wavelength from caculate delta*/
#define EUV_OFF_DCHK	222		/* Error with UV and stop wavelength*/
#define	ENO_DATA	223		/* Data region is 0		*/
#define	ENO_MULTIPLE	224		/* Data interval is invalid with speed*/
#define	EDEL_TOOBIG	225		/* Calculate del is > 1.0 nm	*/
#define	EMIN_TOOLOW	226		/* Calculated wave min < 190.0	*/
#define	EILL_NUMPTS	227		/* Illegal number of pts calculated*/
#define	EACWNDOP	228		/* Accessory window already open*/
#define	EACC_NOTON	229		/* Accessory is not turned ON	*/
#define	EVOL_ERR	230		/* Sipper Volume error		*/
#define	EFOR_ERR	231		/* Sipper Forward error		*/
#define	EPOS_ERR	232		/* Sipper Position error	*/
#define	EPDELAY_ERR	233		/* Sipper Print Delay error	*/
#define	ESTDS_ERR	234		/* Multisampler standards error	*/
#define	ESMPS_ERR	235		/* Multisampler samples error	*/
#define	ESMPS_STDS	236		/* Samps + Stds > 210		*/
#define	ETEMP_ERR	237		/* Temperature value error	*/
#define	ELAI_ERR	238		/* Lambda Accessory Interface Error*/
#define ENO_FILE	239		/* Data file does not exist	*/
#define	EILL_SPEC	240		/* Spectrum is not compatable	*/
#define	EOVWRT_AUCPY	241		/* Overwrite and Autocopy both on*/
#define	EFILES_999	242		/* Trying to create too many files*/
#define	ENO_PEAK	243		/* Calibration peak not found	*/
#define	EREF_BLK	244		/* Reference beam blocked	*/
#define	ETUNG_OUT	245		/* Tungsten lamp is burned out	*/
#define	EUV_OUT		246		/* UV lamp is burned out	*/
#define	ECPWOPEN	247		/* Only 1 CPW open at a time flag*/
#define	EFIXWL_UV	248		/* UV out if fixed wavelength < 340.0*/
#define	EFIXWL_VIS	249		/* VIS out if fixed wavelength > 320.0*/
#define	ETIME_DELTA	250		/* Time Drive delta is too small*/
#define	ECONV_MINS	251		/* Illegal # of mins to convert to secs*/
#define	ETOT_TIME	252		/* Illegal Total Time entry	*/
#define	ETIME_INT	253		/* Time Interval > Total Time	*/
#define	ETIME_MLT	254		/* Time Interval not a mult of Total Time*/
#define	EINST_ERROR	255		/* IEEE Communications Error	*/
#define	EPAUSE_TIME	256		/* Pause initiated time expired */
#define	EPAUSE_NUM	257		/* Total # of pauses not executed*/
#define	ETD_SECS_DEL	258		/* Time Drive delta not in .6sec*/
#define	EONE_TABLE	259		/* One Table window on screen at a time*/
#define	ESAFEMETH	260		/* Cannot change a Safe Method File status*/
#define	EINV_METHUPD	261		/* Incompatable method update	*/
#define	EAPPL_CHANGE	262		/* Close CPW when changing applications*/
#define	EAPPL_UPDATE	263		/* Illegal Application Method Update*/
#define	EMSBOX_ERR	264		/* Do not use Mult. dialog box if quant*/
#define	EDERIVALLOC	265		/* Cannot allocate derivative buffer*/
#define	EPTS_PER_SEC	266		/* Number of pts/second displayed*/
#define	EBLD_VW_LIST	267		/* Could not build the view plot list*/
#define	EMETHOD_FILE	268		/* Can not print a method file	*/
#define	EILL_MODE	269		/* Illegal mode with the Cell Programmer*/
#endif

/************************************************************************/
/*			Inquire Message Codes				*/
/************************************************************************/

#define	IVERFILDEL	0		/* Verify file delete		*/
#define	IVERDRDEL	1		/* Verify Data Region Delete	*/
#define	IDESFILEX	2		/* Destination file exists	*/
#define	IDRFILEX	3		/* Data Region file exits	*/
#define	IVERFILECOPY	4		/* Verify file copy		*/
#define	IVERRETR	5		/* Verify data Retrieve		*/
#define	IVERFOLDERCOPY	6		/* Verify folder copy		*/
#define	IVERFOLDERDEL	7		/* Verify folder delete		*/
#define	IVERSAVE	8		/* Verify data Save		*/
#define	IVERNEW		9		/* Verify New menu item select	*/
#define	IVERDRCLR	10		/* Verify Data Region clear	*/
#define	IVERQUITDOS	11		/* Verify Quit to DOS		*/
#define	IVERQUITDESK	12		/* Verify Quit to Desktop	*/
#define	IVERTOOUTPUT	13		/* Verify to Output		*/
#define	IVERENTERDOS	14		/* Verify enter DOS commands	*/

#define	INQUIRE_OFFSET	500		/* Inquire offset for Macro Lan	*/

/************************************************************************/
/*			Argument Checking				*/
/************************************************************************/

extern	int	error(unsigned int, unsigned int, char *, char *, unsigned int, unsigned int, unsigned int, int);
extern	int	inquire(unsigned int, unsigned int, char *, char *, unsigned int, unsigned int, unsigned int, int);
