#include <stdio.h>

main (argc, argv)
	int argc;
	char **argv;
{
	printf("path = %s; access(path) = %d\n", 
		    argv[1], 
			access(argv[1], 0));
}
