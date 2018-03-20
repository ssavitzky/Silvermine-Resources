/* jcopens.c - contains openDX() and openHDR() *********************/
#include <stdlib.h>
#include <string.h>
#include <std.h>
#include <io.h>
#include <stdio.h>

#define EXIST 0

void errstrg(char*);
char *cpystr(char *,...);
char nums[]={"0123456789"};

FILE far *openDX(char *dxfl)
	{
	int i;
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];
	int namelen;

	/* if dxfile exists, tack on a number, up to 9, truncate if necessary */
	for (i = 0; i < 10; i++)
		{
		if (access(dxfl,EXIST) == -1)
			break;  /* no entry found, so should be able to open?? */
		/* file exists, try adding digit to name */
		_splitpath(dxfl,drive,dir,fname,ext);
		if ((namelen = strlen(fname)) > 7 )
			namelen = 7;				/* truncate name for digit if already 8 */
		fname[namelen++]=nums[i];
		fname[namelen] = '\0';
		_makepath(dxfl,drive,dir,fname,ext);
		}								/* go around again */
	if ( i > 9 )
		{
		errstrg("DXfiles already exist, delete or change dir \n");
		return(0L);
		}

	return(fopen(dxfl,"wt"));
	}

/************************* openHDR **********************/
FILE *openHDR(char *hdrfl)
	{
	return(fopen(hdrfl,"rt"));
	}

