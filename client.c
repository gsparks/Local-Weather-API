#include <sys/socket.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>

#define MAXBUFFER	512

/* To do */
// XML as input to command line ---> ✓
// Server location as option, default to localhost ---> ✓
// XML validation 
// web wrapper with icons and pretty stuff ---> ✓
// Actually format XML response ---> ✓
// recognize Airport code vs lat/lon vs City code (google api? lat/lon)

/* Test Request */
// static char Request[] = "<wwxtp><query><command>TEST</command><id>IZA</id></query></wwxtp>";

/* Instructions */
// Compile : gcc client.c -lgc -o client (must have lgc installed)
// Run : ./client <xml_request> <server>
// The first argument to the program is the formatted xml request and the second in the server represented as a dotted quad.
// If no server is specified the program will default to localhost.

char Server[30] = { 0 };              

int main(int argc, char *argv[])
{
	int fd;
	int ret;
	struct sockaddr_in sa;
	char buffer[MAXBUFFER];
	
	/* Process input */
	char Request[500] = { 0 };                            // Reserve Request (Greg)
	strncpy(Request, argv[1], sizeof(Request) - 1);       // Convert pointer => string and place in Request (Greg)
	if(argc < 3)  
		strncpy(Server, "127.0.0.1", sizeof(Server) - 1); // If no server option specified, use localhost (Greg)
	else if(argc == 3)
		strncpy(Server, argv[2], sizeof(Server) - 1);     // Otherwise use specified server (Greg)
	else
		printf("Warning: Too many arguments given.  Expecting 2, given %d.\n", argc); // Display warning if wrong number of arguments given (Greg)

	/* Create socket */
	printf("creating socket...\n");	
	fd = socket(AF_INET, SOCK_STREAM, 0);
	
	if(fd == -1) {
		printf("socket() failed with errno = %d", errno);
		return errno;
	}
	
	/* Setup sockaddr */
	inet_pton(AF_INET, Server, &(sa.sin_addr));
	sa.sin_family = AF_INET;
	sa.sin_port = htons(4114); /* host to network endian swap */

	/* Connect */
	ret = connect(fd, (struct sockaddr *) &sa, sizeof(sa));

	if(ret == -1) {
		printf("connect() failed with errno = %d", errno);
		return errno;
	}

	/* Send request */
	printf("sending request...\n");
	ret = send(fd, Request, strlen(Request), 0);
	
	if(ret == -1) {
		printf("send() failed with errno = %d", errno);
		return errno;
	}
	
	/* Recieve response */
	do
	{
		ret = recv(fd, &buffer, MAXBUFFER - 1, 0);

		if(ret > 0)
		{
			buffer[ret] = '\0'; /* null terminate the string */
			printf("%s",buffer);
		}
	} while(ret > 0);

	if(ret == -1) {
		printf("recv() failed with errno = %d", errno);
		return errno;
	}

	/* Close */
	ret = close(fd);
	if(ret == -1) {
		printf("close() failed with errno = %d", errno);
		return errno;
	}

}	