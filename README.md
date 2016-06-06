# remoteTextFileTransfer
A program in C to cater to text file requests by clients on a LAN

A few text files are present on the server. Clients request the server with "FILE:filename". If found, the server spawns a new **child process** and that process sends the data to the client over **UDP**. Reliability is ensured at the application level using a few control messages.

All the files are present in the same folder namely - {file1,file2,file3 .... file10}.
To run the server, run command - ./server <port number> 
eg :- ./server 8080 (for port 8080).

To query the server , run client code - ./client <server address> <filename> <port number>
eg:- ./client 127.0.0.1 file3 8080.

All the clients connecting to port 8080 are redirected to ports starting from 8080 to 8081,8082,8083 .

Server everytime tries to reconnect 3 times after a timeout of 2s if it fails to acknowledge from the client.
