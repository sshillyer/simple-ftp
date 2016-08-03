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
	
	while isMoreData:
		response = dataConnection.recv(1024)
		response = response.decode()
		if "FTSERVBYE" in response:
			print("DEBUG STATEMENT: Server done sending directory listing.")
			isMoreData = False
		else:
			print(response)
			dataConnection.sendall(ack.encode()) # This is a trick I used to force each row to be sent separately
	dataConnection.close() # ftclient closes connection P
	dataSocket.close()
	controlSocket.close() # Backup in case ftserv doesn't close connection Q
	quit()

elfif command == "-g":
	# Send the filename to server to validate it exists
	controlSocket.sendall(str(fileName).encode())

# Our protocol will wait for the control connection on controlSocket to send a
# response that states the command is good and the filename (if -g command) is good
# If those are okay, we ack the response and start listening on dataPort
# The server will wait for our "ack" and then initiate a new data connection on dataPort

# Once data connection is established, we will receive the file contents and store them
# in a file called fileName (which was provided on the command line). If filename already
# exists, we can a) abort the entire thing or b) prompt user for a new filename? (Maybe extra credit here?)

