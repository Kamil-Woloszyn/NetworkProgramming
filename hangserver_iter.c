 /* Network server for hangman game */
 /* File: hangserver.c */

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

 extern time_t time ();
//hello world
 int maxlives = 12;
 char *word [] = {
 # include "words"
 };
 # define NUM_OF_WORDS (sizeof (word) / sizeof (word [0]))
 # define MAXLEN 80 /* Maximum size in the world of Any string */
 # define HANGMAN_TCP_PORT 1066
 void play_hangman(int in, int out);
 void draw_hangman(int in, int out);
int main()
 {
 	//int sock, fd, client_len;
 	//struct sockaddr_in server, client;
	int sock, fd, option, r;
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


 	srand ((int) time ((long *) 0)); /* randomize the seed */

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
			
	//Accept connections and handle each one of the new fork process
 	while (1) {
 		client_len = sizeof(client_address);
		fd = accept (sock, (struct sockaddr *) &client_address, &client_len);
 		if (fd <0) {
 			perror ("accepting connection");
 			exit (3);
 		}
		int pid = fork();//create fork valuable
		if(pid < 0)// if fork failed
		{
			perror("fork failed");
			close(fd);// close client
			exit(1);
		}
		if(pid == 0)//check child process
		{
			close(sock);//Child doesn't need the listening socket
			play_hangman(fd, fd);// play the hang man with client
			close(fd);//Close the game
			exit(0);//End the clid process
		}
		else//check parent process
		{
			close(fd);//close parent socket
		}
 		close (fd);//close client socket
 	}
	
	//Clean up
	close(sock);
	return 0;
 }

 /* ---------------- Play_hangman () ---------------------*/

 void play_hangman(int in, int out)
{
    char *whole_word, part_word[MAXLEN], guess[MAXLEN], outbuf[MAXLEN];
    int lives = maxlives;
    int game_state = 'I'; // I = Incomplete
    int i, good_guess, word_length;
    char hostname[MAXLEN];

    gethostname(hostname, MAXLEN);
    sprintf(outbuf, "Playing hangman on host %s: \n\n", hostname);
    write(out, outbuf, strlen(outbuf));

    /* Pick a word at random from the list */
    whole_word = word[rand() % NUM_OF_WORDS];
    word_length = strlen(whole_word);
    syslog(LOG_USER | LOG_INFO, "server chose hangman word %s", whole_word);

    /* No letters are guessed Initially */
    for (i = 0; i < word_length; i++)
        part_word[i] = '-';
    part_word[i] = '\0';

    sprintf(outbuf, "%s %d \n", part_word, lives);
    write(out, outbuf, strlen(outbuf));

    while (game_state == 'I') {
        /* Get a letter from player guess */
        while (1) {
            // Read exactly one character (no more, no less)
            ssize_t r = read(in, guess, 1);
            if (r < 0) {
                if (errno != EINTR) {
                    perror("Error reading input");
                    exit(4);
                }
            } else if (r == 0) {
                // No data read (client may have disconnected)
                break;
            } else {
                // Make sure only one character was received
                guess[0] = tolower(guess[0]); // Ensure lowercase for consistency

                if (!isalpha(guess[0])) {
                    // If the input is not a letter, reject it
                    sprintf(outbuf, "Invalid input! Please enter a single letter.\n");
                    write(out, outbuf, strlen(outbuf));
                } else {
                    // Process the valid guess
                    good_guess = 0;
                    for (i = 0; i < word_length; i++) {
                        if (guess[0] == whole_word[i]) {
                            good_guess = 1;
                            part_word[i] = whole_word[i];
                        }
                    }

                    if (!good_guess) {
                        lives--;
                        draw_hangman(lives, out); // Draw the hangman
                    }

                    if (strcmp(whole_word, part_word) == 0) {
                        game_state = 'W'; /* W ==> User Won */
                    } else if (lives == 0) {
                        game_state = 'L'; /* L ==> User Lost */
                        strcpy(part_word, whole_word); /* Show the whole word */
                    }

                    // Send the updated word and lives to the client
                    sprintf(outbuf, "%s %d \n", part_word, lives);
                    write(out, outbuf, strlen(outbuf));

                    break; // Break out of the input loop once a valid input has been processed
                }
            }
        }
    }
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
      sprintf(hangman,"|\n |\n |\n |\n |\n =====\n");
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
