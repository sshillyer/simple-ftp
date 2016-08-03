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
	int port = convert_string_to_int(argv[1]);
	validate_port(port, errno);

	// Set up a "listening" socket to listen for connections on port passed in
	int listening_sfd,
	    control_sfd;
	int backlog = 5; // maximum connections to listen for and backlog

	listening_sfd = get_socket_bind_to_port(NULL, port_str);
	listen(listening_sfd, backlog);

	struct sockaddr_storage their_addr;
	socklen_t addr_size;

	while (1) {
		// Cite: Beej's guide page 24-ish, and pg 28-29
		addr_size = sizeof their_addr;
		char control_message[BUF_SIZE];

		// Create control connection ("P")
		control_sfd = accept(listening_sfd, (struct sockaddr *)&their_addr, &addr_size);
		if (control_sfd == -1) {
			// Error out if accept fails and loop again listening for more
			perror("accept");
			continue;
		}

		receive_command_from_client(control_sfd, control_message);
		send_string_on_socket(control_sfd, "Hello world");

		close(control_sfd);
	}

	close(listening_sfd);

	return 0;
}