///////////////////////////////////////////////////////////////////////////////
// File Name	: cli.c
// Data		: 2024/05/19
// OS		: Ubuntu 20.04.6 LTS 64bits
// Author	: Park Na Rim
// Student ID	: 2022202065
// -------------------------------------------------------------------------//
// Title : System Programming Assignment #3-1 (ftp server)
// Description : This is a client program that converts user commands into
//               ftp commands and sends them to the server. Then it receives
//               the result and prints it out.
/////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <string.h>
#include <unistd.h> //write()
#include <stdlib.h> //exit()
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_BUF 20
#define CONT_PORT 20001


void log_in(int sockfd)
{
	int n;
	char user[MAX_BUF], *passwd, buf[MAX_BUF];
	//=============check if the ip is acceptable==============//
	n = read(sockfd, buf, MAX_BUF);
	buf[n] = '\0';
	if(!strcmp(buf, "REJECTION")) { //doesn't exist in the access.txt
		printf("**Connection refused**");
		close(sockfd);
		exit(1);
	}
	else { //buf is "ACCEPTED", receives username and password
		printf("**It is connected to Server");

		while(1) { 
			//user name
			memset(user, 0, sizeof(user));
			printf("Input ID: ");
			n = read(STDIN_FILENO, user, sizeof(user)); //get user name
			user[n] = '\0';
			write(sockfd, user, MAX_BUF); //pass user name to server
			
			//password
			passwd = getpass("Input Password: "); //get password
			write(sockfd, passwd, MAX_BUF); //pass password to server
			
			//read results checked by the server
			memset(buf, 0, sizeof(buf));
			n = read(sockfd, buf, MAX_BUF);
			buf[n] = '\0';
				
			if(!strcmp(buf, "OK")) { //Server finds username and password
				memset(buf, 0, sizeof(buf)); //clear buffer

				n = read(sockfd, buf, MAX_BUF); //Confirm login
				buf[n] = '\0';

				if(!strcmp(buf, "OK")) { 	//login success
					printf("**User '%s' logged in**", user);
					return 0;
				}
				else if(!strcmp(buf, "FAIL")) { //login fail
					printf("**Log-in failed**");
				}
				else {	//buf is "DISCONNECTION", three times fail
					printf("**Connection closed**");
					close(sockfd);
					exit(1);
				}
			}
		}//end while
	}//end ACCEPTED
}


int main(int argc, char *argv[])
{
	int sockfd, n, p_pid;
	struct sockaddr_in servaddr;
	char *haddr; //IP address
	haddr = argv[1];
	int PORTNO = atoi(argv[2]); //port

	//==============open socket and connect to server================//
	if((sockfd=socket(PF_INET, SOCK_STREAM, 0)) < 0) { //creat socket
		printf("Error: can't creat socket.\n");
		return -1;
	}

	//set server addr
	bzero((char*)&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;			//set family
	servaddr.sin_addr.s_addr = inet_addr(haddr);	//set ip address
	servaddr.sin_port = htons(PORTNO);		//set port
	
	//connect to server
	connect(sockfd, (struct SA *)&servaddr, sizeof(servaddr));

	log_in(sockfd);

	close(sockfd);
	return 0;
}
