 /* Network server for hangman game */
 /* File: hangserver.c */

 #include <sys/types.h>
 #include <sys/socket.h>
 #include <sys/select.h>
 #include <netinet/in.h>
 #include <stdio.h>
 #include <syslog.h>
 #include <signal.h>
 #include <errno.h>
 #include <stdlib.h>
 #include <string.h>
 #include <unistd.h>
 #include <netdb.h>

 extern time_t time ();

 int maxlives = 12;
 char *word [] = {
 # include "words"
 };
 # define NUM_OF_WORDS (sizeof (word) / sizeof (word [0]))
 # define MAXLEN 80 /* Maximum size in the world of Any string */
 # define HANGMAN_TCP_PORT 1066

 typedef struct sockaddr SA;
 typedef struct sockaddr_in SA_IN;

 void play_hangman(int in, int out);
 void draw_hangman(int in, int out);
int main()
 {
	int sock, fd, client_socket, option, r;
	struct addrinfo hints, *server;
	struct sockaddr client_address;
	socklen_t client_len;
	const int buffer_size = 1024;
	char buffer[buffer_size];
	const char *http_data = 
			"HTTP/1.1 200 OK\r\n"
			"Connection: close\r\n"
			"Content-Type: text/html\r\n\r\n"
			"<h1>Welcome to Hangman Game Server!</h1>";
	const int backlog = 10;
	char hostname_size = 32;
	char hostname[hostname_size];
	char connection[backlog][hostname_size];
	int num = 0;

 	srand ((int) time ((long *) 0)); /* randomize the seed */
	//Setting up fd sets
	fd_set current_sockets, ready_sockets;
	int max_connections;

	//Set up the server hints for dual stack
	printf("Configuring server...\n");
 	
	memset( &hints, 0, sizeof(struct addrinfo) );
	hints.ai_family = AF_INET6;			/* IPv6 connection */
	hints.ai_socktype = SOCK_STREAM;	/* TCP, streaming */
	hints.ai_flags = AI_PASSIVE;  //accept any connections
	//Get address info for both IPv4 and IPv6
	r = getaddrinfo( 0, "1066", &hints, &server );
	if(r != 0)
	{
		perror("Failed to configure server");
		exit(1);
	}
	puts("done");
	//create the socket to communications
	printf("Assigning a socket...");
	sock = socket(server->ai_family,		/* domain, TCP/UDP */
			server->ai_socktype,	/* type, stream or datagram */
			server->ai_protocol		/* protocol */);
	if(sock == -1)//Check if failed to create socket 
	{
		perror("Failed to create socket");
		freeaddrinfo(server);
		exit(1);
	}
	puts("done");
	//Bind the socket
	printf("Binding socket...\n");
	r = bind(sock, server->ai_addr, server->ai_addrlen);
	if(r == -1)//check if failed to bind
	{
		perror("Binding failed");
		exit(1);
	}
	//Free the address information
	freeaddrinfo(server);
	//Listen for connection
	printf("Listening on port %s...\n", "1066");
	r = listen(sock, 5);
	if(r == -1)//check if failed to listning
	{
		perror("Listening failed");
		exit(1);
	}

	fd = accept (sock, (struct sockaddr *) &client_address, &client_len);// client accepting

	// Client is now connect
	r = recv(fd, buffer, buffer_size, 0);
	if(r > 0)
	{
		printf("Received %d bytes: ", r);
		for(int x = 0; x < r; x++)
		{
			putchar(buffer[x]);
		}
	}
	//Send message
	r = send(fd, http_data, strlen(http_data), 0);
	if(r < 1)//check if the send function failed
	{
		perror("Send failed");
		exit(1);
	}
	printf("Send %d bytes\n", r);//print the bytes
	close(fd);//close client 


	//Initliaziling fd set
	max_connections = backlog;
	FD_ZERO(&current_sockets);
	FD_SET(sock, &current_sockets);
	
	//Accept connections and handle each one of the new fork process
 	while (1) {
    	ready_sockets = current_sockets;

    	r = select(max_connections + 1, &ready_sockets, NULL, NULL, 0);
	    if (r == -1) {
        	perror("Select Error");
    	    //exit(1);
	    } 

 	    for (int i = 0; i <= max_connections; i++) {
 	        if (FD_ISSET(i, &ready_sockets)) {
	            if (i == sock) {
            	    // Accept a new connection
        	        client_len = sizeof(client_address);
    	            client_socket = accept(sock, (struct sockaddr*)&client_address, &client_len);
	                if (client_socket < 0) {
                    	perror("Failed to accept connection");
                	    continue;
            	    }
					//connection acception, get name
					r = getnameinfo(&client_address, client_len, hostname, hostname_size, 0, 0, NI_NUMERICHOST);
					//update array
					strcpy(connection[client_socket], hostname);
					printf("New connection from %s\n", connection[client_socket]);
					//add new client socket to the master list
        	        FD_SET(client_socket, &current_sockets);
					num++;
					sprintf(buffer, "Hello player %d Please enter a letter!\n", num);
 					write(client_socket, buffer, strlen (buffer));
					// Handle existing connection with fork
					printf("Player %d join the server\n",num);
            	    int pid = fork();
        	        if (pid == 0) {
    	                // Child process: Handle client
	                    close(sock);  // Child doesn't need the listening socket
                    	play_hangman(client_socket, client_socket);  // Play hangman with the client
                	    close(client_socket);  // Close client socket after the game ends
						FD_CLR(client_socket, &current_sockets);
            	        exit(0);  // Exit the child process
        	        } else if (pid > 0) {
    	                // Parent process: Close client socket here to avoid duplication and avoid zombie
	                    close(client_socket);  // Parent doesn't need the client socket directly
						FD_CLR(client_socket, &current_sockets);
						waitpid(pid, NULL, WNOHANG);// wait for the for the child process to terminate
					} else {
            	        // Fork failed
        	            perror("Fork failed");
    	                exit(1);
	                }
            	}
				
       		}
    	}
	}

	
	//Clean up
	close(sock);
	return 0;
 }
 /* ---------------- Play_hangman () ---------------------*/

 void play_hangman (int in, int out)
 {
 	char * whole_word, part_word [MAXLEN],
 	guess[MAXLEN], outbuf [MAXLEN];

 	int lives = maxlives;
 	int game_state = 'I';//I = Incomplete
 	int i, good_guess, word_length;
 	char hostname[MAXLEN];

 	gethostname (hostname, MAXLEN);
 	sprintf(outbuf, "Playing hangman on host% s: \n \n", hostname);
 	write(out, outbuf, strlen (outbuf));

 	/* Pick a word at random from the list */
 	whole_word = word[rand() % NUM_OF_WORDS];
 	word_length = strlen(whole_word);
 	syslog (LOG_USER | LOG_INFO, "server chose hangman word %s", whole_word);

 	/* No letters are guessed Initially */
 	for (i = 0; i <word_length; i++)
 		part_word[i]='-';
 	
	part_word[i] = '\0';

 	sprintf (outbuf, "%s %d \n", part_word, lives);
 	write (out, outbuf, strlen(outbuf));

 	while (game_state == 'I')
 	/* Get a letter from player guess */
 	{
		while (read (in, guess, MAXLEN) <0) {
 			if (errno != EINTR)
 				exit (4);
 			printf ("re-read the startin \n");
 			} /* Re-start read () if interrupted by signal */
 	good_guess = 0;
 	for (i = 0; i <word_length; i++) {
 		if (guess [0] == whole_word [i]) {
 		good_guess = 1;
 		part_word [i] = whole_word [i];
 		}
 	}
 	if (! good_guess) lives--;
	draw_hangman(lives, out);//draw hangman
 	if (strcmp (whole_word, part_word) == 0)
 		game_state = 'W'; /* W ==> User Won */
 	else if (lives == 0) {
 		game_state = 'L'; /* L ==> User Lost */
 		strcpy (part_word, whole_word); /* User Show the word */
 	}
 	sprintf (outbuf, "%s %d \n", part_word, lives);
 	write (out, outbuf, strlen (outbuf));
 	}
	close(in); // Close the client socket in the child
 }
 //*******Draw hangman diagram*******/
