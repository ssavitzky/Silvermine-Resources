/* zbufr - zeros n bytes of a buffer */
void zbufr(char *string, long n)
	{
	while (n--)
		*string++ = '\0';
	}
