///////////////////////////////////////////////////////////////////////////////
// File Name	: cli.c
// Data		: 2024/05/12
// OS		: Ubuntu 20.04.6 LTS 64bits
// Author	: Park Na Rim
// Student ID	: 2022202065
// -------------------------------------------------------------------------//
// Title : System Programming Assignment #2-3 (ftp server)
// Description : This is a client program that converts user commands into
//               ftp commands and sends them to the server.
/////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <signal.h>

#define BUF_SIZE 256

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
//Input: char *buff	- user command line
//       char *cmd_buff - save converted FTP command
//Output: x
//Purpose: Converts user commands to FTP commands
//////////////////////////////////////////////////////////////////////////// 
void convertFTP(char* buff, char* cmd_buff)
{
	char * token = strtok(buff, " "); //user command
	char *userCmd = token;

	//======================ls -> NLST=====================//
	if (strcmp(userCmd, "ls")==0) {
		strcpy(cmd_buff, "NLST"); //FTP command
		while((token = strtok(NULL, " "))!=NULL) { //option. argument parse
			strcat(cmd_buff, " ");
			strcat(cmd_buff, token);
		}
	}
	//======================dir -> LIST=====================//
	else if (strcmp(userCmd, "dir")==0) {
		strcpy(cmd_buff, "LIST ");
		token = strtok(NULL, " "); //path or name
		strcat(cmd_buff, token);
	}
	//======================pwd -> PWD=====================//
	else if (strcmp(userCmd, "pwd")==0) {
		strcpy(cmd_buff, "PWD");
	}
	//======================cd -> CD & CUDP=====================//
	else if (strcmp(userCmd, "cd")==0) {
		token = strtok(NULL, " ");
		if (strcmp(token, "..")==0) {
			strcpy(cmd_buff, "CDUP ..");
		}
		else {
			strcpy(cmd_buff, "CWD ");
			strcat(cmd_buff, token);
		}
		if(token==NULL) errArg(); //no path, error

	}
	//======================mkdir -> MKD=====================//
	else if (strcmp(userCmd, "mkdir")==0) {
		int errnum=0;
		strcpy(cmd_buff, "MKD");
		while((token = strtok(NULL, " "))!=NULL) { //directory name
			strcat(cmd_buff, " ");
			strcat(cmd_buff, token);
			errnum++;
		}
		if(errnum==0) errArg(); //no directory name, error
	}
	//=====================delete -> DELE=====================//
	else if (strcmp(userCmd, "delete")==0) {
		int errnum=0;
		strcpy(cmd_buff, "DELE");
		while((token = strtok(NULL, " "))!=NULL) { //directory name
			strcat(cmd_buff, " ");
			strcat(cmd_buff, token);
			errnum++;
		}
		if(errnum==0) errArg(); //no file name, error
	}
	//======================rmdir -> RMD=====================//
	else if (strcmp(userCmd, "rmdir")==0) {
		int errnum=0;
		strcpy(cmd_buff, "RMD");
		while((token = strtok(NULL, " "))!=NULL) { //directory name
			strcat(cmd_buff, " ");
			strcat(cmd_buff, token);
			errnum++;
		}
		if(errnum==0) errArg(); //no directory name, error
	}
	//=================rename -> RNFR & RNTO==================//
	else if (strcmp(userCmd, "rename")==0) {
		token = strtok(NULL, " "); //old name
		if(token==NULL) errArg(); //no change name, error
		else {
			strcpy(cmd_buff, "RNFR ");
			strcat(cmd_buff, token);
			strcat(cmd_buff, "\nRNTO "); //Add newline
			token = strtok(NULL, " "); //new name
			strcat(cmd_buff, token);
		}
	}
	//======================quit -> QUIT=====================//
	else if (strcmp(userCmd, "quit")==0) {
		strcpy(cmd_buff, "QUIT");
	}
	//=====================Command Error=====================//
	else {
		printf("Error: invalid command\n");
		exit(1);
	}
	
	int n = strlen(cmd_buff);
	cmd_buff[n]='\0'; //set string end
}


int main(int argc, char **argv)
{
	char buff[BUF_SIZE], cmd_buff[BUF_SIZE], rcv_buff[1024];
	int n;
	int sockfd;
	struct sockaddr_in serv_addr;

	sockfd = socket(AF_INET, SOCK_STREAM, 0); //creat socket

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;			//set family
	serv_addr.sin_addr.s_addr = inet_addr(argv[1]);	//set ip address
	serv_addr.sin_port = htons(atoi(argv[2]));	//set port

	//==========================connect to server==========================//
	connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

	//====read user input and send to server, receive results and print====//
	write(STDOUT_FILENO, "> ", 2);
	while(1) {
		memset(buff, 0, sizeof(buff));
		memset(cmd_buff, 0, sizeof(cmd_buff));

		read(STDIN_FILENO, buff, sizeof(buff));
		
		char*newline = strchr(buff, '\n'); //clear newline characters
		if(newline != NULL) {
			*newline = '\0'; //set string termination
		}

		convertFTP(buff, cmd_buff); //user command -> FTP command

		n = strlen(cmd_buff);
		if(write(sockfd, cmd_buff, n) > 0) { //send string to server
			if((n=read(sockfd, rcv_buff, 1023)) <= 0) { //receive and print string again
				write(STDERR_FILENO, "Error: read() error!\n", 30);
				exit(1);
			}
			rcv_buff[n]='\0'; //set string end

			if(!strcmp(rcv_buff, "QUIT")) {
				write(STDOUT_FILENO, "Program quit!!\n", 16);
				exit(1);
			}

			//========================display result=======================//
			write(STDOUT_FILENO, rcv_buff, strlen(rcv_buff));
			write(STDOUT_FILENO, "\n", 1);
			write(STDOUT_FILENO, "> ", 2);
		}
		else	//quit if the string cannot be sent to the server
			break;
	}//end while
	close(sockfd);
	return 0;
}
