/*******************************************************************************
* File:         ftserv.c
* Author:       Shawn S Hillyer
* Date:         July 28, 2016
* Course:       OSU CSS 372: Project 2
*
* Description:  

*               
* Usage:        ftserv port
*               
* Cite:         Overall flow of a socket-based client/server pair of programs: 
*               beej.us/guide/bgipc/output/html/multipage/unixsock.html  
*
* Note:         Portions of this program were re-used from assignment 4 that I
*               completed in CS 344.
*******************************************************************************/

#include "ftserv.h"


/*******************************************************************************
* main()
* Listens for connection request on port provided on command line
*******************************************************************************/
int main(int argc, char const *argv[]) {
	// allocate space to hold messages and various strings
	char * message;
	char * client_message;
	char * payload;
	const char * port_str = argv[1];
	const char * client_handle;
	char s[INET6_ADDRSTRLEN]; // used to store the connecting host

	// Other variables used - see page 23 beej
	socklen_t addr_size; 
	struct sockaddr_storage client_addr;
	struct hostent * he;

	// Verify Arguments are valid, print error message if not
	check_argument_count(argc, 2, "Usage: ftserv port\n");


	// Parse and validate port, save port as a string for loading address
	int port = convert_string_to_int(argv[1]);
	validate_port(port, errno);


	// Variables for sockets and the server address (See page 16 of beej guide)
	int sfd, control_sfd, data_sfd, status; 
	struct addrinfo hints, *servinfo, *clientinfo;
	struct in_addr **addr_list;
	
	// 0 out hints struct then init to connect to hostname via TCP
	// Cite: lecture slides, man getaddrinfo(3), and beej guide - random bits
	// Use the getaddrinfo() to fill out servinfo by passing in some 'hints'
	memset(&hints, 0, sizeof hints);  // clear out the hints struct for safety
	hints.ai_family = AF_INET;        // AF_UNSPEC would be unspecified IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM;  // Use TCP -- need 2-way communication
	hints.ai_flags = AI_PASSIVE;      // fill in localhost ip

	// populate servinfo using the hints struct (Cite beej pg. 17 sample)
	if ( (status = getaddrinfo(NULL, port_str, &hints, &servinfo)) != 0) {
		perror_exit("getaddrinfo", EXIT_FAILURE);
	}


	// Now open a TCP socket stream; Cite: Slide 10 Unix Networking 2 (lecture)
	// Cite: Beej network guide for using hints structure, page 
	// Must be called after getaddrinfo() so that servinfo struct is populated
	if ((sfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol)) == -1) {
		perror_exit("socket", EXIT_FAILURE);
	}


	// Next, bind socket (sfd) to a port (See beej page 20)
	if (bind(sfd, servinfo->ai_addr, servinfo->ai_addrlen) == 01) {
		perror_exit("bind", EXIT_FAILURE);
	}

	char myhostname[128];
	gethostname(myhostname, 128);
	gethostbyname(myhostname);
	printf("Server open on %s:%d\n", myhostname, port);

	// Inspired by beej guide page 29)
	while(1) {
		// Listen for connections (Beej's guide 22-23)
		if (listen(sfd, 5) == -1) {
			perror_exit("listen", EXIT_FAILURE);
		}

		// Accept the connection
		addr_size = sizeof client_addr;
		// if (control_sfd = accept(sfd, (struct sockaddr *)&client_addr, &addr_size) == -1) {
		if ((control_sfd = accept(sfd, (struct sockaddr *)&client_addr, &addr_size)) == -1) {
			perror_exit("accept", EXIT_FAILURE);
		}
		printf("accept() called, control_sfd = %d\n", control_sfd);
	
		// Figure out who connected and print it to screen (should be stored in )
		// populate servinfo using the hints struct (Cite beej pg. 15-26 sample)
		// THIS BLOCK FROM PAGE 79 - Seemed best way to get the info
		// TODO: Convert the block to a function (make sure to credit!)
		socklen_t len;
		struct sockaddr_storage addr;
		char ipstr[INET6_ADDRSTRLEN];  // String to store iP in
		int data_port, client_port;

		len = sizeof addr;
		getpeername(control_sfd, (struct sockaddr*)&addr, &len);


		// deal with both IPv4 and IPv6:
		if (addr.ss_family == AF_INET) {
			struct sockaddr_in *s = (struct sockaddr_in *)&addr;
			client_port = ntohs(s->sin_port);
			inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof ipstr);
			// getnameinfo(s, s->ai_addrlen, host, sizeof host, service, sizeof service, 0);
		} 
		else { // AF_INET6
			struct sockaddr_in6 *s = (struct sockaddr_in6 *)&addr;
			client_port = ntohs(s->sin6_port);
			inet_ntop(AF_INET6, &s->sin6_addr, ipstr, sizeof ipstr);
			// getnameinfo(s, sizeof &s, host, service, sizeof service, 0);
		}
		// END RIP
		
		memset(&hints, 0, sizeof hints);  // clear out the hints struct for safety
		hints.ai_family = AF_INET;        // AF_UNSPEC would be unspecified IPv4 or IPv6
		hints.ai_socktype = SOCK_STREAM;  // Use TCP -- need 2-way communication
		hints.ai_flags = AI_CANONNAME;      // fill in localhost ip
		if ( (status = getaddrinfo(ipstr, port_str, &hints, &servinfo)) != 0) {
			perror_exit("getaddrinfo", EXIT_FAILURE);
		}

		char client_name[1024];
		if (servinfo->ai_canonname != NULL)
			strncpy(client_name, servinfo->ai_canonname, 1023);
		client_name[1023] = '\0';

		printf("Connection from %s\n", client_name); // Wonder if we can bind the client to use the same port to send??

		// Parse the incoming command
		// client_message is freed before the "chat loop" ends; not doing causes excess left in string
		client_message = malloc(sizeof(char) * BUF_MSG);
		int z;
		for (z = 0; z < BUF_MSG; z++) {
			// We have to zero out the mssage to be sure it gets null terminated
			client_message[z] = '\0';
		}

		// printf("About to call read()\n");
		

		// Cite: Handling recv() from Beej's page 39-40
		int bytes_transmitted;
		if (bytes_transmitted = recv(control_sfd, client_message, sizeof client_message, 0) <= 0) {
			if (bytes_transmitted == 0) {
				// Connection closed by client
				printf("client disconnected\n");
			}
			else {
				perror("recv");
			}
			close(control_sfd);
			break;
		}
		bytes_transmitted = 0;
		// printf("Right after read()\n");

		int commandIsList = 0;
		int commandIsGet = 0;

		if (strcmp(client_message, "-l") == 0) {
			// printf("client_message == -l\n");
			commandIsList = 1;
		}
		else if (strcmp(client_message, "-g") == 0) {
			commandIsGet = 1;
		}
		// else if command is not valid, send error message on control_sfd to client, close open socket P, continue while()
		else {
			char * invalid_command_msg = "Invalid command";
			safe_transmit_msg_on_socket(control_sfd, invalid_command_msg, sizeof invalid_command_msg, WRITE_MODE);
		}

		// get/open a second connection, assign it to data_sfd, connecting back to clients ip and using <dataport> (which it sent over)

		// if client sent -l command, send dir() listing to client
		if(commandIsList) {
			printf("List directory requested on port %d\n", data_port);
			// send the info!
		}

		char * file_name = malloc(sizeof (char *) * MAX_FILENAME_LEN);
		file_name = "hardcoded.txt";
		if(commandIsGet) {
			printf("File \"%s\" requested on port %d\n", file_name, data_port);

			// validate filename then send file if it exists

			// If error opening/finding file, close connection Q (data_sfd) and send error message on connection P (control_sfd)

			// else send the file
		}
		
		// Close the data_sfd connection

		// close the control_sfd connection
		

		// Free client_message so we can re-use it		
		if (client_message) {
			memset(&client_message[0], 0, sizeof(client_message)); // just in case the array had values from before
			free(client_message);
			client_message = NULL;
		}
		
		
	}



	// Free the dynamic allocated memory we used
	if (servinfo) 
		free(servinfo); // freeaddrinfo() ??
	return 0;

}