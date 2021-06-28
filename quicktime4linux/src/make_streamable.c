#include "quicktime.h"

int main(int argc, char *argv[])
{
	int i;
	if(argc < 2 || argv[1][0] == '-')
	{
		printf("usage: %s <filename>\n", argv[0]);
		exit(1);
	}

	for(i = 1; i < argc; i++)
	{
		if(quicktime_make_streamable(argv[i]))
			exit(1);
	}

	return 0;
}
