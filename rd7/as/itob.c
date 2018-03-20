/* itob - simulates idris integer to ascii conversion */
/* converts long to ascii in buffer in radix number system
** returns number of bytes in buffer
*/

#include <stdlib.h>
#include <string.h>

int itob(char *buffer,long value,
	int radix)		 /* number system, always 10 in this work */
	{
	int r = 10;		/* force it */
	return(strlen(ltoa(value,buffer,r)));
	}
