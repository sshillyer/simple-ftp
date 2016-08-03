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

# from socket import *
import sys
import socket

# Validate correct number of arguments
if len(sys.argv) < 5:
	print("Usage: python3 ftclient serverHost serverPort -[l|g] <filename> dataPort")
	quit()

serverHost = str(sys.argv[1])
serverPort = int(str(sys.argv[2]))
command = str(sys.argv[3])
fileName = ''

# Parse the filename only if command is to -g (get)
if command == "-l" and len(sys.argv) == 5:
	dataPort = int(str(sys.argv[4]))
elif command == "-g" and len(sys.argv) == 6:
	fileName = str(sys.argv[4])
	dataPort = int(str(sys.argv[5]))
else:
	print("Usage: python3 ftclient serverHost serverPort -[l|g] <filename> dataPort")
	quit()


# DEBUG statements: Echo the variables read in:
print("serverHost = " + serverHost)
print("serverPort = " + str(serverPort))
print("command = " + command)
if fileName:
	print("fileName = " + fileName)
print("dataPort = " + str(dataPort))


# TODO: Validate all of the arguments (Be sure to modularize and write functions)



# END VALIDATION SECTION

# Next, open the socket and connect to the server
# Cite: PER Pg 451 : a 6 line client that receives time from a server
serverSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
serverSocket.connect((serverHost, serverPort))  # accepts a duple, the hostname and the port #


# Once connected, send the command and the dataPort we wish to use
print("Command was: " + command)
command.rstrip('\n') # prob not necessary

if serverSocket.sendall(command.encode()) != None:
	print("sendall failed")

response = serverSocket.recv(513)
print("Response:\n")
print(response)

if response == "GET DATAPORT":
	print("Server wants the data port #")
	print("Sending: '" + str(dataPort) + "'")
	serverSocket.sendall(str(dataPort).encode())

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