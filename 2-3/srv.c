////////////////////////////////////////////////////////////////////////
// File Name	: srv.c
// Date		: 2024/05/11
// OS		: Ubuntu 20.04.6 LTS 64bits
// Author	: Park Na Rim
// Student ID	: 2022202065
// ----------------------------------------------------------------- //
// Title: System Programming Assignment #2-3 (ftp server)
// Description : This server program connects to the client and creates
// 		a new process. The parent process outputs client
// 		information, and the child process transmits the
// 		received string. 
///////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <signal.h>

#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h> //stat()
#include <pwd.h> //getpwuid()
#include <grp.h> //getgrgid()
#include <time.h> //strftime()

#define INTERVAL 10
#define BUF_SIZE 256

void sh_chld(int); //signal handler for SIGCHLD
void sh_alrm(int); //signal handler for SIGALRM
void sh_int(int); //signal handler for SIGINT
void sh_alrm_info(int);

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

////////////////////////////////////////////////////////////////////
// client_info
// ============================================================== //
// Input : struct sockaddr_in *client_addr - socket information
// Output : 0 - Normal completion
// Purpose : Print client's IP and port information.
// /////////////////////////////////////////////////////////////////
int client_info(struct sockaddr_in *client_addr) 
{
	char ipAddr[20]; //ip address
	//convert to dot ip format
	inet_ntop(AF_INET, &(client_addr->sin_addr), ipAddr, 20);
	printf("==========Client info==========\n");
	printf("client IP: %s\n\n", ipAddr);
	printf("client port: %d\n", ntohs(client_addr->sin_port));
	printf("===============================\n");
	return 0;
}

typedef struct {
	pid_t pid;
	int port;
	time_t serviceTime;
} clientInfo;



void currentCliInfo(clientInfo cliInfo[], int clientNum) 
{
	printf("Current Number of Client: %d\n", clientNum);
	printf(" PID\tPORT\tTIME\n");
	for (int i=0; i<clientNum; i++) {
		printf("%d\t%d\t%ld\n", cliInfo[i].pid, cliInfo[i].port, time(NULL) - cliInfo[i].serviceTime);
	}
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
// 	   char *result_buff	    - save result
// Output : x
// Purpose : Function that implements the ls command(option a, l, al)
//////////////////////////////////////////////////////////////////
void lsCmd(int aflag, int lflag, int alflag, char *arg, char *result_buff)
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
		strcpy(result_buff, printBuf); //save ls result
	}
	else { //-l, -al
		strcpy(result_buff, "");
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

				strcat(result_buff, printBuf); //save ls result
			}
		}
	}
	
	int reslen = strlen(result_buff);
	result_buff[reslen] = '\0'; //set string end

	//===================close directory===================//
	if(closedir(dp) < 0) {
		errWrite("closed error\n");
	}
	free(dList); //free dynamic allocation
}

////////////////////////////////////////////////////////////////////////////
// parseCmd
// ====================================================================== //
// Input : char *buf		- FTP command line
// 	   char *result_buff	- save result buffer
// Output : x
// Purpose : Convert FTP command back to user command, separate command and
// 	     argument, execute command
///////////////////////////////////////////////////////////////////////////
void parseCmd(char *buf, char *result_buff)
{
	char renameBuf[50]; //buffer replication for rename command
	strcpy(renameBuf, buf);

	buf[strcspn(buf, "\n")] = '\0'; //clear newline characters in buffer
	char *token = strtok(buf, " "); //command separation
	char *ftpCmd = token; //ftp command
	
	char userCmd[100]; //user command
	char printBuf[1024]; //buffer for print

	char arg[100]; //save argument
	int aflag = 0, lflag = 0, alflag = 0; //ls options

	strcpy(printBuf, "");
	strcpy(result_buff, "\n");

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

		lsCmd(aflag, lflag, alflag, arg, result_buff); //ls execute
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

		lsCmd(aflag, lflag, alflag, arg, result_buff); //ls -al execute
	}
	//================PWD(pwd)==================//
	if(strcmp(ftpCmd, "PWD")==0) {
		char cwdBuf[1024]; //path

		if((token = strtok(NULL, " ")) != NULL) { //error handling
			if(token[0]=='-') {errWrite("Error: invalid option\n");}
			else {errWrite("Error: argument is not required\n");}
		}
		if(getcwd(cwdBuf, sizeof(cwdBuf)) != NULL) { //get current path
			sprintf(result_buff, "\"%s\" is current directory\n", cwdBuf);
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
						sprintf(result_buff, "\"%s\" is current directory\n", path);
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
					sprintf(result_buff, "Error: cannot create directory '%s': File exists\n", mkBuf);
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
					sprintf(result_buff, "Error: failed to delete '%s'\n", delFile);
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
					sprintf(result_buff, "Error: falied to remove '%s'\n", delDir);
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
			sprintf(result_buff, "Error: name to change already exists\n");
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
		strcpy(result_buff, "QUIT");
	}
	
	//==========save command result===========//
}

