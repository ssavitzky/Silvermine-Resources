/***/static char *moduleID="dosdir 1.0 (c)1988 S. Savitzky";/***/

/*********************************************************************\
**
**	D O S   D I R E C T O R Y   T R E E S
**
**		This module defines the subclass of Dir that handles DOS
**		directories.
**
**	891210	SS	split out of Dirs
**
\*********************************************************************/

#include "coops.h"
#include "tree.h"
#include "dir.h"
#include <io.h>
#include <stdio.h>
#include <time.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/utime.h>
#include <malloc.h>
#include <dos.h>
#include "dostypes.h"		/* from Microsoft library sources */

#undef  global
#define global

extern void errorSet(), errorClear();
extern int errorCheck();


extern Object objInit(), objClone(), objKill(), objNew();
extern Object objDoesNotImplement();
extern Tree   treeNext(), treePrev(), treeDown(), treeUp(),
			  treeSucc(), treePred(), treeFirst(), treeLast(),
			  treeAfter(), treeBefore(), treeFront(), treeBack(),
			  treeCut(), treeFind();
extern Object treeKill();
extern String   treePath(), treeHeader();

extern TreeClassRec crTree;

extern DirClassRec crDir;
extern String dirHeader(), dirName();
extern Object dirNew(), dirKill(), dirOpen();

