////////////////////////////////////////////////////////////////////////
// File Name	: srv.c
// Date		: 2024/05/04
// OS		: Ubuntu 20.04.6 LTS 64bits
// Author	: Park Na Rim
// Student ID	: 2022202065
// ----------------------------------------------------------------- //
// Title: System Programming Assignment #2-2 (ftp server)
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
#include <sys/wait.h>
#include <signal.h>

#define BUF_SIZE 256

void sh_chld(int); //signal handler for SIGCHLD
void sh_alrm(int); //signal handler for SIGALRM


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


int main(int argc, char **argv) 
{
	char buff[BUF_SIZE];
	int n;
	struct sockaddr_in server_addr, client_addr;
	int server_fd, client_fd;
	int len;
	int port;

	signal(SIGCHLD, sh_chld);	//applying signal handler(sh_alrm) for SIGALRM
	signal(SIGALRM, sh_alrm);	//applying signal handler(sh_chld) for SIGCHLD
	
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
			printf("Child Process ID: %d\n", getpid());
			while((n=read(client_fd, buff, BUF_SIZE))>0) { //read client's string
				char*newline = strchr(buff, '\n'); //clear newline characters
				if(newline != NULL) {
					*newline = '\0'; //set string termination
				}
				
				if(!strcmp(buff, "QUIT")) { //program quit
					close(client_fd);
					close(server_fd);
					sh_alrm(1); //1 second
				}
				write(client_fd, buff, strlen(buff)); //send to client
			}
		}	
		else { 			//parent process	
			if(client_info(&client_addr) < 0) { //display client ip and port
				printf("Error: client_info error!!\n");
			}
			//connect to next client

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
	wait(NULL); //Parent process waits for child process to terminate
}


////////////////////////////////////////////////////////////////////
// sh_alrm
// ============================================================== //
// Input : int signum - signal number
// Output : x
// Purpose : Notifies process timeout
// /////////////////////////////////////////////////////////////////
void sh_alrm(int signum) {
	printf("Child process(PID: %d) will be terminated.\n", getpid());
	exit(1);
}
