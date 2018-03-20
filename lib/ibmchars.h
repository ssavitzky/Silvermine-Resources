/*** Header: "IBMCHARS.H 1.0 copyright 1987 S. Savitzky" ***/

/*********************************************************************\
**
**	IBMCHARS --	 include header file for IBM character set extensions
**
**		This file contains constant definitions for the IBM extensions
**		to the ASCII character set, including both the low set (control
**		characters) and the high set (high-order bit set).
**		The foreign-alphabet characters are omitted.
**
**	870813 SS	create PC version.
**
\*********************************************************************/


/*********************************************************************\
**
** The Low Set
**
**	Prefixes:  C_	Character
**			   A_	ANSI terminal driver meaning
**
\*********************************************************************/
					   	/* ansi terminal driver eats some */
#define	  C_SMILE	1
#define	  C_ISMILE	2
#define	  C_HEART	3
#define	  C_DIAMOND	4
#define	  C_CLUB	5
#define	  C_SPADE	6
#define	  A_BEL		7	/* ansi BEL */
#define	  C_DOTBOX	8
#define	  A_HT		9	/* ansi HT */
#define	  A_LF		10	/* ansi LF */
#define	  A_VT		11	/* ansi VT = home */
#define	  C_MALE	11
#define	  A_FF		12	/* ansi FF */
#define	  C_FEMALE	12
#define	  A_CR		13	/* ansi CR */
#define	  C_NOTES	14
#define	  C_SUN		15
#define	  C_RTWEDGE	16
#define	  C_LFWEDGE	17
#define	  C_UDARROW	18
#define   C_BANGBANG 19
#define	  C_PARAG	20
#define	  C_SECTION	21
#define	  C_HBAR	22
#define	  C_UPDNBAR	23
#define	  C_UPARROW	24
#define	  C_DNARROW	25
#define	  C_RTARROW	26
#define	  A_ESC		27	/* ansi ESC */
#define	  C_LFARROW	27
#define	  A_RTCSR	28	/* ansi right cursor */
#define	  C_LLANGLE	28
#define	  A_LFCSR	29	/* ansi left cursor */
#define	  C_LRARROW	29
#define	  A_UPCSR	30	/* ansi up cursor */
#define	  C_UPWEDGE	30
#define	  A_DNCSR	31	/* ansi down cursor */
#define	  C_DNWEDGE	31

#define	  A_DEL		127	/* ansi DEL */
#define	  C_HOUSE	127

/*********************************************************************\
**
** The High Set
**
**		prefixes:    C_ = character
**					UC_ =  uppercase (Greek)
**
\*********************************************************************/

#define	  C_CENT	155
#define	  C_POUND	156
#define	  C_YEN		157

#define	  C_IQMARK	168
#define	  C_ULANGLE	169
#define	  C_URANGLE	170
#define	  C_HALF	171
#define	  C_QUARTER	172
#define	  C_IBANG	173
#define	  C_LSLS	174
#define	  C_GTGT	175
#define   C_HTONE1	176
#define	  C_HTONE2	177
#define	  C_HTONE3	178

/* box drawing set goes here */

#define	  C_BLOCK	219
#define	  C_BHBLOCK	220
#define	  C_LHBLOCK	221
#define	  C_RHBLOCK	222
#define	  C_THBLOCK	223

#define	  C_ALPHA	224
#define	  C_BETA	225
#define	 UC_GAMMA	226
#define	  C_PI		227
#define	 UC_SIGMA	228
#define	  C_SIGMA	229
#define	  C_MU		230
#define	  C_TAU		231
#define	 UC_PHI		232
#define	 UC_THETA	233
#define	 UC_OMEGA	234
#define	  C_DELTA	235
#define	  C_IFNI	236
#define	  C_PHI		237

#define	  C_MEMBER	  238
#define	  C_INTERSECT 239
#define	  C_EQUIV	  240
#define	  C_PLUSMINUS 241
#define	  C_GTEQL	  242
#define	  C_LSEQL	  243
#define	  C_TOPINTEG  244
#define	  C_BOTINTEG  245
#define	  C_DIVIDE	  246
#define	  C_APPROX    247
#define	  C_DEGREE	  248
#define	  C_BULLET	  249
#define	  C_CTRDOT	  250
#define	  C_SQRT	  251
#define	  C_TOTHE_N	  252
#define	  C_SQUARED	  253
#define	  C_QED		  254


/*********************************************************************\
**
** The Box-Drawing Set
**
**			UL-----TOP-----UR		+HORIZ
**			L		|	    R 		V
**			E		|	    I		E
**			F -----CEN----- G		R
**			T		|	    H		T
**			.		|	    T
**			LL-----BOT-----LR
**
**	Prefixes:
**		S  = single lines 		D  = double lines
**		VD = vertical double	HD = horiz double
**
\*********************************************************************/

#define  S_VERT		179
#define  S_RIGHT	180
#define  S_UR		191
#define  S_LL		192
#define  S_BOT		193
#define  S_TOP		194
#define  S_LEFT		195
#define  S_HORIZ	196
#define  S_CEN		197
#define  S_LR		217
#define  S_UL		218

#define  D_VERT		186
#define  D_RIGHT	185
#define  D_UR		187
#define  D_LL		200
#define  D_BOT		202
#define  D_TOP		203
#define  D_LEFT		204
#define  D_HORIZ	205
#define  D_CEN		206
#define  D_LR		188
#define  D_UL		201

#define HD_VERT		179		/* == VERT */
#define HD_RIGHT	181
#define HD_UR		184
#define HD_LL		212
#define HD_BOT		207
#define HD_TOP		209
#define HD_LEFT		198
#define HD_HORIZ	205		/* == D_HORIZ */
#define HD_CEN		216
#define HD_LR		190
#define HD_UL		213

#define VD_VERT		186		/* == D_VERT */
#define VD_RIGHT	182
#define VD_UR		183
#define VD_LL		211
#define VD_BOT		208
#define VD_TOP		210
#define VD_LEFT		199
#define VD_HORIZ	196		/* == HORIZ */
#define VD_CEN		215
#define VD_LR		189
#define VD_UL		214




