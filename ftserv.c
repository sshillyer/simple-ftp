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
	int listening_sfd = get_socket_bind_to_port(NULL, port_str);
	int backlog = 5; // maximum connections to listen for and backlog
	listen(listening_sfd, backlog);

	struct sockaddr_storage their_addr;
	socklen_t addr_size;
	int control_socket;

	addr_size = sizeof their_addr;

	while (1) {
		control_socket = accept(listening_sfd, (struct sockaddr *)&their_addr, &addr_size);


		close(control_socket);
	}

	close(listening_sfd);

	return 0;
}