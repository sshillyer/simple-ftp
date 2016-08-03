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

	listening_sfd = get_socket_bind_to_port(NULL, port_str);
	listen(listening_sfd, backlog);

	struct sockaddr_storage their_addr;
	socklen_t addr_size;

	printf("Server open on %d\n", control_port);

	// Main accept() loop
	while (1) {
		// Cite: Beej's guide page 24-ish, and pg 28-29
		addr_size = sizeof their_addr;
		char * control_message = malloc(sizeof(char) * BUF_SIZE);
		char * file_name = malloc(sizeof(char) * BUF_SIZE);
		int i;
		for (i = 0; i < BUF_SIZE; i++) {
			control_message[i] = '\0';
			file_name[i] = '\0';
		}
		char command[BUF_SIZE];
		char client_ip_str[INET6_ADDRSTRLEN];
		int command_type;

		// Create control connection ("P")
		control_sfd = accept(listening_sfd, (struct sockaddr *)&their_addr, &addr_size);
		if (control_sfd == -1) {
			// Error out if accept fails and loop again listening for more
			perror("accept");
			clear_buff(control_message);
			if (control_message) free (control_message);
			continue;
		}
		
		// First we need ftclient's ip. Cite: Beej Page 29 example code
		inet_ntop(their_addr.ss_family, 
			get_in_addr( (struct sockaddr *)&their_addr), 
			client_ip_str, sizeof client_ip_str);

		// Print IP of the connecting server before proceeding
		printf("Connection from %s.\n", client_ip_str);

		// Read command from ftclient
		if (receive_string_from_client(control_sfd, control_message) == -1) {
			fprintf(stderr, "DEBUG STATEMENT: Unable to read command from client\n");
			close(control_sfd);
			clear_buff(control_message);
			if (control_message) free (control_message);
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
				clear_buff(control_message);
				if (control_message) free (control_message);
				continue;
			}

			data_port = convert_string_to_int(control_message);
			validate_port(control_port, errno);
			strncpy(data_port_str, control_message, sizeof data_port_str);
// DEBUG STATEMENTS			
			printf("DEBUG STATEMENT:Client sent dataport # as string: %s\n", control_message);
			printf("DEBUG STATEMENT: dataport converted to int: %d\n", data_port);
			printf("DEBUG STATEMENT: Server stored data_port_str as: %s\n", data_port_str);
// END DEBUG STATEMENTS
		}
		else {
			// Command not valid; send error message on control connection
			send_string_on_socket(control_sfd, "Invalid command");
			// close(control_sfd); // ftclient closes the connection P
			clear_buff(control_message);
			if (control_message) free (control_message);
			continue;
		}

		// Command was valid - open the data_connection. 
		printf("DEBUG STATEMENT: attempting to get_socket_connect_on_port(%s, %s)\n", client_ip_str, data_port_str);
		
		data_sfd = get_socket_connect_on_port(client_ip_str, data_port_str);
		if (data_sfd == -1 ) {
			printf("DEBUG STATEMENT: Error connecting on that port.\n");
			close(control_sfd);
			clear_buff(control_message);
			if (control_message) free (control_message);
			continue;
		}

		printf("DEBUG STATEMENT: data_sfd = %d\n", data_sfd);


		// Send the directory contents to ftclient on data connection Q
		if (command_type == LIST_COMMAND) {
			printf("List directory requested on port %d.\n", data_port);
			printf("Sending directory contents to %s:%s\n", client_ip_str, data_port_str);
			send_directory_contents(data_sfd);
		}


		// Send the filename (if it exists) to client
		else if (command_type == GET_COMMAND) {
			if (receive_string_from_client(control_sfd, file_name) == -1) {
				fprintf(stderr, "DEBUG STATEMENT: Unable to read filename from client\n");
				close(control_sfd);
				clear_buff(file_name);
				if (file_name) free (file_name);
				continue;
			}

			printf("File \"%s\" requested on port %d.\n", file_name, data_port);
			
			// Cite: stackoverflow.com/questions/3501338
			char * line = NULL;
			size_t len = 0;
			ssize_t read;
			FILE *fp = fopen(file_name, "r");
			if (fp == NULL) {
				printf("File not found or is not readable. Sending error message to %s:%s\n", client_ip_str, data_port_str);
			}

			while ((read = getline(&line, &len, fp)) != -1) {
				printf("Retrieved line of length %zu : \n", read);
				printf("%s", line);
			}

			fclose(fp);
			if(line) free(line);
			// THEN CLEANUP

			// if (is_file_readable(file_name) == 0) {
			// 	printf("File is readable.\n");
			// }
			// else {
			// 	printf("File is not readable.\n");
			// }
		}


		// Free up resources
		close(data_sfd);
		close(control_sfd); // A failsafe in case ftclient doesn't close connection P
		if (control_message) free (control_message);
	} // while loop. 9. ftserver repeats from 2 until terminated.

	close(listening_sfd); // TODO: Need to creat a sighandler that closes the port on SIGINT

	return 0;
}