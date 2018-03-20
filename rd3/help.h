/*** moduleId = "help.h v1.0 (c)1988 S. Savitzky"; ***/

/*********************************************************************\
**
** help.h		external definitions for help file
**
**	880422	SS	create
**
\*********************************************************************/

extern char *initMsg[];

extern char *ovMsg[];
#define ovHelp ((char *)ovMsg)

extern char *demoMsg[];
#define demoHelp ((char *)demoMsg)

extern char *mainMsg[];
#define mainHelp ((char *)mainMsg)

extern char *ascMsg[];
#define ascHelp ((char *)ascMsg)

extern char *binMsg[];
#define binHelp ((char *)binMsg)

extern char *selectMsg[];
#define selectHelp ((char *)selectMsg)

extern char *convertMsg[];
#define convertHelp ((char *)convertMsg)

extern char *fileSysMsg[];
#define fileSysHelp ((char *)fileSysMsg)

extern char *paramMsg[];
#define paramHelp ((char *)paramMsg)

extern char *dirMsg[];
#define dirHelp ((char *)dirMsg)

extern char *fileMsg[];
#define fileHelp ((char *)fileMsg)

extern char *dstDirMsg[];
#define dstDirHelp ((char *)dstDirMsg)

extern char *dstFileMsg[];
#define dstFileHelp ((char *)dstFileMsg)

extern char *driveMsg[];
#define driveHelp ((char *)driveMsg)

extern char *volMsg[];
#define volHelp ((char *)volMsg)

extern char *driveTypeMsg[];
#define driveTypeHelp ((char *)driveTypeMsg)

extern void initHelp();
