/* flips.c - converts idris long/short to dos */

#include <string.h>
#include <std.h>
#include <stdlib.h>

/**** DEFINE SWAP_S **********************/
#define SWAP_S(w)   (w = swaps(w))

#if 0 /* All we need out of this is swapf */
int swaps(int i)
	{
	union {
		int k;
		char c[2];
		}j;
	char ctemp;

	j.k = i;
	ctemp = j.c[0];
	j.c[0]=j.c[1];
	j.c[1]=ctemp;
	return(j.k);
	}

void swapl(LONG *lptr, long n)
	{
	long i,k;
	int j;
	union {
		LONG ll;
		int ss[2];
			}l;	
		int stemp;
	char *sptr;
	sptr = (char *)lptr;
	k = 4*n;
	for (i = 0L,j=30000;i < k;i+=30000)
		{
		if ((k - i)<30000)
			j = k-i;
		swab(sptr+i,sptr+i,j);
		}

	for (; n;n--)
		{
		l.ll = *lptr;
		stemp = l.ss[0];
		l.ss[0]=l.ss[1];
		l.ss[1]=stemp;
		*lptr++ = l.ll;
		}
	}
#endif

/* swapf is the same as swapl with variables changed to float */
void swapf(float *fptr, long n)
	{
	long i,k;
	int j;
	union {
		float ll;
		int ss[2];
			}l;	
	int stemp;
	char *sptr;
	sptr = (char *)fptr;
	k = 4*n;
	for (i = 0L,j=30000;i < k;i+=30000)
		{
		if ((k - i)<30000)
			j = k-i;
		swab(sptr+i,sptr+i,j);
		}

	for (; n;n--)
		{
		l.ll = *fptr;
		stemp = l.ss[0];
		l.ss[0]=l.ss[1];
		l.ss[1]=stemp;
		*fptr++ = l.ll;
		}
	}




