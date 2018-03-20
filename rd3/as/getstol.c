/* getstol - converts 3600 short data to intel longs */

union {
	short s;
	char c[2];
	}j,k;
long cvstol(int i)
	{
	j.s=i;
	k.c[1] = j.c[0];
	k.c[0] = j.c[1];
	return((long)k.s);
	}

long getNext3sl()		/*get next 2 bytes & convert short to long */
	{
	 k.c[1] = dNextChar();
	 k.c[0] = dNextChar();
	 return((long)k.s);
	}

int getNext3s(void)		/*get next 2 bytes & return short */
	{
	 k.c[1] = dNextChar();
	 k.c[0] = dNextChar();
	 return(k.s);
	}