void draw_hangman(int lives, int out)
 {
	char hangman[MAXLEN];// store characters of the maxinmum size of the array of 100
  switch(lives)//check the lives 
  {//draw hangman
    case 12:
      sprintf(hangman,"\n\n\n\n\n\n\n");
      break;
    case 11:
      sprintf(hangman,"\n\n\n\n\n\n=====\n");
      break;
    case 10:
      sprintf(hangman," |\n |\n |\n |\n |\n =====\n");
      break;
    case 9:
      sprintf(hangman,"+    \n |\n |\n |\n |\n |\n=====\n");
      break;
	case 8:
      sprintf(hangman,"+---+\n |  \n |  \n |\n |\n |\n=====\n");
      break;
    case 7:
      sprintf(hangman,"+---+\n |  |\n |  \n |\n |\n |\n=====\n");
      break;
    case 6:
      sprintf(hangman,"\+---+\n |  |\n |  O\n |  \n |\n=====\n");
      break;
    case 5:
      sprintf(hangman,"\+---+\n |  |\n |  O\n |  |\n |\n=====\n");
      break;
    case 4:
      sprintf(hangman,"\+---+\n |  |\n |  O\n | /|\n |\n |\n=====\n");
      break;
    case 3:
      sprintf(hangman,"\+---+\n |  |\n |  O\n | /|\\\n | \n |\n=====\n");
      break;
    case 2:
      sprintf(hangman,"\+---+\n |  |\n |  O\n | /|\\\n | /\n |\n=====\n");
      break;
    case 1:
      sprintf(hangman,"\+---+\n |  |\n |  O\n | /|\\\n | /\\\n |\n=====\n");
      break;
	case 0:
      sprintf(hangman,"Game Over!\n");
      break;
    default:
      sprintf(hangman,"\n\n\n\n\n\n\n");
      break;
  }
  write (out, hangman, strlen(hangman));// send out message of the hangman diagram
 }

