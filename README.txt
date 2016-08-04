--------------------------------------------------------------------------------
Program Author:  Shawn S Hillyer
Date:            7/15/2016

Description:     This README.txt file explains how to compile, execute, and 
                 control the chatclient and chatserv.py programs.
--------------------------------------------------------------------------------

TLDR (Too-Long-Didn't Read):
1) Execute 'make' at command line prompt on Linux to build ftserv
2) Execute 'ftserv serverport' at command line (where serverport is a valid port
number on which the server will listen for incoming connections)
3) Execute 'python3 ftclient.py <SERVER_HOST> <SERVER_PORT> [-l | -g] <FILENAME> <DATA_PORT>' where serverhost is an ip address/hostname, server port is the SAME server port used in step 2, -l is the list comand, and -g is the get command, FILENAME is the filename desired, and DATA_PORT is the port to use.
  
  EXAMPLES:
    python3 ftclient.py flip1.engr.oregonstate.edu 12345 -l 12346
    # Note that filename is excluded
    python3 fitclient.py flip2.engr.oregonstate.edu 12345 -g README.txt 12346
    # Note that filename is required

4) IF the -l command is sent, then ftclient.py execution will receive the dir
   reading from the server's PWD.

   ELSE if the -g command is sent, ftserv will attempt to open FILENAME.
     If FILENAME does not exist, an error message is sent to ftclient.py
     If FILENAME does exist, it is read and each line is sent to ftclient.py
     ftclient.py will store FILENAME's contents into a local file called FILENAME,
      but if FILENAME already exists, it will PREPEND the filename with a prefix.
      It will iterate prefixes starting with 'copy_1' and incrementing until a valid, un-used filename is encountered.

5) ftclient.py will execute and then close relevant sockets and quit.
   ftserv will continue to listen on the designated port until it is sent the
   SIGINT signal (CTRL+C on a linux machine).


NOTE: This program is designed to only send a plaintext file and does not send
it as a byte-stream.