int clientNum=0;
clientInfo cliInfo[50]; //save current client's information
int chPid[50];

int main(int argc, char **argv) 
{
	char buff[BUF_SIZE], result_buff[1024];
	int n;
	struct sockaddr_in server_addr, client_addr;
	int server_fd, client_fd;
	int len;
	int port;
	time_t timeStart = time(NULL);

	signal(SIGCHLD, sh_chld);	//applying signal handler(sh_alrm) for SIGALRM
	signal(SIGALRM, sh_alrm);	//applying signal handler(sh_chld) for SIGCHLD
	signal(SIGINT, sh_int);
	signal(SIGALRM, sh_alrm_info);

	server_fd = socket(PF_INET, SOCK_STREAM, 0);	//creat socket
	
	//set server addr
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;		//set family
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);//set ip address
	server_addr.sin_port = htons(atoi(argv[1]));	//set port

	//bind: connect address to socket
	bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));

	//listen to client requests
	listen(server_fd, 5);

	//============connect to client, read and execute==============//
	while(1) {
		pid_t pid; //process ID
		len = sizeof(client_addr);
		client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &len);

		//fork
		if((pid=fork())<0) { 	//fork error
			printf("Error: fork() error!!\n");
			exit(1);
		}
		else if(pid==0) {	//child process
			close(server_fd);
			printf("Child Process ID: %d\n", getpid());
			
			memset(buff, 0, sizeof(buff));

			while((n=read(client_fd, buff, BUF_SIZE))>0) { //read client's string
				
				printf("> %s\t[%d]\n", buff, getpid()); //print client's commands
				memset(result_buff, 0, sizeof(result_buff));
				parseCmd(buff, result_buff); //Execute after separating commands

				write(client_fd, result_buff, strlen(result_buff)); //send to client

				if(!strcmp(buff, "QUIT")) { //program quit
					/*for(int i=0; i<clientNum; ++i) {
						if(cliInfo[i].pid==getpid()) {
							for(int j=i; j<clientNum-1; ++j) {
								cliInfo[j] = cliInfo[j+1];
							}
							clientNum--;
							break;
						}
					}*/
					close(client_fd);
					close(server_fd);
					sh_alrm(1); //1 second
				}
				memset(buff, 0, sizeof(buff)); //buffer clear!!
			}
			close(client_fd);
			exit(0);
		}	
		else { 			//parent process	
			if(client_info(&client_addr) < 0) { //display client ip and port
				printf("Error: client_info error!!\n");
			}

			chPid[clientNum] = pid;
			cliInfo[clientNum].pid = pid;
			cliInfo[clientNum].port = ntohs(server_addr.sin_port);
			cliInfo[clientNum].serviceTime = time(NULL);
			clientNum++;
			currentCliInfo(cliInfo, clientNum); //prinit
			alarm(10);
		}

		close(client_fd);
	}//end while
	close(server_fd);
	return 0;
}


////////////////////////////////////////////////////////////////////
// sh_chld
// ============================================================== //
// Input : int signum - signal number
// Output : x
// Purpose : Handles terminated child processes.
// /////////////////////////////////////////////////////////////////
void sh_chld(int signum) {
	printf("Status of Child process was changed.\n");
	pid_t pid;
	int status;
	while((pid=waitpid(-1, &status, WNOHANG)) >0) {
		for(int i=0; i<clientNum; ++i) {
			if(cliInfo[i].pid == pid) {
				for(int j=i; j<clientNum-1; ++j) {
					cliInfo[j] = cliInfo[j+1];
				}
				clientNum--;
				break;
			}
		}
	}
	//wait(NULL); //Parent process waits for child process to terminate
}


////////////////////////////////////////////////////////////////////
// sh_alrm
// ============================================================== //
// Input : int signum - signal number
// Output : x
// Purpose : Notifies process timeout
// /////////////////////////////////////////////////////////////////
void sh_alrm(int signum) {
	printf("Client ( %d)'s Release\n", getpid());
	exit(1);
}

void sh_alrm_info(int signum) {
	currentCliInfo(cliInfo, clientNum);
	alarm(10);
}

void sh_int(int signum) {
	printf("server whil be terminated.\n");
	for(int i=0; i<clientNum; ++i){
		kill(chPid[i], SIGTERM);
		waitpid(chPid[i], NULL, 0);
	}
	exit(0);
}

