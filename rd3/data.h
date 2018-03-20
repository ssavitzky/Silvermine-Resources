/*** headerID = "data.h 1.0 (c) 1989 S. Savitzky ***/

/*********************************************************************\
**
**	Data -- 	Header file for Data Structure Definitions
**
**	890813 SS	drag pieces in from all over
**
\*********************************************************************/

/*********************************************************************\
**
** Views.
**
\*********************************************************************/

global View  oDriveView;			/* Drive characteristics */
global View	 oVolView;				/* Volume formats */

global View	 oSrcAllView;			/* Source Dir+files list */
global View	 oSrcDirView;			/* Source Directory list */
global View  oDstDirView;			/* Dest. Directory list */

global View  oSrcFileView;			/* Source File list */
global View	 oDstFileView;			/* Dest. File list */

global View	 oTextView;				/* File text */

global View	 oHelpView;				/* Help Text */
global View	 oInitView;				/* Initial Text */

global View	 oMsgView;				/* Messages */

/*********************************************************************\
**
** Screens
**
\*********************************************************************/

ScreenRec HelpScreen,		ViewScreen,
		  SrcDirScreen,		SrcFileScreen,		SrcAllScreen,
		  DstDirScreen, 	DstFileScreen,
		  MainScreen,		DriveScreen,		VolScreen;

/*********************************************************************\
**
** Menus
**
\*********************************************************************/

global MenuItem	
			QuitMenu[],		HelpMenu[],		VuAscMenu[],	VuBinMenu[],
		 	SelectFSMenu[],	SrcDirMenu[],	SrcFileMenu[],
			DstDirMenu[],	DstFileMenu[],	MainMenu[],		DriveMenu[];

/*********************************************************************\
**
** Functions
**
\*********************************************************************/


global void dataInit();				/* Initialize data module		*/


/*********************************************************************\
**
** Version-dependent data
**
\*********************************************************************/

#ifdef V_SP

extern char *dJcampHdr;
extern char  dJcampBuf[];
#endif
