/*** headerID = "filevr.h 1.0 (c) 1989 S. Savitzky ***/

/*********************************************************************\
**
**	FileVr -- 	Header file for File Viewer
**
**	891210 SS	split from viewer.c
**
\*********************************************************************/

/* Uncomment to use new fileVr code	*/
#define FILE_VIEWER 1

typedef struct _FileVrPart {
	Dir			d;				/* the directory node for this file */
	long		org;			/* origin of current buffer */
	short		bx;				/* index of this buffer in pool */
} FileVrPart;

typedef struct _FileVrRec {
	ObjectPart 	object;
	TextVrPart	textVr;
	FileVrPart 	fileVr;
} FileVrRec, *FileVr;


global VrClass clFileViewer;	/* contents of file */
global char fileVrEOL;			/* end-of-line character */
