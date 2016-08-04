/*******************************************************************************
* File:         ftserv.c
* Author:       Shawn S Hillyer
* Date:         July 28, 2016
* Course:       OSU CSS 372: Project 2
*
* Description:  Simple file transfer server. Can send a list to a client that
*               follows the protocol, or send a file. Uses two sockets (one
*               for control, one for data)
*               
* Usage:        ftserv port
*               
* Cite:         Overall flow of a socket-based client/server pair of programs: 
*               beej.us/guide/bgipc/output/html/multipage/unixsock.html  
*               Other sources cited in specific places in ftserv.c & ftserv.h
*
* Note:         Portions of this program were re-used from assignment 4 that I
*               completed in CS 344 (Specifically, some header file functions)
*******************************************************************************/

#include "ftserv.h"


/*******************************************************************************
* main()
* Listens for connection request on port provided on command line
*******************************************************************************/
int main(int argc, char const *argv[]) {
	// Parse command line arguments, validate port number is good
	const char * port_str = argv[1];
	char data_port_str[PORT_MAX_LEN];
	check_argument_count(argc, 2, "Usage: ftserv port\n");
	int control_port = convert_string_to_int(argv[1]), // Connection 'P'
	    data_port = -1; // For connection 'Q'
	validate_port(control_port, errno);

	// Set up a "listening" socket to listen for connections on port passed in
	int listening_sfd,
	    control_sfd,
	    data_sfd;
	int backlog = 5; // maximum connections to listen for and backlog

	// Get address info, call socket(), and call bind() using function
	listening_sfd = get_socket_bind_to_port(NULL, port_str);
	
	// Now listen on the new bound socket
	listen(listening_sfd, backlog);

	// These variables are used to store the client's address info
	struct sockaddr_storage their_addr;
	socklen_t addr_size;

	// Echo to console that server is open on the control port specified
	printf("Server open on %d\n", control_port);

	// Main accept() loop
	while (1) {
		// Cite: Beej's guide page 24-ish, and pg 28-29

		// Allocate buffers to store control connection and the file name
		addr_size = sizeof their_addr;
		char * control_message = malloc(sizeof(char) * BUF_SIZE);
		char * file_name = malloc(sizeof(char) * BUF_SIZE);
		
		// NULL-fill both buffers - not doing so is unsafe in my testing
		int i;
		for (i = 0; i < BUF_SIZE; i++) {
			control_message[i] = '\0';
			file_name[i] = '\0';
		}

		// Several other buffers to store strings and the command_type
		char command[BUF_SIZE];                // the command sent to server
		char client_ip_str[INET6_ADDRSTRLEN];  // Client's IP address as a string
		int command_type;                      // See ftserv.h - Can be LIST_COMMAND or GET_COMMAND

		// Create control connection ("P") by accepting a connection.
		control_sfd = accept(listening_sfd, (struct sockaddr *)&their_addr, &addr_size);
		if (control_sfd == -1) {
			// Error out if accept fails and loop again listening for more
			perror("accept");
			clear_buff(control_message);
			if (control_message) free (control_message);
			continue;
		}
		
		// We also need ftclient's ip so we can connect back to it again on the data connection
		// Cite: Beej Page 29 example code
		inet_ntop(their_addr.ss_family, 
			get_in_addr( (struct sockaddr *)&their_addr), 
			client_ip_str, sizeof client_ip_str);

		// Print IP of the connecting server before proceeding
		printf("Connection from %s.\n", client_ip_str);

		// Read command from ftclient
		if (receive_string_from_client(control_sfd, control_message) == -1) {
			// Fail-safe. Close socket and try accepting again if read fails
			fprintf(stderr, "ERROR: Unable to read command from client\n");
			close(control_sfd);
			clear_buff(control_message);
			if (control_message) free (control_message);
			continue;
		}

		// printf("DEBUG STATEMENT: Command received: \"%s\"\n", control_message);

		// Set the command_type integer depending on what was sent from client
		command_type = get_command_type(control_message);

		// If the command sent is a valid command, send back query to client asking for
		// the dataport on which we should open a data connection.
		if (command_is_valid(command_type)) {
			// printf("DEBUG STATEMENT:Command is valid.\n");
			send_string_on_socket(control_sfd, "DATAPORT?");

			// Read the dataport from client
			if (receive_string_from_client(control_sfd, control_message) == -1) {
				// Another failsafe to loop and accept() again on failure
				fprintf(stderr, "ERROR: Unable to read dataport from client\n");
				send_string_on_socket(control_sfd, "DATAPORT ERR");
				// close(control_sfd); // ftclient closes the connection P
				clear_buff(control_message);
				if (control_message) free (control_message);
				continue;
			}

			// Parse the data port and convert it to an integer for validation
			data_port = convert_string_to_int(control_message);
			validate_port(control_port, errno);

			// Copy the message to another buffer for safekeeping and re-use later
			strncpy(data_port_str, control_message, sizeof data_port_str);

			// DEBUG STATEMENTS
			// printf("DEBUG STATEMENT:Client sent dataport # as string: %s\n", control_message);
			// printf("DEBUG STATEMENT: dataport converted to int: %d\n", data_port);
			// printf("DEBUG STATEMENT: Server stored data_port_str as: %s\n", data_port_str);
			// END DEBUG STATEMENTS
		}

		// else command is not valid, send error message on control connection
		else {
			send_string_on_socket(control_sfd, "Invalid command");
			// close(control_sfd); // ftclient closes the connection P
			clear_buff(control_message);
			if (control_message) free (control_message);
			continue;
		}


		// printf("DEBUG STATEMENT: attempting to get_socket_connect_on_port(%s, %s)\n", client_ip_str, data_port_str);
		
		// If we reached this far, then command valid and we got a valid dataport to communicate on.
		
		// Open a new socket to send data back-and-forth
		data_sfd = get_socket_connect_on_port(client_ip_str, data_port_str);
		if (data_sfd == -1 ) {
			printf("ERROR: Unable to open data connection on the port provided.\n");
			close(control_sfd);
			clear_buff(control_message);
			if (control_message) free (control_message);
			continue;
		}

		// printf("DEBUG STATEMENT: data_sfd = %d\n", data_sfd);


		// IF sent '-l' command, Send the directory contents to ftclient on data connection Q
		if (command_type == LIST_COMMAND) {
			printf("List directory requested on port %d.\n", data_port);
			printf("Sending directory contents to %s:%s\n", client_ip_str, data_port_str);
			send_directory_contents(data_sfd);
		}


		// ELSE IF sent '-g' command, send the file called 'filename' (if it exists) to client
		else if (command_type == GET_COMMAND) {

			// First, read the filename from client
			if (receive_string_from_client(control_sfd, file_name) == -1) {
				fprintf(stderr, "DEBUG STATEMENT: Unable to read filename from client\n");
				close(control_sfd);
				clear_buff(file_name);
				if (file_name) free (file_name);
				continue;
			}

			printf("File \"%s\" requested on port %d.\n", file_name, data_port);
			
			char * end_of_file_msg = "FTCLIENT END FILE";
			if (send_file_strings(data_sfd, file_name, control_sfd) == -1) {
				printf("File not found or is not readable. Sending error message to %s:%s\n", client_ip_str, data_port_str);
				// Cleanup occurs in the final section of while loop just below.
			}
		}


		// Free up resources - close the data connection and control connection.
		close(data_sfd);
		close(control_sfd); // A failsafe in case ftclient doesn't close connection P
		if (control_message) free (control_message);
	} // while loop

	close(listening_sfd); // No way for this to really trigger - not sure how to pass the variable to SIGINT handler?

	return 0;
}