################################################################################
# File:         ftclient.py
# Author:       Shawn S Hillyer
# Date:         July 29, 2016
# Course:       OSU CSS 372: Project 2
#
# Description:  
#               
# Usage:        ftclient serverHost serverPort -command <filename> dataPort
#               
# Citations:    Textbook page 167 for basic tcp python code
#               Pyton Essential Reference - 4th Edition  ("PER")
################################################################################

from socket import *
import sys

if len(sys.argv) != 2:
	print("Usage: python3 ftserv serverPort")
	quit()

serverPort = int(str(sys.argv[1]))

serverSocket = socket(AF_INET, SOCK_STREAM)
serverSocket.bind(('', serverPort))
# serverSocket.connect((serverHost, serverPort))  # accepts a tuple, the hostname and the port # as an int
serverSocket.listen(5)

while True:
	client, addr = serverSocket.accept()
	print("Got a connection from %s", str(addr))
# Once connected, send the command and the dataPort we wish to use

# Our protocol will wait for the control connection on serverSocket to send a
# response that states the command is good and the filename (if -g command) is good
# If those are okay, we ack the response and start listening on dataPort
# The server will wait for our "ack" and then initiate a new data connection on dataPort

# Once data connection is established, we will receive the file contents and store them
# in a file called fileName (which was provided on the command line). If filename already
# exists, we can a) abort the entire thing or b) prompt user for a new filename? (Maybe extra credit here?)



# dataSocket = socket(AFI_INET, SOCK_STREAM)
# dataSocket.bind(('', dataPort))
# dataSocket.listen()

# Main server loop: listen for incoming connection
# while 1:
# 	connectionSocket, addr = serverSocket.accept()
# 	again = True

# 	# Once connected, exchange messages until quit message received
# 	while again:
# 		userInput = ''
# 		message = ''  # TODO: try deleting this line, prob superflous
# 		message = connectionSocket.recv(513)
# 		message = message.decode()

# 		if message == "\quit":
# 			again = False
# 			print("Peer has requested disconnect.\nServer is listening for more connections.")
# 			break
# 		else:
# 			print(message)
# 			userInput = input(serverHandle + "> ")
# 			userInput.rstrip('\n')
# 			if userInput != "\quit":
# 				response = serverHandle + "> " + userInput
# 			else:
# 				response = userInput
# 			connectionSocket.sendall(response.encode())
# 			if userInput == '\quit':
# 				print("Sent \quit command, disconnected from client.\nServer is listening for more connections.")
# 				break

	# Once done exchanging messages, close the socket
	# connectionSocket.close()