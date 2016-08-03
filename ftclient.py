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
import os.path

if len(sys.argv) < 5:
	print("Usage: python3 ftclient serverHost serverPort -[l|g] <filename> dataPort")
	quit()

serverHost = str(sys.argv[1])
serverPort = int(str(sys.argv[2]))
command = str(sys.argv[3])
fileName = ''

if command == "-l" and len(sys.argv) == 5:
	dataPort = int(str(sys.argv[4]))
elif command == "-g" and len(sys.argv) == 6:
	fileName = str(sys.argv[4])
	dataPort = int(str(sys.argv[5]))
else:
	print("Usage: python3 ftclient serverHost serverPort -[l|g] <filename> dataPort")
	quit()

controlSocket = socket(AF_INET, SOCK_STREAM)
controlSocket.connect((serverHost, serverPort))  # accepts a duple, the hostname and the port #

# Once connected, send the command and the dataPort we wish to use
print("Command was: " + command)
command.rstrip('\n') # prob not necessary

if controlSocket.sendall(command.encode()) != None:
	print("sendall failed")

# The respone here worked:
response = controlSocket.recv(1024)
response = response.decode()

print("Response:")
print(response)

# Send dataport if asked, otherwise something went wrong.
if response == "DATAPORT?":
	print("Server wants the data port #")
	print("Sending: '" + str(dataPort) + "'")
	controlSocket.sendall(str(dataPort).encode())
elif response == "DATAPORT ERR":
	print("Error sending dataport to server; unable to process request.")
	controlSocket.close()
	quit()
elif response == "Invalid command":
	print("Invalid command sent to server; unable to process request.")
	controlSocket.close()
	quit()


# Command was valid - listen for the data connection from server

dataSocket = socket(AF_INET, SOCK_STREAM)
dataSocket.bind(('', dataPort))
dataSocket.listen(1)
dataConnection, addr = dataSocket.accept()

# If we sent the list command, listen for and print strings until the server sends
# the string "FTCLIENT END DIR LIST"
if command == "-l":
	print("Receiving directory structure from " + serverHost + ":" + str(dataPort))
	isMoreData = True
	ack = "ACK"

	# Make sure file doesn't exist locally, give it a new name if it does
	originalFN = fileName
	updated = False
	i = 0
	while (os.path.isfile(fileName)):
		i = i + 1
		fileName = originalFN + str(i)
		updated = True
	if updated == True:
		printf("Filename \"" + originalFN + "\" exists. Updating copied file name to " + fileName)
	
	# Read data from connection Q (Data connection) until no more lines
	while isMoreData:
		response = dataConnection.recv(1024)
		response = response.decode()
		if "FTSERVBYE" in response:
			print("DEBUG STATEMENT: Server done sending directory listing.")
			isMoreData = False
		else:
			print(response)
			dataConnection.sendall(ack.encode()) # This is a trick I used to force each row to be sent separately
	# Clean up
	dataConnection.close() # ftclient closes connection P
	dataSocket.close()
	controlSocket.close() # Backup in case ftserv doesn't close connection Q
	quit()


# If we sent the get command, send filename and see if file is found before getting
elif command == "-g":
	# Send the filename to server to validate it exists
	controlSocket.sendall(str(fileName).encode())
	response = controlSocket.recv(1024)
	response = response.decode()
	ack = "ACK"

	if response == ack:
		isMoreData = True
		while isMoreData:
			response = dataConnection.recv(1024)
			response = response.decode()
			if "FTSERVBYE" in response:
				print("File transfer complete.")
				isMoreData = False
			else:
				print(response)
				dataConnection.sendall(ack.encode()) # This is a trick I used to force each row to be sent separately
	elif response == "NEG":
		print(serverHost + ":" + str(serverPort) + " says FILE NOT FOUND")

	# Clean up
	dataConnection.close() # ftclient closes connection P
	dataSocket.close()
	controlSocket.close() # Backup in case ftserv doesn't close connection Q
	quit()

# Our protocol will wait for the control connection on controlSocket to send a
# response that states the command is good and the filename (if -g command) is good
# If those are okay, we ack the response and start listening on dataPort
# The server will wait for our "ack" and then initiate a new data connection on dataPort

# Once data connection is established, we will receive the file contents and store them
# in a file called fileName (which was provided on the command line). If filename already
# exists, we can a) abort the entire thing or b) prompt user for a new filename? (Maybe extra credit here?)

