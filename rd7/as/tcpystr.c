/* test of copystr */
#include <stdio.h>
#include <std.h>
TEXT string1[]="string1";
TEXT string2[]="string2";
TEXT string3[]="string3";

main()
	{
	int i,j;
	TEXT *s;
	TEXT string[80];

	s=cpystr(string,string1,string2,"\n",NULL);
	i = printf("%s\n",string);
	s=cpystr(s,string3,"\n",NULL);
	j = printf("%s\n",string);
	printf("i = %i, j= %i\n",i,j);
	}




