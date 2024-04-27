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

#define MAX_BUFF 1024
#define SEND_BUFF 1024


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


int client_info(struct sockaddr_in *client_addr) 
{
	char ipAddr[20];
	inet_ntop(AF_INET, &(client_addr->sin_addr), ipAddr, 20);
	printf("==========Client info==========\n");
	printf("client IP: %s\n\n", ipAddr);
	printf("client port: %d\n", ntohs(client_addr->sin_port));
	printf("===============================\n");
	return 0;
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
// Purpose : Function that implements the ls command(option a, l)
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
				
				strcat(result_buff, printBuf);
			}
		}
	}
	
	int reslen = strlen(result_buff);
	result_buff[reslen] = '\0';

	//===================close directory===================//
	if(closedir(dp) < 0) {
		errWrite("closed error\n");
	}
	free(dList); //free dynamic allocation
}



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


int main(int argc, char **argv)
{
	char buff[MAX_BUFF], result_buff[SEND_BUFF];
	int n;

	//===========open socket and listen===============//
	struct sockaddr_in server_addr, client_addr;
	int sockfd, connfd;
	int len, len_out;
	int port = atoi(argv[1]);

	if((sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		write(STDERR_FILENO, "Server: Can't open stream socket.\n", 50);
		exit(1);
	}

	//set server addr
	//bzero((char*)&server_addr, sizeof(server_addr));
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(port);	
	
	//bind
	if(bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
		write(STDERR_FILENO, "Server: Can't bind local address.\n", 50);
		exit(1);
	}
	
	listen(sockfd, 5);
	while(1) {
		len = sizeof(client_addr);
		connfd = accept(sockfd, (struct sockaddr *)&client_addr, &len);
		if(connfd < 0) {
			write(STDERR_FILENO, "Server: accept failed.\n", 50);
			exit(1);
		}
		if(client_info(&client_addr) < 0) { //display client ip and port
			write(STDERR_FILENO, "Error: client_info() error!!\n", 50);
		}

		while(1) {
			n = read(connfd, buff, MAX_BUFF); //read FTP command
			buff[n] = '\0';

			if(cmd_process(buff, result_buff) < 0) { //cmd execute
				write(STDERR_FILENO, "Error: cmd_process() error!!\n", 50);
				break;
			}
			write(connfd, result_buff, strlen(result_buff)); //send to client

			if(!strcmp(result_buff, "QUIT")) {
				write(STDOUT_FILENO, "QUIT\n", 5);
				close(connfd);
				close(sockfd);
				return 0;
			}
		}
	}
	close(sockfd);
	return 0;
}
