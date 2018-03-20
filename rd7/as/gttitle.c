/*	gttitle - gets title according to following rules 


called only if no -t on command line
rule then is:
    if not default header file
        then read header file
        if first ## is not TITLE or empty
            then goto krec
    if krec ident not empty then use it
    else use name of output file
*/
#include <stdlib.h>
#include <std.h>
#include "spec.h"
#define TITLESIZE 72

VOID gttitle(title,ishdr,pkrec,filid)
    TEXT *title;
    LONG ishdr;
    KREC *pkrec;
    TEXT *filid;
    {
    FAST TEXT *s;
    TEXT *idptr;
    TEXT temp[TITLESIZE+12];
    LONG hdrlength,idlength;
    LONG i,j,k;
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];

    if (ishdr)
        {                   /* not default header */
        if ((hdrlength=rdline(temp,(LONG)sizeof(temp)))>0)
            {               /* first line of file must be ##TITLE= */
                            /*   even if empty                     */
            temp[hdrlength+1] = '\0';
            if(!strstr(temp,"##TITLE=")) 
                {                           /* not found */
                puts("first line of header file not ##TITLE=");
                puts(temp);
                hdrlength = 0;
                }
            }
        else
            {
            hdrlength -=8;
            if (hdrlength > 1)
                {
                s = temp + 8;
                for (i=0;i<hdrlength;i++)
                    *title++ = *s++;
                *title = '\0';          /* have a title */
                return;
                }
            }
        }

    /* no title yet, so look in krec */
    idptr = pkrec->ident;
    s = idptr + 72;
    /* find end of text info */
    for (i=72;i && ((*s==' ')||(*s=='\0'));s--,i--) ;
    if (idlength = i)        /* i = idlength > 0 */
        {
        idlength++;
        s = idptr;
        for (i=0;i<idlength;i++)
            *title++ = *s++;
        *title = '\0';
        return;
        }

    /* still no title, so use file output name */
	_splitpath(filid,drive,dir,fname,ext);
    strcpy(title,fname);

    return;
    }

