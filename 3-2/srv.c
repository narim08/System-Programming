////////////////////////////////////////////////////////////////////////
// File Name	: srv.c
// Date		: 2024/05/26
// OS		: Ubuntu 20.04.6 LTS 64bits
// Author	: Park Na Rim
// Student ID	: 2022202065
// ----------------------------------------------------------------- //
// Title: System Programming Assignment #3-1 (ftp server)
// Description : Server program that converts the FTP command received
//               from the client back into a user command. it then
//               executes the command and send the results to client.
///////////////////////////////////////////////////////////////////////

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
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> //inet_ntop()

#define MAX_BUF 100
#define SEND_BUF 1024


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

		//save ls result
		strcpy(result_buff, printBuf);
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


///////////////////////////////////////////////////////////////////
// cmd_process
// ============================================================= //
// Input : char *buff	 	- ftp command line buffer
// 	 : char *result_buff	- save result buffer
// Output : 0			- normal completion
// Purpose : Analyze and execute FTP command
///////////////////////////////////////////////////////////////////
int cmd_process(char* buff, char* result_buff)
{
	char arg[100];				//ls argument(path)
	int aflag=0, lflag=0, alflag=0;		//ls option(-a, -l, -al)

	char *token = strtok(buff, " ");	//command
	char *cmd = token;

	if(strcmp(cmd, "QUIT")==0) {		//quit command
		strcpy(result_buff, "QUIT");
		return 0;
	}
	else { 					//ls command execute
		write(STDOUT_FILENO, buff, sizeof(buff));
		write(STDOUT_FILENO, "\n", 1);

		strcpy(arg, "."); 		//set default path to current directory

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
	return 0;
}


int exe_client(int control_sockfd)
{
	char buff[MAX_BUF], result_buff[SEND_BUF];
	int n;
	memset(buff, 0, sizeof(buff));
	memset(result_buff, 0, sizeof(result_buff));

	if ((n = read(control_sockfd, buff, MAX_BUF)) > 0) {
        	buff[n] = '\0';
        	printf("%s\n", buff);
		
		// Send ACK
       		write(control_sockfd, "200 Port command successful", MAX_BUF);
		printf("200 port command successful\n");

        	// Extract IP and port from PORT command
        	unsigned int ip[4];
        	int p1, p2, dataPort;
        	sscanf(buff, "PORT %u,%u,%u,%u,%d,%d", &ip[0], &ip[1], &ip[2], &ip[3], &p1, &p2);
        	char ip_str[INET_ADDRSTRLEN];
		snprintf(ip_str, sizeof(ip_str), "%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);
        	dataPort = p1 * 256 + p2;
		
		//================data connection=============//
		sleep(2);
        	int data_sockfd;
        	struct sockaddr_in data_addr;
        	if ((data_sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
            		printf("Error: can't creat data socket\n");
            		return -1;
       		 }

		//set
		memset(&data_addr, 0, sizeof(data_addr));
       		data_addr.sin_family = AF_INET;
        	data_addr.sin_addr.s_addr = inet_addr(ip_str);
		data_addr.sin_port = htons(dataPort);
		printf("ip: %s / port: %d\n", inet_ntoa(data_addr.sin_addr), ntohs(data_addr.sin_port));

		if(connect(data_sockfd, (struct sockaddr *) &data_addr, sizeof(data_addr)) < 0) {
            		//printf("Error: can't connect\n");
            		perror("Error: can't connect");
			close(data_sockfd);
            		return -1;
       		 }
		
		memset(buff, 0, sizeof(buff));
		memset(result_buff, 0, sizeof(result_buff));
		if((n = read(data_sockfd, buff, MAX_BUF))>0) {
			buff[n]='\0';
			if(cmd_process(buff, result_buff) < 0) { //cmd execute
				write(STDERR_FILENO, "Error: cmd_process() error!!\n", 50);
				exit(1);
			}
			write(data_sockfd, result_buff, strlen(result_buff)); //send to client
		}
	
	close(data_sockfd);
	
	}
	
	return 0;
}


int main(int argc, char *argv[]) 
{
	int control_sockfd, cli_sockfd;
	struct sockaddr_in serv_addr, cli_addr;
	socklen_t cli_addr_len = sizeof(cli_addr);
	int port = atoi(argv[1]);
	
	if ((control_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        	printf("Error: can't creat socket\n");
        	exit(1);
    	}

	//set
	serv_addr.sin_family = AF_INET;
    	serv_addr.sin_addr.s_addr = INADDR_ANY;
    	serv_addr.sin_port = htons(port);

	//bind: connect address to socket
	if(bind(control_sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
		printf("Server: Can't bind local address.\n");
		exit(1);
	}

	//listen to client requests
	listen(control_sockfd, 5);

	while(1) {
		if ((cli_sockfd = accept(control_sockfd, (struct sockaddr *)&cli_addr, &cli_addr_len)) < 0) {
            	printf("Error: can't accept\n");
            	continue;
        	}
		int exeN;
        	if((exeN = exe_client(cli_sockfd) < 0)) {
			printf("Error: exe_client error\n");
			exit(1);		
		}
        	close(cli_sockfd);
	}

	close(control_sockfd);
	return 0;
}
