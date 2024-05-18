////////////////////////////////////////////////////////////////////////
// File Name	: srv.c
// Date		: 2024/05/19
// OS		: Ubuntu 20.04.6 LTS 64bits
// Author	: Park Na Rim
// Student ID	: 2022202065
// ----------------------------------------------------------------- //
// Title: System Programming Assignment #3-1 (ftp server)
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
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pwd.h>

#define MAX_BUF 20


int user_match(char *user, char *passwd)
{
	FILE *fp;
	struct passwd *pw;

	fp = fopen("passwd", "r"); //Open file read-only
	
	//============Check login information==========//
	while((pw = fgetpwent(fp)) != NULL) { //Check the file by reading it line by line
		if(strcmp(user, pw->pw_name)==0 && strcmp(passwd, pw->pw_passwd)==0) {
			fclose(fp);
			return 1; //Authentication successful
		}
	}
	fclose(fp);
	return 0; //Authentication failed
}


int log_auth(int connfd)
{
	char user[MAX_BUF], passwd[MAX_BUF];
	int n, count=1;
	
	while(1) {
		memset(user, 0, MAX_BUF);
		if((n = read(connfd, user, MAX_BUF))<=0) {exit(1);}
		user[n]='\0';

		memset(passwd, 0, MAX_BUF);
		if((n = read(connfd, passwd, MAX_BUF))<=0) {exit(1);}
		passwd[n]='\0';

		write(connfd, "OK", MAX_BUF);

		if((n = user_match(user, passwd)) == 1) { //success
			write(connfd, "OK", MAX_BUF);
			return 1;
		}
		else if(n == 0) {
			if(count >= 3) { //3 times fail
				write(connfd, "DISCONNECTION", MAX_BUF);
				return 0;
			}
			write(connfd, "FAIL", MAX_BUF);
			count++;
			continue;
		}
	}
	return 1;
}


int main(int argc, char *argv[])
{
	int listenfd, connfd;
	struct sockaddr_in servaddr, cliaddr;
	FILE *fp_checkIP; 	//FILE stream to check client's IP

	listenfd = socket(PF_INET, SOCK_STREAM, 0);	//creat socket

	//set server addr
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;			//set family
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);	//set ip address
	servaddr.sin_port = htons(atoi(argv[1]));	//set port

	//bind: connect address to socket
	bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

	//listen to client requests
	listen(listenfd, 5);

	//============connect to client, read and execute==============//
	while(1) {
		int clilen = sizeof(cliaddr);
		connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen);

		//==========check client's IP===========//
		char cliIP[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &cliaddr.sin_addr, cliIP, sizeof(cliIP));
		fp_checkIP = fopen("access.txt", "r");
		if(fp_checkIP == NULL) {
			printf("Error: Failed to open 'access.txt' file");
			exit(1);
		}

		int allow = 0;
		char allowIP[INET_ADDRSTRLEN];
		while(fgets(allowIP, sizeof(allowIP), fp_checkIP) != NULL) {
			allowIP[strcspn(allowIP, "\n")] = 0;
			if(!strcmp(cliIP, allowIP)) {
				write(connfd, "ACCEPTED", MAX_BUF);
				allow++;
				break;
			}
		}
		fclose(fp_checkIP);
		if(allow == 0) {
			write(connfd, "REJECTION", MAX_BUF);
			printf("** It is NOT authenticated client: %s **\n", cliIP);
			close(connfd);
			continue;
		}

		//================Check login information==============//
		if(log_auth(connfd)==0) { //if 3 times fail (ok: 1, fail: 0)
			printf("** Fail to log-in **\n");
			close(connfd);
			continue;
		}
		printf("** Success to log-in **\n");
		close(connfd);
	}
	
	close(listenfd);
	return 0;
}
