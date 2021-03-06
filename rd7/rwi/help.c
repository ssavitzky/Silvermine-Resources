/*********************************************************************\
**
**	help.c		-- help strings for rwiRMX
**
**		Eventually this may be automatically generated.
**
**	880422	SS	create
**
\*********************************************************************/

#include "version.h"

/*********************************************************************\
**
** Customization stuff
**
\*********************************************************************/

#ifndef V_RAW
#define PRODUCT "RWiRMX"
#define OS 		"iRMX-86"
#define LOGO 	1
#else
#define PRODUCT "RAW"
#define OS		"RAW"
#define LOGO 	4
#endif

static char *id = PRODUCT " version " VERSION "\n"
    COPYRIGHT "  All rights reserved.";

#define SER_PREFIX "This copy of " PRODUCT ", serial number "

static char serialNumberLine[] = SER_PREFIX SERNUM;
static char customerNameLine[] = CUSTNAME;

#define SERLOC (serialNumberLine + strlen(SER_PREFIX))


/*********************************************************************\
**
** Initial Message
**
\*********************************************************************/

char *initMsg[] = {
"",
#if LOGO == 1
" ~|~~~~~~|  |       |       ~|~~~~~~|  |~~~|~~~|   ~|       |~",
"  |      |  |       |        |      |  |   |   |    |       | ",
"  |      |  |       |        |      |  |   |   |    |__   __| ",
"  |      |  |       |   ||   |      |  |   |   |      |___|   ",
"  |______|  |       |        |______|  |       |      __|__   ",
"  |   |     |   |   |   ||   |   |     |       |    __|   |__ ",
"  |   |___  |   |   |   ||   |   |___  |       |    |       | ",
"  |      |  |   |   |   ||   |      |  |       |    |       | ",
" _|      |  |___|___|   ||  _|      |  |       |   _|       |_",
#endif
#if LOGO == 2
"                     ~|~~~~~~|  |~~~|~~~|  ~|       |~",
"                      |      |  |   |   |   |       | ",
"                      |      |  |   |   |   |       | ",
"                  _   |      |  |   |   |   ~~~~|   | ",
"~|_____               |______|  |       |   ____|___| ",
" |    ~  |     |  |   |   |     |       |   |   |     ",
" |       |  |  |  |   |   |___  |       |   |   ~~~~| ",
" |       |  |  |  |   |      |  |       |   |       | ",
" |       |__|__|  |  _|      |  |       |  _|       |_",
#endif
#if LOGO == 3
" ~|~~~~~~|  |       |      ",
"  |      |  |       |      ",
"  |      |  |       |      ",
"  |      |  |       |   || ",
"  |______|  |       |      ",
"  |   |     |   |   |   || ",
"  |   |___  |   |   |   || ",
"  |      |  |   |   |   || ",
" _|      |  |___|___|   || ",
#endif
#if LOGO == 4
" ~|~~~~~~|  |~~~~~~~|  |       |",
"  |      |  |       |  |       |",
"  |      |  |       |  |       |",
"  |      |  |       |  |       |",
"  |______|  |_______|  |       |",
"  |   |     |       |  |   |   |",
"  |   |___  |       |  |   |   |",
"  |      |  |       |  |   |   |",
" _|      |  |       |  |___|___|",
#endif
"",
"READ/WRITE " OS " DISKS -- " PRODUCT " version " VERSION,
COPYRIGHT,
"",
serialNumberLine,
"is licensed to",
customerNameLine,
"",
"Type '?' for help, A for more About " PRODUCT,
(char *)0
};

/*********************************************************************\
**
** Fixer for initial message
**
\*********************************************************************/

void initHelp()
{
	register int   i, len;
	register char *p;
	char xbuf[MAXPAT];

	/* Fix up logo if necessary */

	for (i = 0; i < 10; ++i) {
		for (p = initMsg[i]; *p; ++p)	{
			switch (*p) {
			 case '~':	*p = '\337'; break;
			 case '_':	*p = '\334'; break;
			 case '|':	*p = '\333'; break;
			}
		}
	}

	/* Do appropriate magic with customer name */

	len = sizeof(customerNameLine);
	XORPAT(xbuf, MAXPAT);
	if (*SERLOC == PFXCHAR) return;
	for (i = 0; i < len; ++i) 
		customerNameLine[i] ^= xbuf[i];

}

