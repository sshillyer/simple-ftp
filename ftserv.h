/*******************************************************************************
* File:         ftserv.c
* Author:       Shawn S Hillyer
* Date:         July 28, 2016
* Course:       OSU CSS 372: Project 2
*
* Description:  Include headers, constants, & common functions for file transfer
*               server
*               
* Usage:        #include <"ftserv.h">
*               
* Cite:         Overall flow of a socket-based client/server pair of programs: 
*               beej.us/guide/bgipc/output/html/multipage/unixsock.html  
*               Other sources cited in specific places in ftserv.c & ftserv.h
*
* Note:         Portions of this program were re-used from assignment 4 that I
*               completed in CS 344 (Specifically, some header file functions)
*******************************************************************************/

#ifndef SSHILLYER_FTSERV_H
#define SSHILLYER_FTSERV_H
#define _GNU_SOURCE

#include <arpa/inet.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/unistd.h>

// Constants

#define MIN_PORT_NUMBER 1
#define MAX_PORT_NUMBER 65535
#define BUF_HANDLE 12
#define BUF_SIZE 1024
#define BUF_MSG 500
#define MAX_FILENAME_LEN 500
#define READ_MODE 1
#define WRITE_MODE 2
#define LIST_COMMAND 1
#define GET_COMMAND 2
#define INVALID_COMMAND -1
#define PORT_MAX_LEN 6

/*******************************************************************************
* void check_argument_count(int arg_c, int req, const char * message)
* : validates that arg_c == req and prints message if not
* argc: number of arguments expected
* req: number of arguments required
* message: usage message
*******************************************************************************/
void check_argument_count(int arg_c, int req, const char * message) {
	if (arg_c != req) {
		fprintf(stderr, "%s", message);
		exit(EXIT_FAILURE);
	}
}


/*******************************************************************************
* int convert_string_to_int(const char * string)
* string: A null terminated string to be converted to an integer.
* Takes a string argument and returns best representation of it as a string
*******************************************************************************/
int convert_string_to_int(const char * string) {
	// parse port from command line argument and check result
	// Even though we are using the string version of the port, validate as an int
	errno = 0; // 0 out before evaluating the call to strtol
	int result = strtol(string, NULL, 10);
	if (errno == ERANGE) {
		fprintf(stderr, "The string '%s' converts to an integer outside the valid range.\n", string);
		exit(EXIT_FAILURE);
	}
	return result;
}


/*******************************************************************************
* void validate_port(int port, int err)
* confirm that port was parsed within valid range for ports
* Cite: man strtol, example section
*******************************************************************************/
void validate_port(int port, int err) {
	if ((errno == ERANGE && (port == LONG_MAX || port == LONG_MIN)) 
		|| (errno != 0 && port == 0) 
		|| (port > MAX_PORT_NUMBER || port < MIN_PORT_NUMBER)) 
	{

		fprintf(stderr, "strtol() cannot convert port, invalid format or out of range\n");
		exit(EXIT_FAILURE);
	}
}


/*******************************************************************************
* void safe_transmit_msg_on_socket(int message_length, int fd, char * buffer, int mode) {
* Safely sends a string on the socket by looping until all bytes are sent.
* Has a read and a write mode that can be set using the constants defined in this 
* header (READ_MDOE, WRITE_MODE)
*******************************************************************************/
void safe_transmit_msg_on_socket(int fd, char * buffer, int message_length, int mode) {
	int bytes_remaining = message_length, bytes_transmitted = 0;
	while ( bytes_remaining > 0 ) {
		int i = message_length - bytes_remaining;
		if (mode == READ_MODE)
			bytes_transmitted = read(fd, buffer + i, bytes_remaining);
		else if (mode == WRITE_MODE)
			bytes_transmitted = write(fd, buffer, bytes_remaining);
		bytes_remaining -= bytes_transmitted;
	}
	if (bytes_transmitted == -1) {
		if (mode == 1)
			perror("read");
		else if (mode == 2)
			perror("write");	
		exit(EXIT_FAILURE);
	}
}


/*******************************************************************************
* get_socket_bind_to_port(const char * ip, const char * port) 
* Cite: Beej's Guide, all of the chapters on creating a socket, binding, etc. 
* Uses the ip and port number to get addr info, a socket, and binds it to port
*******************************************************************************/
int get_socket_bind_to_port(const char * ip, const char * port) {
	// Variables used in getaddrinfo() call
	int status,
	    sfd; // socket file descriptor
	struct addrinfo hints;
	struct addrinfo *res;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	status = getaddrinfo(ip, port, &hints, &res);
	if (status != 0) {
		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
	}
	
	// Make a socket
	sfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (sfd == -1) {
		fprintf(stderr, "socket did not return valid socket\n");
		return sfd;
	}

	// Bind the socket to the port passed in to getaddrinfo
	status = bind(sfd, res->ai_addr, res->ai_addrlen);
	if (status == -1) {
		fprintf(stderr, "bind failed to bind socket\n");
		exit(EXIT_FAILURE);
	}

	// Free the response from getaddrinfo
	freeaddrinfo(res);

	// Need socket file descriptor to reference in calling code
	return sfd; 
}


