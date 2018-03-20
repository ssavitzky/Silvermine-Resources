/* nextitem.c - returns next flipped long or float  */

int dNextChar(void);
union {
	int y;
	char c[2];
	} cint;

long gNextLong(void)
	{
	int i;
	union {
		long x;
		char c[4];
		} item;

	for (i = 0; i < 4; i++)
		{
		if ((cint.y = dNextChar()) == -1)
			return(0L);		/* next level has to check EOF ! */
		item.c[3-i] = cint.c[1];
		}
	return(item.x);
	}

float gNextFloat(void)
	{
	int i;
	union {
		float x;
		char c[4];
		} item;

	for (i = 0; i < 4; i++)
		{
		if ((cint.y = dNextChar()) == -1)
			return(0L);		/* next level has to check EOF ! */
		item.c[3-i] = cint.c[0];
		}
	return(item.x);
	}

