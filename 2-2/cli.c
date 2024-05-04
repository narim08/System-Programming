///////////////////////////////////////////////////////////////////////////////
// File Name	: cli.c
// Data		: 2024/05/04
// OS		: Ubuntu 20.04.6 LTS 64bits
// Author	: Park Na Rim
// Student ID	: 2022202065
// -------------------------------------------------------------------------//
// Title : System Programming Assignment #2-2 (ftp server)
// Description : This is a client program that sends the input string to
// 		 the server and then receives it back and prints it.
//////////////////////////////////////////////////////////////////////////////

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

int main(int argc, char **argv) 
{
	char buff[BUF_SIZE];
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
	while(1) {
		write(STDOUT_FILENO, "> ", 2);
		read(STDIN_FILENO, buff, BUF_SIZE); //read user input

		if(write(sockfd, buff, strlen(buff)) > 0) { //send string to server
			if(read(sockfd, buff, sizeof(buff)) > 0) { //receive and print string again
				printf("from server:%s", buff);
			}
			else	//quit if string is not received
				break;
		}
		else	//quit if the string cannot be sent to the server
			break;
	}//end while
	close(sockfd);
	return 0;
}
