/* Hangclient.c - Client for hangman server.  */
// Comment
 #include <stdio.h>
 #include <sys/types.h>
 #include <sys/socket.h>
 #include <netinet/in.h>
 #include <netdb.h>
 #include <string.h>
 #include <sys/select.h>
 #include <syslog.h>
 #include <signal.h>
 #include <errno.h>
 #include <stdlib.h>
 #include <string.h>
 #include <unistd.h>

 # define LINESIZE 80
 # define HANGMAN_TCP_PORT 1066

int main(int argc, char *argv[])
{
	const char *port = "1066";
	char *server;
	struct addrinfo hints,*host;
	int r,sockfd,count;
	char buffer[BUFSIZ];

 	char i_line[LINESIZE];
 	char o_line[LINESIZE];

	if( argc<2 )
	{
		fprintf(stderr,"Format: client hostname\n");
		exit(1);
	}
	server = argv[1];

	/* obtain and convert server name and port */
	printf("Looking for server on %s...",server);
	memset( &hints, 0, sizeof(hints) );		/* use memset_s() */
	hints.ai_family = AF_INET;				/* IPv4 */
	hints.ai_socktype = SOCK_STREAM;		/* TCP */
	r = getaddrinfo( server, port, &hints, &host );
	if( r!=0 )
	{
		perror("failed");
		exit(1);
	}
	puts("found");

	/* create a socket */
	sockfd = socket(host->ai_family,host->ai_socktype,host->ai_protocol);
	if( sockfd==-1 )
	{
		perror("failed");
		exit(1);
	}
	puts("Created Socket");

	/* connect to the socket */
	r = connect(sockfd,host->ai_addr,host->ai_addrlen);
	if( r==-1 )
	{
		perror("failed");
		exit(1);
	}

	puts("Connected to Socket");
	/* loop to interact with the server */
	while((count = read (sockfd, i_line, LINESIZE)) > 0)
	{
		write (1, i_line, count);
 		count = read (0, o_line, LINESIZE);//0 = STDIN
 		write (sockfd, o_line, count);
	}	/* end while loop */

	/* all done, clean-up */
	freeaddrinfo(host);
	close(sockfd);
	puts("Disconnected");

	return(0);
}