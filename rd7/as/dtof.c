/* dtof.c - simulates idris dtof function */
/* converts double number, dbl, to string, starting at s, having 
** the format: [-]d*.d*
**  p specifies max no. of characters to left of decimal pt
**  q specifies actual number to right.
**  returns actual number of chars in s
**    if dec + q +2 >p, fills field with * and returns p
*/
#include <float.h>
#include <stdlib.h>

int dtof(char *s,double dbl,int p, int q)
	{
	register char *s1,*s2;
	char c;
	int dec,sign,i,n;
	
	s2 = fcvt(dbl,q,&dec,&sign);

	if ( abs(dec) > p)		/*number too big for field, fill with * */
		{
		for (i = 0; i < p; i++)
			*s++ = '*';
		return(p);
		}

	s[0] = sign ? '-' : ' ';	/* move in sign or blank for first char*/
	n = 1;
	s1 = &s[1];
	
	if (dec > 0)
		{
		while (dec--)
			{
			*s1++ = *s2++;
			n++;
			}
		*s1++= '.';
		n++;
		}
	else
		{
		*s1++='.';
		n++;
		while (dec++)
			*s1++ = '0';
			n++;
		}

	/* move in fraction */
		for (i=0;i<q;i++)
			{
			*s1++ = *s2++;
			n++;
			}
		return(n);
	}

