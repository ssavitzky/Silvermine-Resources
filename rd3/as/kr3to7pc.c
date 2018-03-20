/* kr3to7pc - moves info from 3000krec to flipped rd7 krec */
/* from: oldnew - moves info from okrec to AIDS krec */
/* modified 90-3-23++*/
/* -date- 83/06/23  Savitzky */
#include <std.h>
#include "okrec.h"
#include "..\..\as\spec.h"
#include <string.h>

#define IR 0
#define UV 1
#define LS 2

long *oldnew(ok,nk)
   OKREC *ok;
   KREC  *nk;
   {
	memset(nk,'\0',sizeof(KREC));
   strncpy(nk->spname,ok->name,sizeof(nk->spname));
	nk->dtype = (UTINY)4;
   nk->npts = (LONG)swaps(ok->npts);
   nk->istart = (LONG)swaps(ok->nstrt) * 100L;
   nk->ifin = (LONG)swaps(ok->nfin) * 100L;
   nk->ndel = -swaps(ok->ndel);
   nk->flags = (ULONG)swaps(ok->nflags);
   nk->scale = (nk->flags & 1)? 30000L:20000L;  
   nk->miny = (LONG)swaps(ok->miny);
   nk->maxy = (LONG)swaps(ok->maxy);
   nk->naccs = ok->naccs;
   nk->absex = ok->absex;
	swapf(&nk->absex,1L);
   nk->nsmth = ok->nsmth;
   nk->instno = swaps(ok->instno);
   strnset(nk->ident,' ',72);
   strncpy(nk->ident,ok->ident,64);
/* date and time from orig sectff? */
	/* set extended krec for cuv */
	nk->ebytes = 256;
	/* have all the standard stuff, now do datatype specific */
	/* depends on knowledge of ALL instrument numbers! */
	switch (nk->instno)
		{
		case 280:
		case 983:
		case 780:
		case 580:
		case 505:    /* actually NIR, I think */
			nk->stype = IR;
			convir(ok,nk);
			break;
		/* LS */
		case 3003:
		case 3004:
		case 3005:
			nk->stype = LS;
			convls(ok,nk);
			break;
/************************** need UV! **
		case ??
			nk->stype = UV;
			convuv(ok,nk);
			break;
 ****************************************/
		default:
			/* set to "unknown" datatype */
			nk->stype = 4;
			break;
		}
		
	return((long*)(ok + sizeof(OKREC)));
   }
