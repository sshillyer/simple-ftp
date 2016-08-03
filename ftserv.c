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
	// Parse command line arguments, validate port number is good
	const char * port_str = argv[1];
	check_argument_count(argc, 2, "Usage: ftserv port\n");
	int control_port = convert_string_to_int(argv[1]), // Connection 'P'
	    data_port = -1; // For connection 'Q'
	validate_port(control_port, errno);

	// Set up a "listening" socket to listen for connections on port passed in
	int listening_sfd,
	    control_sfd,
	    data_sfd;
	int backlog = 5; // maximum connections to listen for and backlog

	listening_sfd = get_socket_bind_to_port(NULL, port_str);
	listen(listening_sfd, backlog);

	struct sockaddr_storage their_addr;
	socklen_t addr_size;

	while (1) {
		// Cite: Beej's guide page 24-ish, and pg 28-29
		addr_size = sizeof their_addr;
		char * control_message = malloc(sizeof(char) * BUF_SIZE);
		char command[BUF_SIZE];
		int command_type;

		// Create control connection ("P")
		control_sfd = accept(listening_sfd, (struct sockaddr *)&their_addr, &addr_size);
		if (control_sfd == -1) {
			// Error out if accept fails and loop again listening for more
			perror("accept");
			continue;
		}

		// Read command from ftclient
		if (receive_string_from_client(control_sfd, control_message) == -1) {
			fprintf(stderr, "DEBUG STATEMENT: Unable to read command from client\n");
			close(control_sfd);
			continue;
		}
		printf("DEBUG STATEMENT: Command received: \"%s\"\n", control_message);

		command_type = get_command_type(control_message);
		if (command_is_valid(command_type)) {
			printf("DEBUG STATEMENT:Command is valid.\n");
			send_string_on_socket(control_sfd, "DATAPORT?");

			// Read the dataport from client
			if (receive_string_from_client(control_sfd, control_message) == -1) {
				fprintf(stderr, "Unable to read dataport from client\n");
				send_string_on_socket(control_sfd, "DATAPORT ERR");
				// close(control_sfd); // ftclient closes the connection P
				continue;
			}
			printf("DEBUG STATEMENT:Client sent dataport # as string: %s\n", control_message);
			data_port = convert_string_to_int(control_message);
			validate_port(control_port, errno);
			printf("DEBUG STATEMENT: dataport converted to int: %d\n", data_port);
		}
		else {
			// Send error message on control socket
			send_string_on_socket(control_sfd, "Invalid command");
			// close(control_sfd); // ftclient closes the connection P
			continue;
		}

		// Command was valid. 

		if (command_type == LIST_COMMAND) {

		}


		// Free up resources
		close(control_sfd); // TODO: Move this to the client 8. ftclient closes connection P
		if (control_message) free (control_message);
	} // while loop. 9. ftserver repeats from 2 until terminated.

	close(listening_sfd); // TODO: Need to creat a sighandler that closes the port on SIGINT

	return 0;
}