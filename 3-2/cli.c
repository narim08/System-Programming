////////////////////////////////////////////////////////////////////////////////
// File Name	: cli.c
// Data		: 2024/05/26
// OS		: Ubuntu 20.04.6 LTS 64bits
// Author	: Park Na Rim
// Student ID	: 2022202065
// ---------------------------------------------------------------------------//
// Title : System Programming Assignment #3-2 (ftp server)
// Description : The client program creates a random port number and establishes
// 		 a separate connection with the server.
////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <string.h>
#include <unistd.h> //write()
#include <stdlib.h> //exit()
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#define MAX_BUF 200
#define RCV_BUF 1024


////////////////////////////////////////////////////////////////////////////
//convert_addr_to_str
//========================================================================//
//Input: unsigned long ip_addr	- ip address
//	 unsigned int port	- port number
//Output: addr			- port command
//Purpose: Create a port command with ip and new port.
////////////////////////////////////////////////////////////////////////////
char* convert_addr_to_str(unsigned long ip_addr, unsigned int port)
{
	//Save IP separately
	static char addr[64];
	unsigned char byte[4];
	byte[0] = ip_addr & 0xFF;
	byte[1] = (ip_addr >> 8) & 0xFF;
	byte[2] = (ip_addr >> 16) & 0xFF;
	byte[3] = (ip_addr >> 24) & 0xFF;

	//creat port command
	snprintf(addr, sizeof(addr), "PORT %d,%d,%d,%d,%d,%d", byte[0], byte[1], byte[2], byte[3], port/256, port%256);

	return addr;
}

////////////////////////////////////////////////////////////////////////////
//conv_cmd
//========================================================================//
//Input: char* buff	-user command line buffer
//	 char* cmd_buff -convert, ftp command line buffer
//Output: 1		-Normal completion
//	  -1		-invalid command
//Purpose: Convert user command to FTP command
////////////////////////////////////////////////////////////////////////////
int conv_cmd(char *buff, char *cmd_buff)
{
	char *token = strtok(buff, " "); //user command
	char *userCmd = token;

	if(strcmp(userCmd, "quit")==0) { //quit
		strcpy(cmd_buff, "QUIT"); //FTP command
		cmd_buff[4]='\0';
	}
	else if(strcmp(userCmd, "ls")==0) { //ls
		strcpy(cmd_buff, "NLST"); //FTP command
		while((token = strtok(NULL, " "))!=NULL) { //option. argument parse
			strcat(cmd_buff, " ");
			strcat(cmd_buff, token);
		}
		int n = strlen(cmd_buff);
		cmd_buff[n] = '\0'; //set string end
	}
	else { //invalid command, error
		write(STDERR_FILENO, "Error: invalid command\n", 50);
		return -1;
	}
	return 1;
}

////////////////////////////////////////////////////////////////////
// main
// ============================================================== //
// Input : int argc	- number of arguments
// 	   char *argv[]	- content of argument
// Output : 0 - Normal completion
// 	   -1 - Error
// Purpose : connect to server with socket. Communicate with the
// 	     server over a separate connection
// /////////////////////////////////////////////////////////////////
int main(int argc, char *argv[])
{
	int sockfd, data_sockfd, len, n;
	char buff[MAX_BUF], cmd_buff[MAX_BUF], rcv_buff[MAX_BUF];
	struct sockaddr_in serv_addr, data_addr, cli_addr;
	socklen_t cli_len = sizeof(cli_addr);

	char *haddr; //IP address
	haddr = argv[1];
	int PORTNO = atoi(argv[2]);

	//==============open socket and connect to server================//
	if((sockfd=socket(PF_INET, SOCK_STREAM, 0)) < 0) { //creat socket
		printf("Error: can't creat socket.\n");
		return -1;
	}

	//set server addr
	bzero((char*)&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;			//set family
	serv_addr.sin_addr.s_addr = inet_addr(haddr);	//set ip address
	serv_addr.sin_port = htons(PORTNO);			//set port

	if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        	perror("Error: can't connect to server");
        	close(sockfd);
        	return -1;
    	}

	//==============input ls command, convert NLST==============//
	write(STDOUT_FILENO, "> ", 2);
	memset(buff, 0, sizeof(buff));
	memset(cmd_buff, 0, sizeof(cmd_buff));
	if((len=read(STDIN_FILENO, buff, sizeof(buff)))>0) {//read user command line
		char*newline = strchr(buff, '\n'); //clear newline characters
		if(newline != NULL) {
			*newline = '\0'; //set string termination
		}
		if(conv_cmd(buff, cmd_buff) < 0) { //convert ls to NLST(buff->cmd_buff)
			printf("Error: invalid command\n");
			return -1;
		}
		if(!strcmp(cmd_buff, "QUIT")) {
			printf("221 Goodbye\n");
			exit(0);
		}
	}

	//====================send Port command=====================//
	int dataPort = 12345; //new port
	char*hostport = convert_addr_to_str(serv_addr.sin_addr.s_addr, dataPort);
	
	write(sockfd, hostport, strlen(hostport)); //send port cmd to server
	
	memset(buff, 0, sizeof(buff));
	read(sockfd, buff, sizeof(buff));
	write(1, buff, strlen(buff));
	write(1, "\n", 1);
	if(strncmp(buff, "200", 3)!=0) { //check ACK
		write(STDERR_FILENO, "Error: not ACK\n", 16);
		return -1;
	}
	
	//========================data connection=======================//
	if((data_sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		printf("Error: can't creat socket\n");
		close(sockfd);
		return -1;
	}

	//set new address
	memset(&data_addr, 0, sizeof(data_addr));
	data_addr.sin_family = AF_INET;
	data_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    	data_addr.sin_port = htons(dataPort);

	if (bind(data_sockfd, (struct sockaddr *) &data_addr, sizeof(data_addr)) < 0) {
        	perror("Error: can't bind data socket");
        	close(data_sockfd);
        	close(sockfd);
        	return -1;
    	}

    	if(listen(data_sockfd, 1)<0) {
		perror("Error: can't listen");
		close(data_sockfd); close(sockfd); return -1;
	}

	int cli_sockfd;
	if ((cli_sockfd = accept(data_sockfd, (struct sockaddr *)&cli_addr, &cli_len)) < 0) {
         	perror("Error: can't accept data connection");
            	close(data_sockfd); close(sockfd); return -1;
       	}
	
	write(cli_sockfd, cmd_buff, strlen(cmd_buff)); //send command to server
	memset(rcv_buff, 0, sizeof(rcv_buff));
	memset(buff, 0, sizeof(buff));
	n = read(cli_sockfd, buff, MAX_BUF-1); //read 150
	buff[n]='\0';
	write(1, buff, strlen(buff));
	
	if((n=read(cli_sockfd, rcv_buff, RCV_BUF-1)) < 0) { //read server result
		write(STDERR_FILENO, "Error: read() error!!\n", 50);
		exit(1);
	}
	rcv_buff[n]='\0'; //set string termination

	//========================display result=======================//
	write(STDOUT_FILENO, rcv_buff, strlen(rcv_buff));
	write(STDOUT_FILENO, "\n", 1);
	
	memset(buff, 0, sizeof(buff));
	n = read(cli_sockfd, buff, sizeof(buff)); //226
	buff[n]='\0';
	write(1, buff, strlen(buff));	
	
	close(cli_sockfd);	
	close(data_sockfd);
	close(sockfd);

	return 0;
}
