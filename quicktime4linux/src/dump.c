#include "quicktime.h"







main(int argc, char *argv[])
{
	quicktime_t *file;
	int result = 0;
	
	if(argc < 2)
	{
		printf("Need a movie.\n");
		exit(1);
	}

	if(!(file = quicktime_open(argv[1], 1, 0)))
	{
		printf("Open failed\n");
		exit(1);
	}

	quicktime_dump(file);

	//while(!result)
	//{
	//	result = quicktime_read_atom(file);
	//	if(!result)
	//	{
	//		quicktime_skip_atom(file);
	//		printf("%c%c%c%c %ld\n", file->atom.type[0], file->atom.type[1], file->atom.type[2], file->atom.type[3], file->atom.end);
	//	}
	//};
	
	quicktime_close(file);
}