/*********************************************************************\
**
** Overview Message
**
\*********************************************************************/

char *ovMsg[] = {
"    OVERVIEW:  About " PRODUCT " version " VERSION,
"",
"       "PRODUCT " is a product of  Silvermine Resources, Inc.",
"                            P. O. Box 7474",
"                            Wilton, CT 06897.",
"",
"       " COPYRIGHT,
"       All rights reserved.",
"",
"       This software is licensed for use on a SINGLE KEYBOARD",
"       and a SINGLE CPU.  Any unauthorized use, distribution,",
"       or modification, including (but not limited to) disassembly",
"       or patching of the program, is a violation of Federal law.",
"       Use space and backspace to move in the menu and Enter to select,",
"       or simply type the first letter of the command name.",
"",
"",
"   USING RWiRMX:",
"",
"       Use the up and down arrows, PgUp, and PgDn, to move up and down",
"       within a window.  Home moves to the first item in a list, and",
"       End moves to the last.",
"",
"       Use the left and right arrows to move between views (windows).",
"       In a directory view left arrow gets you to the other directory;",
"       in other views left arrow gets you to a \"higher-level\" view,",
"       and right arrow gets you to a \"lower-level\" view.",
"",
"       In either directory view, you can read the directory tree from",
"       the current drive using the Read command, or select a new drive",
"       (and read its directory) with the D(rive) command.",
"",
"       In the From views, the T(ag) command tags single files or",
"       whole directories for copying with the Write command.",
"       A(scii), and B(inary) tag a file and specify a conversion.",
"",
"   COMMAND-LINE / ENVIRONMENT OPTIONS:",
"",
"       -i <drive-letter>                              set input drive",
"       -o <path>                                      set output path",
"       -d <drive-letter>[HAP][C]=<phys-drive#>        set drive type",
"          H = Hard / A = AT-type / P = PC-type",
"          C = CompatiCard driver",
"",
"       Options can be put on the command line, or in an environment",
"       variable called RWI (with a SET statement in your AUTOEXEC.BAT",
"       file).", 
	(char*)0
};

/*********************************************************************\
**
** Message for disabled commands in demo version
**
\*********************************************************************/

#ifdef V_DE

char *demoMsg[] = {
"   DISABLED COMMAND",
"",
"       You have attempted to use a 'Write' command, which is",
"       disabled in this demonstration version of " PRODUCT ".",
"",
"       You may order the complete version of " PRODUCT " using the order",
"       form you recieved with this disk.  If someone has already",
"       used that form, print the file ORDER.TXT.",
	(char*)0
};

#endif

/*********************************************************************\
**
** Main Menu Help
**
\*********************************************************************/

char *mainMsg[] = {
"   MAIN VIEW",
"",
"       About",
"           This command gives you an overview of the program and",
"           how to use it.",
"       Source",
"           Transfer control to the Source directory view.",
"       Dest",
"           Transfer control to the Destination directory view.",
"       Quit",
"           Exit the program.",
"       ?",
"           Display help.",
"",
"",
"       Use the Esc or left arrow key to return from a help screen.",
	(char *)0
};

/*********************************************************************\
**
** Select Menu Help
**
\*********************************************************************/

char *selectMsg[] = {
"   DRIVE MENU",
"",
"       A-J (DOS)",
"           Select a drive for DOS or " OS " files.  The directory of the",
"           drive you select will be read in and displayed in tree form.",
"",
"       /",
"           The '/' command returns control to the main menu.",
"",
"      Esc",
"           The Esc key returns to the Directory View menu with no",
"           action.",
	(char *)0
};

/*********************************************************************\
**
** Conversion Menu Help
**
\*********************************************************************/

#if 0
char *convertMsg[] = {
"   CONVERSION TYPE MENU",
"",
"       ASCII, Binary"
#ifdef V_SP
					", CSS, Jcamp, Spectrum"
#endif
,
"           Specify the conversion type on ALL files in the current",
"           directory and its subdirectories, and TAG the files for",
"           output as well."
"",
"       /",
"           The '/' command returns control to the main menu.",
"",
"      Esc",
"           The Esc key returns to the Directory View menu with no",
"           action.",
	(char *)0
};
#endif

