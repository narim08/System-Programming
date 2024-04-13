////////////////////////////////////////////////////////////////////////
// File Name	: srv.c
// Date		: 2024/04/13
// OS		: Ubuntu 20.04.6 LTS 64bits
// Author	: Park Na Rim
// Student ID	: 2022202065
// ----------------------------------------------------------------- //
// Title: System Programming Assignment #1-3 (ftp server)
// Description : Server program that converts the FTP command received
//               from the client back into a user command. it then
//               executes the command and output the results.
//////////////////////////////////////////////////////////////////////

#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h> //stat()
#include <pwd.h> //getpwuid()
#include <grp.h> //getgrgid()
#include <time.h> //strftime()
#include <unistd.h> //read(), write(), access()
#include <stdio.h>
#include <string.h>
#include <stdlib.h> //exit()

#define MAX_BUF 100 //size of buffer

////////////////////////////////////////////////////////////////////
// errWrite
// ============================================================== //
// Input : char *errstr	- error text
// Output : x
// Purpose : Print an error
// /////////////////////////////////////////////////////////////////
void errWrite(char *errStr)
{
	write(1, errStr, strlen(errStr));
	exit(1);
}

///////////////////////////////////////////////////////////////////
// selSort
// ============================================================= //
// Input : char *dList[] - directory list array
// 	 : int cnt	 - number of directory list
// Output : x
// Purpose : Sort the directory list in ascending order
///////////////////////////////////////////////////////////////////
void selSort(char *dList[], int cnt)
{
	int i, j, min; //index
	char *temp;

	for(i=0; i<cnt-1; i++){ //Selection Sort
		min = i;
		for(j=i+1; j<cnt; j++){
			if(strcmp(dList[j], dList[min]) < 0){
				min = j;
			}
		}
		temp = dList[min];
		dList[min] = dList[i];
		dList[i] = temp;
	}
}

//////////////////////////////////////////////////////////////////
// lsCmd
// ============================================================ //
// Input : int aflag, lflag, alflag - ls options
// 	   char *arg	 	    - ls argument (path)
// Output : x
// Purpose : Function that implements the ls command(option a, l)
//////////////////////////////////////////////////////////////////
void lsCmd(int aflag, int lflag, int alflag, char *arg)
{
	DIR *dp;
	struct dirent *dirp;
	char *dname = arg;

	struct stat *buf; //dList
	struct stat bbuf; //dList[i]
	struct passwd *pwd; //User ID
	struct group *grp; //Group ID
	struct tm *mtime; //modified time
	char timeInfo[30]; //time information
	
	int cnt = 0; //file count
	char **dList; //directory list
	char printBuf[200]; //Buffer for print

	//=====================open directory===================//
	if((dp = opendir(dname)) == NULL) {
		if(stat(dname, buf) < 0) { //no exist directory
			errWrite("cannot access : No such directory\n");
		}
		else if(access(dname, R_OK) < 0) { //no access rights
			errWrite("cannot access : Access denied\n");
		}
	}
	
	//======================read directory==================//
	while((dirp = readdir(dp)) != NULL) { //counting directory list
		cnt++;
	}
	rewinddir(dp); //reset read position

	dList = (char**)malloc(sizeof(char*)* cnt); //dynamic allocation of directory list
	if(dList==NULL) {errWrite("empty directory");} //allocation error
	
	dirp = readdir(dp);
	for(int i=0; dirp!=NULL; i++){ //read again from the beginning and save to the list
		dList[i] = dirp->d_name;
		dirp = readdir(dp);
	}

	selSort(dList, cnt); //sorts the directory list by selection sort


	//=============Excutes the ls command by option=============//
	if(lflag==0 && alflag==0){ //no-option or -a
		strcpy(printBuf, ""); //print buffer initialization
		int j=1; //variables for print 4 or 5 per line

		for(int i = 0; i<cnt; i++) { //print the entire list
			if(aflag==0 && dList[i][0]=='.') continue; //no-option, ignore hidden files

			j++;
			if(!stat(dList[i], &bbuf)){ //distinguish between directory and file
				strcat(printBuf, dList[i]);
				if(S_ISDIR(bbuf.st_mode)==1){ //directory, add /
					strcat(printBuf, "/\t");
				}
				else { strcat(printBuf, "\t"); } //file
			}
			if(j%5==0) {strcat(printBuf, "\n");} //new line
		}
		strcat(printBuf, "\n");
		write(1, printBuf, strlen(printBuf)); //ls print
	}
	else { //-l, -al
		for(int i =0; i<cnt; i++) {
			if(lflag==1 && dList[i][0] == '.') continue; //-l, ignore hidden files

			if(!stat(dList[i], &bbuf)) { //print file properties
				if(S_ISREG(bbuf.st_mode)==1) {
					strcpy(printBuf, "-"); //regular file
				}
				else {
					strcpy(printBuf, "d"); //directory
				}

				//user permission 
				strcat(printBuf, ((bbuf.st_mode & S_IRUSR) ? "r" : "-")); //read
				strcat(printBuf, ((bbuf.st_mode & S_IWUSR) ? "w" : "-")); //write
				strcat(printBuf, ((bbuf.st_mode & S_IXUSR) ? "x" : "-")); //execute
				
				//group permission
				strcat(printBuf, ((bbuf.st_mode & S_IRGRP) ? "r" : "-")); //read
				strcat(printBuf, ((bbuf.st_mode & S_IWGRP) ? "w" : "-")); //write
				strcat(printBuf, ((bbuf.st_mode & S_IXGRP) ? "x" : "-")); //execute

				//other permission
				strcat(printBuf, ((bbuf.st_mode & S_IROTH) ? "r" : "-")); //read
				strcat(printBuf, ((bbuf.st_mode & S_IWOTH) ? "w" : "-")); //write
				strcat(printBuf, ((bbuf.st_mode & S_IXOTH) ? "x\t" : "-\t")); //execute
				
				//number of links
				sprintf(printBuf + strlen(printBuf), "%ld\t", bbuf.st_nlink);

				//user id of owner
				pwd = getpwuid(bbuf.st_uid);
				strcat(printBuf, pwd->pw_name);
				strcat(printBuf, "\t");

				//group id of owner
				grp = getgrgid(bbuf.st_gid);
				strcat(printBuf, grp->gr_name);
				strcat(printBuf, "\t");

				//size in byte
				sprintf(printBuf + strlen(printBuf), "%ld\t", bbuf.st_size);

				//time of last modification
				mtime = localtime(&bbuf.st_mtime);
				strftime(timeInfo, sizeof(timeInfo), "%m %d %H:%M ", mtime);
				strcat(printBuf, timeInfo);
				strcat(printBuf, "\t");

				//file name
				strcat(printBuf, dList[i]);
				if(S_ISDIR(bbuf.st_mode)==1){ //directory
					strcat(printBuf, "/\n");
				}
				else { strcat(printBuf, "\n"); }

			}
			write(1, printBuf, strlen(printBuf)); //ls print
		}
	}

	//===================close directory===================//
	if(closedir(dp) < 0) {
		errWrite("closed error\n");
	}
	free(dList); //free dynamic allocation
}

