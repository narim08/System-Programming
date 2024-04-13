////////////////////////////////////////////////////////////////////////////////////////////////////////
// File Name	: kw2022202065_ls.c
// Date		: 2024/04/05
// Os		: Ubuntu 20.04.6 LTS 64bits
//
// Author	: Park Na Rim
// Student ID	: 2022202065
// ---------------------------------------------------------------------------------------------------//
// Title : System Programming Assignment #1-2 ( ftp server )
// Description : This is a program that output directory information using the opendir(), readdir() and closedir() functions.
////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h> //exit()
#include <unistd.h> //access()
#include <sys/stat.h> //stat()

int main(int argc, char *argv[])
{
	DIR *dp;
	struct dirent *dirp;
	char *dname;		// Directory name
	struct stat *buf;	// Directory information
	
	// Process by number of arguments
	if (argc < 2)		// Current directory
		dname = ".";
	else if (argc > 2) {	// Error
		printf("only one directory path can be processed\n");
		exit(1);
	}
	else
		dname = argv[1];


	//=================================Open directory====================================//
	if((dp = opendir(dname)) == NULL) {	// Returns a pointer to the input directory stream
		// Error handling
		if(stat(dname, buf) < 0) {	// Directory does not exist
			printf("cannot access '%s' : No such directory\n", dname);
			exit(1);
		}
		else if(access(dname, R_OK) < 0) { // No read permission
			printf("cannot access '%s' : Access denied\n", dname);
			exit(1);
		}
	}
	
	//================================Read directory=====================================//
	while((dirp = readdir(dp)) != NULL) { // Read files one by one until end of directory stream
		printf("%s\n", dirp->d_name);
	}
	
	//================================Close directory====================================//
	if(closedir(dp) < 0)			// On error, -1 is returned
		printf("closed error!\n");

	return 0;
}
