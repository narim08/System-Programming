///////////////////////////////////////////////////////////////////////////////
// File Name	: cli.c
// Data		: 2024/04/13
// OS		: Ubuntu 20.04.6 LTS 64bits
// Author	: Park Na Rim
// Student ID	: 2022202065
// -------------------------------------------------------------------------//
// Title : System Programming Assignment #1-3 (ftp server)
// Description : This is a client program that converts user commands into
//               ftp commands and sends them to the server.
/////////////////////////////////////////////////////////////////////////////


#include <unistd.h> //write()
#include <stdio.h>
#include <string.h>
#include <stdlib.h> //exit()

////////////////////////////////////////////////////////////////////////////
//errArg
//========================================================================//
//Input: x
//Output: x
//Purpose: Program ends after error print
////////////////////////////////////////////////////////////////////////////
void errArg()
{
	printf("Error: another argument is required\n");
	exit(1);
}


////////////////////////////////////////////////////////////////////////////
//converFTP
//========================================================================//
//Input: int argc	- number of arguments
//       char *argv[]	- save arguments
//Output: char*		- FTP command return
//Purpose: Converts user commands to FTP commands
//////////////////////////////////////////////////////////////////////////// 
char* convertFTP(int argc, char *argv[])
{
	char *userCmd = argv[1];
	static char ftpCmd[100];
	int opt = 0;

	//======================ls -> NLST=====================//
	if (strcmp(userCmd, "ls")==0) {
		int aflag =0, lflag = 0; //-a, -l, -al
		strcpy(ftpCmd, "NLST");
		while((opt=getopt(argc, argv, "al")) != -1) {
			switch(opt) {
				case 'a':
					aflag++; //-a
					break;
				case 'l':
					lflag++; //-l
					break;
				case '?':
					printf("Error: invalid option\n");
					exit(1);
			}
		}
		if(aflag!=0 && lflag==0) {
			strcat(ftpCmd, " -a");
		}
		else if(aflag==0 && lflag!=0) {
			strcat(ftpCmd, " -l");
		}
		else if(aflag!=0 && lflag!=0) {
			strcat(ftpCmd, " -al");
		}
		else { //No option -> argument(paht)
			for(int i = 2; i<argc; i++) { //Combining FTP commands and arguments
				strcat(ftpCmd, " ");
				strcat(ftpCmd, argv[i]);
				return ftpCmd;
			}
		}
		for(int i = 3; i<argc; i++) { //Combining FTP commands, option and arguments
			strcat(ftpCmd, " ");
			strcat(ftpCmd, argv[i]);
		}
		return ftpCmd;
	}
	//======================dir -> LIST=====================//
	else if (strcmp(userCmd, "dir")==0) {
		strcpy(ftpCmd, "LIST");
	}
	//======================pwd -> PWD=====================//
	else if (strcmp(userCmd, "pwd")==0) {
		strcpy(ftpCmd, "PWD");
	}
	//======================cd -> CD & CUDP=====================//
	else if (strcmp(userCmd, "cd")==0) {
		if (strcmp(argv[2], "..")==0) {
			strcpy(ftpCmd, "CDUP");
		}
		else {
			strcpy(ftpCmd, "CWD");
		}
		if(argv[2]==NULL) errArg(); //no path, error

	}
	//======================mkdir -> MKD=====================//
	else if (strcmp(userCmd, "mkdir")==0) {
		strcpy(ftpCmd, "MKD");
		if(argv[2]==NULL) errArg(); //no directory name, error
	}
	//=====================delete -> DELE=====================//
	else if (strcmp(userCmd, "delete")==0) {
		strcpy(ftpCmd, "DELE");
		if(argv[2]==NULL) errArg(); //no file name, error
	}
	//======================rmdir -> RMD=====================//
	else if (strcmp(userCmd, "rmdir")==0) {
		strcpy(ftpCmd, "RMD");
		if(argv[2]==NULL) errArg(); //no directory name, error
	}
	//=================rename -> RNFR & RNTO==================//
	else if (strcmp(userCmd, "rename")==0) {
		if(argv[2]==NULL) errArg(); //no change name, error
		else {
			strcpy(ftpCmd, "RNFR ");
			strcat(ftpCmd, argv[2]);
			strcat(ftpCmd, "\nRNTO "); //Add newline
			strcat(ftpCmd, argv[3]);
			return ftpCmd;
		}
	}
	//======================quit -> QUIT=====================//
	else if (strcmp(userCmd, "quit")==0) {
		strcpy(ftpCmd, "QUIT");
	}
	//=====================Command Error=====================//
	else {
		printf("Error: invalid command\n");
		exit(1);
	}
	
	//=============Combining commands and arguments==========//
	for(int i=2; i<argc; i++)
	{
		strcat(ftpCmd, " ");
		strcat(ftpCmd, argv[i]);
	}
	return ftpCmd;
}


int main(int argc, char *argv[])
{
	char buf[100];
	strcpy(buf, convertFTP(argc, argv)); //User command -> FTP command
	write(1, buf, strlen(buf)); //Standard output
	printf("\n");

	return 0;
}
