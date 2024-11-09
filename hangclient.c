/* Hangclient.c - Client for hangman server.  */
 #include <sys/types.h>
 #include <sys/socket.h>
 #include <netinet/in.h>
 #include <stdio.h>
 #include <syslog.h>
 #include <signal.h>
 #include <errno.h>
 #include <stdlib.h>
 #include <string.h>
 #include <unistd.h>
 #include <netdb.h>
 #include <ctype.h> 

 # define LINESIZE 80
 # define HANGMAN_TCP_PORT 1066

/* Function to check if input is a single letter */
int is_valid_input(char *input) {
    // Check if the input is exactly one alphabetic character
    return (strlen(input) == 1 && isalpha(input[0]));
}

/* Function to get a valid single letter from the user */
void get_single_letter_input(char *input) {
    while (1) {
        printf("Please enter only one letter: ");
        fgets(input, LINESIZE, stdin); // Read user input

        // Remove newline character from input (if any)
        input[strcspn(input, "\n")] = 0;

        // Check if input is valid
        if (is_valid_input(input)) {
            break; // Valid input, break the loop
        } else {
            // Invalid input, prompt again
            printf("Invalid input! Please enter a single letter.\n");
        }
    }
}

 int main (int argc, char * argv [])
 {	
 	struct sockaddr_in server; /* Server's address assembled here */
 	struct hostent * host_info;
 	int sock, count;
 	char i_line[LINESIZE];
 	char o_line[LINESIZE];
 	char * server_name;

 	/* Get server name from the command line.  If none, use 'localhost' */

 	server_name = (argc == 1)?  argv [1]: "localhost";

 	/* Create the socket */
 	sock = socket (AF_INET, SOCK_STREAM, 0);
 	if (sock <0) {
 		perror ("Creating stream socket");
 		exit (1);
 	}

 	host_info = gethostbyname(server_name);
 	if (host_info == NULL) {
 		fprintf (stderr, "%s: unknown host:%s \n", argv[0], server_name);
 		exit (2);
 	}

 	/* Set up the server's socket address, then connect */

 	server.sin_family = host_info->h_addrtype;
 	memcpy ((char *) & server.sin_addr, host_info->h_addr, host_info->h_length);
 	server.sin_port = htons (HANGMAN_TCP_PORT);

 	if (connect (sock, (struct sockaddr *) & server, sizeof server) <0) {
 		perror ("connecting to server");
 		exit (3);
 	}
 	/*OK connected to the server.  
 Take a line from the server and show it, take a line and send the user input to the server. 
 Repeat until the server terminates the connection. */

 	printf ("Connected to server %s \n", server_name);
 	while ((count = read (sock, i_line, LINESIZE)) > 0) {
		
 		write(1, i_line, count); // Display server message
        
        // Get valid input from the user
        get_single_letter_input(o_line); // Get a single letter from the user

        // Send the input to the server
        if (write(sock, o_line, strlen(o_line)) < 0) {
            perror("Sending to Server");
            break;
        }
 	}
	if(count < 0)
	{
		perror("Reading from Socket");

	}
	close(sock);
	return 0;
 }