/*********************************************************************\
**
** Parameter Menu Help
**
\*********************************************************************/

char *paramMsg[] = {
"   DRIVE PARAMETER MENU",
"",
"       Density",
"           Select Drive Density (Single, Double, or High)",
"",
"       Heads",
"           Select Heads (# sides) (1 or 2)",
"",
"       Sector",
"           Select Sector Size (0128, 256, 512, or 1024)",
"           (128 is selected by zero to prevent confusion with 1024)",
"",
"       Track",
"           Select Track Size (Sectors per track)",
"           (2, 4, 8, 9, or 16)",
"",
"       /",
"           The '/' command returns control to the main menu.",
"",
"      Esc",
"           The Esc key returns to the Directory View menu with no",
"           action.",
	(char *)0
};


/*********************************************************************\
**
** FileSys Menu Help
**
\*********************************************************************/

char *fileSysMsg[] = {
"   FILE SYSTEM TYPE MENU",
"",
"       DOS",
"       " OS,
"           Specify the file system (operating system) with which",
"           to read the current drive",
"",
"       /",
"           The '/' command returns control to the main menu.",
"",
"      Esc",
"           The Esc key returns to the Directory View menu with no",
"           action.",
	(char *)0
};

/*********************************************************************\
**
** Drive Type Menu Help
**
\*********************************************************************/

char *driveTypeMsg[] = {
"   DRIVE TYPE MENU",
"",
"       AT",
"           AT-type (high density).  Used for high-density " OS " disks.",
"       CompatiCard",
"           This selection is a toggle; it turns the CompatiCard flag",
"           for the drive either on or off.  Drives controlled by a",
"           CompatiCard can read the volume information on an " OS,
"           disk.",
"       Hard",
"           Hard disk.",
"       PC",
"           PC-type ('double' density).  Used for low-density " OS,
"           disks.",
"       Quit",
"           The Quit command exits the program.",
"       /",
"           The '/' command returns control to the Main View.",
"       Esc",
"           The Escape key returns to the Drive View menu without making",
"           any selection.",
	(char *) 0
};

/*********************************************************************\
**
** Drive View Help
**
\*********************************************************************/

char *driveMsg[] = {
"   DRIVES VIEW",
"",
"       DriveType",
"           Select type of drive (PC, AT, etc.).  The default format",
"           of an " OS " disk is determined by the drive type.",
"",
"       MediaType",
"           Select type (density) of media.",
"			HD: 1.2Mb, MD: 720K, DD: 360K, LD: 180K",
"",
"       System",
"           Select a file system (operating system) for the drive.",
"           The choices are DOS and " OS ".",
"",
"       From",
"           Specify hilited drive as source; go to From view.",
"",
"       To",
"           Specify hilited drive as destination; go to To view.",
"",
"       Quit",
"           The Quit command exits the program.",
"       /",
"           The '/' command returns control to the Main View.",
	(char *) 0
};

/*********************************************************************\
**
** Volume View Help
**
\*********************************************************************/

char *volMsg[] = {
"   VOLUME FORMAT VIEW",
"",
"       ExtSize",
"           Enter the extension size for the highlighted volume format.",
"           (Typical values are 3 and 41)",
"       Files",
"           Enter the number of files (fnodes) for the highlighted volume",
"           format.",
"       MapStart",
"           Enter the number of the block on which the fnode map file",
"           of the highlighted volume format starts.",
"       Select",
"           Select the highlighted volume format for the current drive",
"           (the drive currently highlighted in the Drive view).",
"       Quit",
"           The Quit command exits the program.",
"       /",
"           The '/' command returns control to the Main View.",
	(char *) 0
};

/*********************************************************************\
**
** Source Directory Menu Help
**
\*********************************************************************/

char *dirMsg[] = {
"   SOURCE (FROM) DIRECTORY VIEW",
"",
#ifdef V_CVT
"       ASCII, Binary"
#ifdef V_SP
					", CSS, Jcamp, Spectrum"
#endif
,
"           Specify the conversion type on ALL files in the current",
"           directory and its subdirectories, and TAG the files for",
"           output as well."
"",
#endif
"       Drives",
"           Go to the Drives view to select a disk drive for input",
"           or change the parameters of the current input drive.",
#ifdef V_RAW
"       Parameters",
"           Set the raw drive parameters for this drive",
#endif
"       Read",
"           Read the directory on the selected input disk,",
"       Tag",
"           The Tag command marks the entire contents of the current",
"           directory, and all of its sub-directories, to be written",
"           out when the Write command is issued.",
"       Untag",
"           The Untag command removes the tag mark on the entire contents",
"           of the current directory and all of its sub-directories.",
"       Write",
"           Write all tagged files under the current directory.",
"       Quit",
"           The Quit command exits the program.",
"       /",
"           The '/' command returns control to the Main View.",
	(char *)0
};

