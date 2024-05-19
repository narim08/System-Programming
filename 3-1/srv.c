////////////////////////////////////////////////////////////////////////
// File Name	: srv.c
// Date		: 2024/05/19
// OS		: Ubuntu 20.04.6 LTS 64bits
// Author	: Park Na Rim
// Student ID	: 2022202065
// ----------------------------------------------------------------- //
// Title: System Programming Assignment #3-1 (ftp server)
// Description : This server program checks the client's IP and connects
// 		 to it. When a client attempts to log in, it is checked
// 		 and the results are delivered.
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


////////////////////////////////////////////////////////////////////
// user_match
// ============================================================== //
// Input : char *user	- user name
// 	   char *passwd	- password
// Output : 1 - success
// 	    0 - fail
// Purpose : Check whether the information used to log in is the
// 	     same as the saved information.
// /////////////////////////////////////////////////////////////////
int user_match(char *user, char *passwd)
{
	struct passwd *pw;
	FILE *fp = fopen("passwd", "r"); //Open file read-only
	if(fp == NULL) { 		//Error if file cannot be opened
		printf("Error: Failed to open passwd file\n");
		return 0;
	}

	//============Check login information==========//
	while((pw = fgetpwent(fp)) != NULL) { //Check the file by reading it line by line
		//Check user name and password
		if(strcmp(user, pw->pw_name)==0 && strcmp(passwd, pw->pw_passwd)==0) {
			fclose(fp);
			return 1; //Authentication successful
		}
	}
	fclose(fp);
	return 0; //Authentication failed
}


////////////////////////////////////////////////////////////////////
// log_auth
// ============================================================== //
// Input : int connfd	- Client's file descriptor
// Output : 1 - success
// 	    0 - fail
// Purpose : Check login information attempted by client.
// /////////////////////////////////////////////////////////////////
int log_auth(int connfd)
{
	char user[MAX_BUF], passwd[MAX_BUF];
	int n, count=1;
	
	//===============Repeat login up to 3 times===============//
	while(1) {
		printf("** User is trying to log-in (%d/3) **\n", count);

		//username received
		memset(user, 0, MAX_BUF);
		if((n = read(connfd, user, sizeof(user)))<=0) {exit(1);}
		user[n]='\0';

		//password received
		memset(passwd, 0, MAX_BUF);
		if((n = read(connfd, passwd, MAX_BUF))<=0) {exit(1);}
		passwd[n]='\0';

		//Information received notification
		write(connfd, "OK", MAX_BUF);

		//===Check whether the saved name and password match===//
		if((n = user_match(user, passwd)) == 1) { //success
			write(connfd, "OK", MAX_BUF);
			return 1;
		}
		else if(n == 0) { //fail
			if(count >= 3) { //3 times fail
				write(connfd, "DISCONNECTION", MAX_BUF);
				return 0;
			}
			printf("** Log-in failed **\n");
			write(connfd, "FAIL", MAX_BUF);
			count++;
			continue;
		}
	}
	return 1;
}


////////////////////////////////////////////////////////////////////
// IP_match
// ============================================================== //
// Input : char *allowIP - IP written in 'access.txt' file
// 	   char *cliIP	- client's IP
// Output : 1 - success
// 	    0 - fail
// Purpose : Check whether the client's IP is accessible.
// /////////////////////////////////////////////////////////////////
int IP_match(char *allowIP, char *cliIP)
{
	if(!strcmp(allowIP, "*.*.*.*")) { //Allowed if all wildcards
		return 1;
	}

	//======Separate IP into 4 parts based on ‘.’======//
	char *allowPart[4], *cliPart[4];

	//copy original
	char *allowCP = strdup(allowIP);
	char *cliCP = strdup(cliIP);

	allowPart[0] = strtok(allowCP, ".");
	cliPart[0] = strtok(cliCP, ".");

	for(int i=1; i<4; i++) {	//Separately store them in each array
		allowPart[i] = strtok(NULL, ".");
		cliPart[i] = strtok(NULL, ".");
	}

	for(int i=0; i<4; i++) {	//Match check for each part
		if(allowPart[i] && cliPart[i]) { //While both IPs are valid
			//If it is not a wildcard and the two are different, fail.
			if(strcmp(allowPart[i], "*")!=0 && strcmp(allowPart[i], cliPart[i])!=0) {
				free(allowCP); free(cliCP); return 0;
			}
		}
		else { //Fail if invalid
			free(allowCP); free(cliCP); return 0;
		}
	}

	//Success after error checking
	free(allowCP);
	free(cliCP);
	return 1;
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
	printf(" - IP: %s\n", ipAddr);
	printf(" - port: %d\n", ntohs(client_addr->sin_port));
	return 0;
}


////////////////////////////////////////////////////////////////////
// main
// ============================================================== //
// Input : int argc	- number of arguments
// 	   char *argv[]	- content of argument
// Output : 0 - Normal completion
// Purpose : Print out the login process after connecting to the
// 	     client, check client's IP address.
// /////////////////////////////////////////////////////////////////
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

		printf("** Client is trying to connect **\n");
		if(client_info(&cliaddr) < 0) { //display client ip and port
			printf("Error: client_info error!!\n");
		}

		//==================check client's IP==================//
		char cliIP[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &cliaddr.sin_addr, cliIP, sizeof(cliIP));
		fp_checkIP = fopen("access.txt", "r"); //accessible IP file
		if(fp_checkIP == NULL) { //Error if file cannot be opened
			printf("Error: Failed to open 'access.txt' file\n");
			exit(1);
		}

		int allow = 0; //1 if client IP is allowed
		char allowIP[INET_ADDRSTRLEN]; //accessible IP

		//Check the file by reading it line by line
		while(fgets(allowIP, sizeof(allowIP), fp_checkIP) != NULL) {
			allowIP[strcspn(allowIP, "\n")] = 0; //Extract one line (excluding newlines)

			if(IP_match(allowIP, cliIP)==1) { //Connect if the IP is accessible
				allow++;
				printf("** Client is connected **\n");
				write(connfd, "ACCEPTED", MAX_BUF);
				break;
			}

		}
		fclose(fp_checkIP);
		if(allow == 0) { //Inaccessible IP, connection terminated
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
