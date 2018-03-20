/* cpystr-copy string idris-like function */
#include <stdarg.h>
#include <string.h>

char *cpystr(char *ds,...)
	/* ds = destination string */
	{
	va_list argptr;
	register char *dest;
	register char *source;

	dest = ds;
	*dest = '\0';		/* start with empty string */
	va_start(argptr,ds);
	while (source = va_arg(argptr,char*))
		strcat(dest,source);
	return (ds + strlen(ds));
	}