/*******************************************************************************
* get_socket_connect_on_port(const char * ip, const char * port)
* Cite: Beej's Guide, all of the chapters on creating a socket, binding, etc. 
* Gets addr info and a socket, then connects to the ip:port combo passed in
*******************************************************************************/
int get_socket_connect_on_port(const char * ip, const char * port) {
	// Variables used in getaddrinfo() call
	int status,
	    sfd; // socket file descriptor
	struct addrinfo hints;
	struct addrinfo *res;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	status = getaddrinfo(ip, port, &hints, &res);
	if (status != 0) {
		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
	}
	
	// Make a socket
	sfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (sfd == -1) {
		fprintf(stderr, "socket did not return valid socket\n");
		return sfd;
	}

	// Connect to the sfd
	status = connect(sfd, res->ai_addr, res->ai_addrlen);
	if (status == -1) {
		perror("connect");
		return status;
	}

	// Free the response from getaddrinfo
	freeaddrinfo(res);

	// Need socket file descriptor to reference in main()
	return sfd; 
}


/*******************************************************************************
* void clear_buff(char * buff) 
* Clears a buffer by placing null terminator in every element
*******************************************************************************/
void clear_buff(char * buff) {
	int len = strlen(buff);
	int i;
	for (i = 0; i < len; i++) {
		buff[i] = '\0';
	}
}


/*******************************************************************************
* void send_string_on_socket(int sfd, char * msg) 
* Sends msg to the socket identified by sfd
*******************************************************************************/
void send_string_on_socket(int sfd, char * msg) {
	int len = strlen(msg);
	safe_transmit_msg_on_socket(sfd, msg, len, WRITE_MODE);
}


/*******************************************************************************
* int receive_string_from_client(int sfd, char * buffer)
* Reads a string on sfd and stores it in buffer. Returns -1 on fail, 0 success
*******************************************************************************/
int receive_string_from_client(int sfd, char * buffer) {
	int bytes_transmitted;
	if (bytes_transmitted = recv(sfd, buffer, BUF_SIZE, 0) <= 0) {
		if (bytes_transmitted == 0) {
			// Connection closed by client
			printf("client disconnected\n");
		}
		else {
			perror("recv");
		}
		close(sfd);
		return -1;
	}

	return 0;
}


/*******************************************************************************
* int get_command_type(char * command)
* Parses a command and returns an integer representing the command type
*******************************************************************************/
int get_command_type(char * command) {
	if (strcmp(command, "-l") == 0) {
		return LIST_COMMAND;
	}
	else if (strcmp(command, "-g") == 0) {
		return GET_COMMAND;
	}
	else return INVALID_COMMAND;
}


/*******************************************************************************
* int command_is_valid(int command_type) 
* Returns true (1) if the integer passed in represents a valid command type, 0
* if not.
*******************************************************************************/
int command_is_valid(int command_type) {
	if (command_type == LIST_COMMAND || command_type == GET_COMMAND)
		return 1;
	else
		return 0;
}


/*******************************************************************************
* int send_directory_contents(int sfd)
* Executes all steps required to send the results of the 'ls' command to client
* identified by sfd.
*******************************************************************************/
int send_directory_contents(int sfd) {
	char * end_of_dir_msg = "FTCLIENT END DIR LIST";
	char ackdump[10];

	// Citation: gnu.org/savannah-checkouts/gnu/libc/manual/html_node/Simple-Directory-Lister.html)
	DIR *dp;
	struct dirent *ep;

	// Open the pwd
	dp = opendir ("./");
	if (dp != NULL) {
		// Send each item from the directory to the sfd passed in
		while (ep = readdir (dp)) {
			send_string_on_socket(sfd, ep->d_name);
			receive_string_from_client(sfd, ackdump); // Trick to chunk the write() calls
		}
		// Close the file
		(void) closedir (dp);
	}
	// If file could not be opened, return -1
	else {
		perror("Couldn't open the directory\n");
		return -1;
	}

	// Send a "goodbye" message to client
	send_string_on_socket(sfd, "FTSERVBYE");

	return 0; // success
}


/*******************************************************************************
* int send_file_strings(int sfd, const char * file_name, int control_sfd)
* Sends a text file identified by file_name using sfd, passing an ack or neg 
* message to control_sfd so that client knows whether to receive an error msg
* or the store the strings in a file
*******************************************************************************/
int send_file_strings(int sfd, const char * file_name, int control_sfd) {

	char ackdump[10];

	// Cite: Inspriation from stackoverflow.com/questions/3501338
	char * line = NULL;
	size_t len = 0;
	ssize_t read;

	FILE *fp = fopen(file_name, "r");
	if (fp == NULL) {
		send_string_on_socket(control_sfd, "NEG");
		return -1;
	}

	send_string_on_socket(control_sfd, "ACK");
	
	while ((read = getline(&line, &len, fp)) != -1) {
		send_string_on_socket(sfd, line);
		receive_string_from_client(sfd, ackdump); // Trick to chunk the write() calls
	}

	send_string_on_socket(sfd, "FTSERVBYE");

	fclose(fp);
	if(line) free(line);

	return 0;
}



/*******************************************************************************
* void *get_in_addr(struct sockaddr *sa)
* Cite: This function borrowed from Page 28 of Beej guide
* Returns a struct depending on the type of IP address acquired from getaddrinfo
*******************************************************************************/
void *get_in_addr(struct sockaddr *sa) {
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

#endif