/*********************************************************************\
**
** File Menu Help
**
\*********************************************************************/

char *fileMsg[] = {
"   SOURCE FILE VIEW",
"",
#if 0
"       Directory (or left arrow)",
"           Switch to the Directory View (to the left).",
#endif
"       ASCII, Binary"
#ifdef V_SP
					", Jcamp, Spectrum"
#endif
,
"           Tells what kind of conversion to do on the current file",
"           when it is written out.  (These also tag the file.)",
#ifdef V_RAW
"       Parameters",
"           Set the raw drive parameters for this track.",
#endif
"       Tag",
"           The Tag command marks current file to be written out",
"           when the Write command is issued.",
"       Untag",
"           The Untag command removes the tag mark on current file.",
"       View (or right arrow)",
"           View a file in a window (non-ASCII files are viewed in Hex).",
"       Write",
"           Write the current file (whether or not it is tagged).",
"       Quit",
"           The Quit command exits the program.",
"       /",
"           The '/' command returns control to the Main View.",
	(char *)0
};

/*********************************************************************\
**
** Dst Directory Menu Help
**
\*********************************************************************/

char *dstDirMsg[] = {
"   DESTINATION DIRECTORY VIEW",
"",
"       Drives",
"           Go to the Drives view to select a disk drive for output",
"           or change the parameters of the current output drive.",
"       New",
"           Create a new directory.  Enter the name on the prompt line.",
"       Output",
"           Mark the highlighted directory as the destination for",
"           all Write commands.  Make it the DOS working directory",
"           (that is, CD to it) if it is on a DOS disk.",
"       Read",
"           Read the directory on the selected output disk,",
"       Tag",
"           The Tag command marks the entire contents of the current",
"           directory, and all of its sub-directories, to be written",
"           out when the Write command is issued.",
"       Untag",
"           The Untag command removes the tag mark on the entire contents",
"           of the current directory and all of its sub-directories.",
"       Write",
"           Write all tagged files under the current directory.",
"       Quit",
"           The Quit command exits the program.",
"       /",
"           The '/' command returns control to the Main Menu.",
	(char *)0
};

/*********************************************************************\
**
** Dst File Menu Help
**
\*********************************************************************/

char *dstFileMsg[] = {
"   DESTINATION FILE VIEW",
"",
#ifdef V_CVT
"       ASCII, Binary",
"           Tells how to view the current file.",
#endif
#ifdef V_SP
"       Jcamp",
"           Mark the highlighted file as the Jcamp header file.",
#endif
"       Tag",
"           The Tag command marks current file to be written out",
"           when the Write command is issued.",
"       Untag",
"           The Untag command removes the tag mark on current file.",
"       View (or right arrow)",
"           View a file in a window (non-ASCII files are viewed in Hex).",
"       Write",
"           Write the current file (whether or not it is tagged).",
"       Quit",
"           The Quit command exits the program.",
"       /",
"           The '/' command returns control to the Main View.",
	(char *)0
};

/*********************************************************************\
**
** View Menu Help
**
\*********************************************************************/

char *ascMsg[] = {
"   ASCII FILE VIEW",
"",
"       Binary",
"           The Binary command switches to the Binary File View.",
"       Quit",
"           The Quit command exits the program.",
"       /",
"           The '/' command returns control to the Main View.",
"       Esc",
"           The Escape key is equivalent to the Left Arrow command.",
	(char *) 0
};

char *binMsg[] = {
"   BINARY FILE VIEW",
"",
"       ASCII",
"           The ASCII command switches to the ASCII File View.",
"       Quit",
"           The Quit command exits the program.",
"       /",
"           The '/' command returns control to the Main View.",
"       Esc",
"           The Escape key is equivalent to the Left Arrow command.",
	(char *) 0
};


