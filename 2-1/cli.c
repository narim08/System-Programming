#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_BUFF 1024
#define RCV_BUFF 1024


int conv_cmd(char* buff, char*cmd_buff) 
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
		cmd_buff[n] = '\0';
	}
	else { //invalid command
		write(STDERR_FILENO, "Error: invalid command\n", 50);
		return -1;
	}
	return 1;
}



void process_result(char* rcv_buff)
{
	write(STDOUT_FILENO, rcv_buff, sizeof(rcv_buff));
	write(STDOUT_FILENO, "\n", 1);
}


int main(int argc, char **argv)
{
	char buff[MAX_BUFF], cmd_buff[MAX_BUFF], rcv_buff[RCV_BUFF];
	int n;

	//==============open socket and connect to server================//
	int sockfd, len;
	struct sockaddr_in server_addr;
	char *haddr; //IP addr
	haddr = argv[1];

	int PORTNO = atoi(argv[2]);

	if((sockfd=socket(PF_INET, SOCK_STREAM, 0)) < 0) { //creat socket
		printf("Error: can't creat socket.\n");
		return -1;
	}

	//set server addr
	bzero(buff, sizeof(buff));
	bzero((char*)&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(haddr);
	server_addr.sin_port = htons(PORTNO);

	//connect
	if(connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr))<0) {
		printf("Error: can't connect.\n");
		return -1;
	}

	//read user command
	write(STDOUT_FILENO, "> ", 2);
	while((len = read(STDIN_FILENO, buff, sizeof(buff))) > 0) {
		char*newline = strchr(buff, '\n');
		if(newline != NULL) {
			*newline = '\0';
		}

		if(conv_cmd(buff, cmd_buff) < 0) { //convert ls to NLST(buff->cmd_buff)
			write(STDERR_FILENO, "Error: conv_cmd() error!!\n", 50);
			exit(1);
		}

		n = strlen(cmd_buff);
		if(write(sockfd, cmd_buff, n) != n) { //ftp cmd send to server
			write(STDERR_FILENO, "Error: write() error!!\n", 50);
			exit(1);
		}
		
		if((n=read(sockfd, rcv_buff, RCV_BUFF-1)) < 0) { //read server result
			write(STDERR_FILENO, "Error: read() error!!\n", 50);				exit(1);
		}
		rcv_buff[n]='\0';

		if(!strcmp(rcv_buff, "QUIT")) { //program quit
			write(STDOUT_FILENO, "Program quit!!\n", 16);
			exit(1);
		}

		write(STDOUT_FILENO, rcv_buff, strlen(rcv_buff));
		write(STDOUT_FILENO, "\n", 1);
		write(STDOUT_FILENO, "> ", 2);

	} //end while	
	return 0;
}
