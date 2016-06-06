all:
	gcc UDPServer.c DieWithMessage.c AddressUtility.c -o server
	gcc UDPClient.c DieWithMessage.c AddressUtility.c -o client
clean:
	rm -rf server client

