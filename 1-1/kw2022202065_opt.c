//////////////////////////////////////////////////////////////////////
// File Name	: kw2022202065_opt.c
// Date		: 2024/03/30
// OS		: Ubuntu 20.04.6 LTS 64bits
// Author	: Park Na Rim
// Student ID	: 2022202065
// -------------------------------------------------------------------
// Title : System Programming Assignment #1-1 (ftp server)
// Description : Parameter parsing (-a, -b, -c [str] by using getopt().
//////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv)
{
	int aflag = 0, bflag = 0;	// Flags for options -a and -b
	char *cvalue = NULL;		// Arguments for option -c
	int c = 0;	 		// Return value of getopt()
	opterr = 0;			// Error variable

	//===Repeat until there are options on the command line===//
	while((c = getopt(argc, argv, "abc:")) != -1)
	{
		switch (c)
		{
			case 'a':
				aflag++;
				break;
			case 'b':
				bflag++;
				break;
			case 'c':
				cvalue = optarg; // Save Option's argument value
				break;
			case '?':		// Unknown option character
				break;
		}
	}

	//==============Print flags and c's argument==============//
	printf("aflag = %d, bflag = %d, cvalue = %s\n", aflag, bflag, cvalue);

	//======Print non-option arguments remaining in argv======//
	while(optind < argc)
	{
		printf("Non-option argument %s\n", argv[optind++]);
	}

	return 0;
}