////////////////////////////////////////////////////////////////////////////
// parseCmd
// ====================================================================== //
// Input : char *buf	- FTP command line
// Output : x
// Purpose : Convert FTP command back to user command, separate command and
// 	     argument, execute command
///////////////////////////////////////////////////////////////////////////
void parseCmd(char *buf)
{
	char renameBuf[50]; //buffer replication for rename command
	strcpy(renameBuf, buf);

	buf[strcspn(buf, "\n")] = '\0'; //clear newline characters in buffer
	char *token = strtok(buf, " "); //command separation
	char *ftpCmd = token; //ftp command
	char userCmd[100]; //user command
	char printBuf[100]; //buffer for print
	char arg[100]; //save argument
	int aflag = 0, lflag = 0, alflag = 0; //ls options

	//==================NLST(ls)===================//
	if(strcmp(ftpCmd, "NLST")==0) {
		strcpy(userCmd, "ls"); //ftp command -> user command
		strcpy(arg, "."); //set default path to current directory

		while((token = strtok(NULL, " ")) != NULL) { //option, argument analysis
			if(token[0]=='-') { //option
				if (token[2]=='l'){ alflag++; } //-al
				else if (token[1]=='l'){ lflag++; } //-l
				else if (token[1]=='a'){ aflag++; } //-a
			}
			else { //no option, argument = path
				strcpy(arg, token);
			}
		}

		lsCmd(aflag, lflag, alflag, arg); //ls execute
	}
	
	//=================LIST(dir)==================//
	if(strcmp(ftpCmd, "LIST")==0) {
		strcpy(userCmd, "dir"); //ftp command -> user command
		strcpy(arg, "."); //set default path to current directory
		alflag++; //dir == ls -al

		while((token = strtok(NULL, " ")) != NULL) {
			if(token[0]=='-') {errWrite("Error: invalid option\n");}
			strcpy(arg, token);
		}

		lsCmd(aflag, lflag, alflag, arg); //ls -al execute
	}
	//================PWD(pwd)==================//
	if(strcmp(ftpCmd, "PWD")==0) {
		char cwdBuf[1024]; //path
		
		if((token = strtok(NULL, " ")) != NULL) { //error handling
			if(token[0]=='-') {errWrite("Error: invalid option\n");}
			else {errWrite("Error: argument is not required\n");}
		}
		if(getcwd(cwdBuf, sizeof(cwdBuf)) != NULL) { //get current path
			strcpy(printBuf, "\"");
			strcat(printBuf, cwdBuf);
			strcat(printBuf, "\" is current directory\n");
			write(1, printBuf, strlen(printBuf));
		}
	}
	//==================CWD(cd)=================//
	if(strcmp(ftpCmd, "CWD")==0 || strcmp(ftpCmd, "CUDP")==0) {
		char cdBuf[1024];
		char path[1024];
		if((token = strtok(NULL, " ")) != NULL) {
			if(token[0]=='-') {errWrite("Error: invalid option\n");}
			else { 
				strcpy(cdBuf, token); //save path

				if(chdir(cdBuf)!=-1) { //change path
					if(getcwd(path, sizeof(path))!=NULL) { //get current path
						strcpy(printBuf, "\"");
						strcat(printBuf, path);
						strcat(printBuf, "\" is current directory\n");
						write(1, printBuf, strlen(printBuf));
					}
				}
				else {errWrite("Error: directory not found\n");}	
			}
		}

	}
	//=================MKD(mkdir)===============//
	if(strcmp(ftpCmd, "MKD")==0) {
		char mkBuf[50];
		struct stat st;

		while((token = strtok(NULL, " ")) != NULL) {
			if(token[0]=='-') {errWrite("Error: invalid option\n");}
			else{
				strcpy(mkBuf, token); //directory name
				if(stat(mkBuf, &st)==0) { //directory exists
					sprintf(printBuf, "Error: cannot create directory '%s': File exists\n", mkBuf);
					write(1, printBuf, strlen(printBuf));
				}
				else if(mkdir(mkBuf, 0775)==-1) { //create directory, drwxrwxr-x
					errWrite("Error: mkdir error\n");
				}
			}
		}

	}
	//===============DELE(delete)==============//
	if(strcmp(ftpCmd, "DELE")==0) {
		char delFile[50];
		
		while((token = strtok(NULL, " ")) != NULL) {
			if(token[0]=='-') {errWrite("Error: invalid option\n");}
			else {
				strcpy(delFile, token); //File name
				if(unlink(delFile) == -1) { //delete file
					sprintf(printBuf, "Error: failed to delete '%s'\n", delFile);
					write(1, printBuf, strlen(printBuf));
				}
			}
		}
	}
	//=================RMD(rmdir)==============//
	if(strcmp(ftpCmd, "RMD")==0) {
		char delDir[50];
	
		while((token = strtok(NULL, " ")) != NULL) {
			if(token[0]=='-') {errWrite("Error: invalid option\n");}
			else {
				strcpy(delDir, token);
				if(rmdir(delDir)==-1) { //remove directory
					sprintf(printBuf, "Error: falied to remove '%s'\n", delDir);
					write(1, printBuf, strlen(printBuf));
				}
			}
		}
	}//===============RNFR & RNTO==============//
	if(strcmp(ftpCmd, "RNFR")==0) {
		struct stat st;
		char oldName[50], newName[50], rBuf[50];
		
		char *retok;
		retok = strtok(renameBuf, " "); //cmd

		retok = strtok(NULL, " ");
		strcpy(oldName, retok); //oldName\nRNTO
		char* nullFind = strchr(oldName, '\n'); //delete newline character
		if(nullFind != NULL) {
			*nullFind = '\0'; //oldName
		}

		retok = strtok(NULL, " "); //newName
		strcpy(newName, retok);
		newName[strchr(newName, '\n') - newName] = '\0'; //delete newline character
		
		if(stat(newName, &st)==0) { //file exists
			sprintf(printBuf, "Error: name to change already exists\n");
			write(1, printBuf, strlen(printBuf));
			exit(1);
		}

		if(rename(oldName, newName)==-1) { //Rename directory & file
			errWrite("Error: rename() error\n");
		}
	}
	//===============QUIT(quit)===============//
	if(strcmp(ftpCmd, "QUIT")==0) {
		if((token = strtok(NULL, " ")) != NULL) {
			if(token[0]=='-') {errWrite("Error: invalid option\n");}
			else {errWrite("Error: argument is not required\n");}
		}
		strcpy(printBuf, "QUIT success\n"); ///normal termination of program
		write(1, printBuf, strlen(printBuf));
		exit(0);
	}
}

int main(int argc, char*argv[])
{
	int n = 0;
	char buf[MAX_BUF];
	
	ssize_t readCheck = read(0, buf, MAX_BUF); //read standard output
	if(readCheck == -1) {
		printf("Error: read error\n");
		exit(1);
	}

	write(1, buf, strlen(buf)); //print buffer

	parseCmd(buf); //Execute after separating commands

	return 0;
}
