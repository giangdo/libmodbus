/*
 * Author: Giang Do
 *
 * This program is used to talk with modbus slave simulation program through UDP at port 61234
 * It can request read/set modbus registers of specific slave ID or all slave IDs (from 1 to 254)
 *
 * Usage:
 *    + To read register value of slave 28: (return register will be in hexadecimal format)
 *       ./ctrlSlave 28 r
 *    + To read register value of slave 230: (return register will be in hexadecimal format)
 *       ./ctrlSlave 230 r
 *    + To read register value of all slave from 1 to 250: (return register will be in hexadecimal format)
 *       ./ctrlSlave a r
 *
 *    + To set register value of slave 28: (value of register need to be in hexadecimal format)
 *       ./ctrlSlave 28 s ab0 ffd0 ae09 1 1 1 1 1 9 a a b c d e f
 *    + To set register value of slave 230: (value of register need to be in hexadecimal format)
 *       ./ctrlSlave 230 s ff ffd0 ae09 1 1 1 1 1 9 a a b c d e f
 *    + To set register value of all slave from 1 to 250: (value of register need to be in hexadecimal format)
 *       ./ctrlSlave a s ab0 ffd0 ae09 1 1 1 1 1 9 a a b c d e f
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define BUFSIZE 1024

void error(char *msg);
/*
 *  * error - wrapper for perror
 *   */
void error(char *msg) {
	perror(msg);
	exit(0);
}

int main(int argc, char **argv) {
	int sockfd, portno, n;
	int serverlen;
	struct sockaddr_in serveraddr;
	struct hostent *server;
	char *hostname;
	char buf[BUFSIZE];

#if 0
	/* check command line arguments */
	if (argc != 3) {
		fprintf(stderr,"usage: %s <hostname> <port>\n", argv[0]);
		exit(0);
	}

	hostname = argv[1];
	portno = atoi(argv[2]);
#else
	if (argc < 3)
	{
		fprintf(stderr,"usage: %s <slaveID> <r/s> <modbus value> \n", argv[0]);
		fprintf(stderr,"<slaveID> is decimal format \n");
		fprintf(stderr,"modbus value is in hexadecimal format, if you want to set modbus value, you need to specify all 16 register\n");
		exit(0);
	}

	hostname = "127.0.0.1";
	portno = 61234;
#endif

	/* socket: create the socket */
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0)
		error("ERROR opening socket");

	/* gethostbyname: get the server's DNS entry */
	server = gethostbyname(hostname);
	if (server == NULL) {
		fprintf(stderr,"ERROR, no such host as %s\n", hostname);
		exit(0);
	}

	/* build the server's Internet address */
	bzero((char *) &serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	bcopy((char *)server->h_addr,
	      (char *)&serveraddr.sin_addr.s_addr, server->h_length);
	serveraddr.sin_port = htons(portno);

	/* get a message from the user */
	bzero(buf, BUFSIZE);
#if 0
	printf("Please enter msg: ");
	if (fgets(buf, BUFSIZE, stdin) == NULL)
	{
		error("There are no input");
	}
#else
	for (unsigned char i = 1; i < argc; ++i)
	{
		strcat(buf, argv[i]);
		strcat(buf, " ");
	}

	if (strcmp(argv[1], "a") == 0)
	{
		printf("SlaveID = all\n");
	}
	else
	{
		printf("SlaveID = %d\n", atoi(argv[1]));
		if (atoi(argv[2]) > 250)
		{
			fprintf(stderr, "!!!Wrong slave ID\n");
			exit(EXIT_FAILURE);
		}
	}

	if (strcmp(argv[2], "r") == 0)
	{
		if (argc != 3)
		{
			fprintf(stderr, "!!!Wrong command, if you want to read all slave: ./ctrlSlave a r \n"\
			        "If you want to read register of modbus slave id = 23 : ./ctrlSlave 23 r\n");
			exit(EXIT_FAILURE);
		}
	}
	else if (strcmp(argv[2], "s") == 0)
	{
		if (argc != 19)
		{
			fprintf(stderr, "!!!Wrong command\n");
			fprintf(stderr, "!!!Wrong command, if you want to set all slave: ./ctrlSlave a s 0 1 2 3 4 5 6 7 8 9 a b c d e f \n"\
			        "If you want to set register of modbus slave id = 23: ./ctrlSlave a s 0 1 2 3 4 5 6 7 8 9 a b c d e f \n");
			exit(EXIT_FAILURE);
		}

		printf("Set value     : ");
		for (unsigned char i = 3; i < argc; ++i)
		{
			printf("%s ", argv[i]);
		}
		printf("\n");
	}
	else
	{
		fprintf(stderr,"usage: %s <slaveID> <r/s> <modbus value> \n", argv[0]);
		fprintf(stderr,"<slaveID> is decimal format \n");
		fprintf(stderr,"modbus value is in hexadecimal format, if you want to set modbus value, you need to specify all 16 register\n");
		exit(EXIT_FAILURE);
	}
#endif

	/* send the message to the server */
	serverlen = sizeof(serveraddr);
	n = sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr *)&serveraddr, serverlen);
	if (n < 0)
		error("ERROR in sendto");

	/* print the server's reply */
	n = recvfrom(sockfd, buf, BUFSIZE, 0, (struct sockaddr *)&serveraddr, (socklen_t*)&serverlen);
	if (n < 0)
		error("ERROR in recvfrom");
	printf("%s", buf);
	return 0;
}
