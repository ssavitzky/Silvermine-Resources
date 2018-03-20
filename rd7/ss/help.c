/*********************************************************************\
**
**	help.c		-- help strings for RD7
**
**		Eventually this may be automatically generated.
**
**	880422	SS	create
**
\*********************************************************************/

#include "rd7.h"

#ifdef V_AB
#define V_PREFIX "AB"
#endif

#ifdef V_SP
#define V_PREFIX "SP"
#endif

#ifdef V_DE
#define V_PREFIX "DEMO"
#endif


/*********************************************************************\
**
** Customization stuff
**
\*********************************************************************/

static char *id = "rd7 version " V_PREFIX " " VERSION "\n" 
    COPYRIGHT "  All rights reserved.";

#define SER_PREFIX "This copy of rd7, serial number "

static char serialNumberLine[] = SER_PREFIX SERNUM;
static char customerNameLine[] = CUSTNAME;

#define SERLOC (serialNumberLine + strlen(SER_PREFIX))


/*********************************************************************\
**
** Initial Message
**
\*********************************************************************/

#define LOGO 4

char *initMsg[] = {
"",
#if LOGO == 1
"|~~~~~~~~     |~~~~~~~        |~~~~~~~",
"  ~      ~      ~      ~             ~",
"  ~      ~      ~       ~           ~ ",
"  ~      ~      ~       ~          ~  ",
"  ~~~~~~~       ~       ~         ~   ",
"  ~   ~         ~       ~        ~    ",
"  ~    ~        ~       ~       ~     ",
"  ~     ~       ~      ~       ~      ",
" ~~      ~     ~~~~~~~~        ~      ",
#endif
#if LOGO == 2
"               |~  ~~|~~        |~                     ",
"               |     |          |                      ",
"               |     |          |                      ",
"               |     |          |             ~        ",
"~|_____  ______|     |    ______|   ~|_____   |  |~~~~|",
" |    ~  |     |     |    |     |    |    ~   |  |     ",
" |       |     |     |    |     |    |        |  ~~~~~|",
" |       |     |     |    |     |    |        |       |",
" |       |_____|   __|__  |_____|    |        |  |____|",
#endif
#if LOGO == 3
"               |~   |~~~~~~~",
"               |           ~",
"               |          ~ ",
"               |         ~  ",
"~|_____  ______|        ~   ",
" |    ~  |     |       ~    ",
" |       |     |      ~     ",
" |       |     |     ~      ",
" |       |_____|     ~      ",
#endif
#if LOGO == 4
"                 ||   |||||||||",
"                 ||          ||",
"                 ||          ||",
"                 ||         // ",
"||||||\\   /||||||||        //  ",
"||    \\\\  ||     ||       //   ",
"||        ||     ||      //    ",
"||        ||     ||     //     ",
"||        |||||||||    //      ",
#endif
"",
"READ 7000 DISKS -- rd7 version " V_PREFIX " " VERSION,
COPYRIGHT,
"",
serialNumberLine,
"is licensed to",
customerNameLine,
"",
"Type '?' for help, A for more About RD7",
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

#if LOGO != 4
	for (i = 0; i < 10; ++i) {
		for (p = initMsg[i]; *p; ++p)	{
			switch (*p) {
			 case '~':	*p = '\337'; break;
			 case '_':	*p = '\334'; break;
			 case '|':	*p = '\333'; break;
			}
		}
	}
#endif

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
"    OVERVIEW:  About rd7 version " V_PREFIX " " VERSION,
"",
"       rd7 is a product of  Silvermine Resources, Inc.",
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
"   USING rd7:",
""
"       Use the up and down arrows, PgUp, and PgDn, to move up and down",
"       within a window.  Home moves to the first item in a list, and",
"       End moves to the last.",
"",
"       Use the left and right arrows to move between views (windows).",
"       In a directory view left arrow gets you to the other directory;",
"       in other views left arrow gets you to a \"higher-level\" view,",
"       and right arrow gets you to a \"lower-level\" view.",
"",
"       In either the DOS or the Idris directory view, you can read",
"       the directory from the current drive using the Read command,",
"       or select a new drive with the D(rive) command.",
"",
"       In the Idris views, the T(ag) command tags single files or",
"       whole directories for copying with the Write command.",
"       A(scii), B(inary), S(pectrum), and J(CAMP) tag a file and",
"       specify a conversion (ASCII is the default; Spectrum and JCAMP",
"       are available only in rd7-SP).",
"",
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
"       disabled in this demonstration version of rd7.",
"",
"       You may order the complete version of rd7 using the order",
"       form you recieved with this disk.  If someone has already",
"       used that form, print the file ORDER.RD7.",
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
"       DOS",
"           Transfer control to the DOS directory view.",
"       Idris",
"           Transfer control to the Idris directory view.",
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
"       0-9 (Idris)",
"           Select a drive for DOS or Idris files.  The directory of the",
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

/*********************************************************************\
**
** Directory Menu Help
**
\*********************************************************************/

char *dirMsg[] = {
"   IDRIS DIRECTORY VIEW",
"",
#if 0
"       DOS (or left arrow)",
"           Switch to the DOS Directory View.",
"       File (or right arrow)",
"           Switch to the File View (to the right).",
"       Conversion",
"           The Conversion command enters a sub-menu that lets you",
"           specify the conversion type (A, B, J, S) for the entire",
"           directory and its subdirectories.  All files are tagged.",
#endif
"       ASCII, Binary"
#ifdef V_SP
					", CSS, Jcamp, Spectrum"
#endif
,
"           Specify the conversion type on ALL files in the current",
"           directory and its subdirectories, and TAG the files for",
"           output as well."
"",
"       Drive",
"           Select a disk drive for input and read its directory.",
"       Read",
"           Read the directory on the selected input (Idris) disk,",
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
"   IDRIS FILE VIEW",
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
** DOS Directory Menu Help
**
\*********************************************************************/

char *dosMsg[] = {
"   DOS DIRECTORY VIEW",
"",
#if 0
"       Idris (or left arrow)",
"           Switch to the Idris Directory View.",
"       File (or right arrow)",
"           Switch to the File View (to the right).",
#endif
"       Drive",
"           Select a DOS disk drive and read its directory.",
"       New",
"           Create a new directory.  Enter the name on the prompt line.",
"       Output",
"           Mark the highlighted directory as the destination for",
"           all Write commands.  Make it the DOS working directory",
"           (that is, CD to it).",
"       Read",
"           Read the directory on the selected output (DOS) disk,",
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
** Dos File Menu Help
**
\*********************************************************************/

char *dosFileMsg[] = {
"   DOS FILE VIEW",
"",
"       ASCII, Binary",
"           Tells how to view the current file.",
#if 0
"       Directory (or left arrow)",
"           The Directory or left arrow command moves control",
"           into the Directory View.",
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
#if 0
"       Files (or left arrow)",
"           The Files command or left arrow moves control back to the",
"           File View.",
#endif
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
#if 0
"       Files (or right arrow)",
"           The Files command or left arrow moves control back to the",
"           File View.",
#endif
"       Quit",
"           The Quit command exits the program.",
"       /",
"           The '/' command returns control to the Main View.",
"       Esc",
"           The Escape key is equivalent to the Left Arrow command.",
	(char *) 0
};


