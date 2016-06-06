#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include "Practical.h"

int main(int argc, char *argv[]) {

  if (argc!=4) // Test for correct number of arguments
    DieWithUserMessage("Parameter(s)","<Server Address/Name> <File Name> <Server Port/Service>");

  char *server = argv[1];     // First arg: server address/name
  char *fileName = argv[2]; // Second arg: File Name

  size_t fileNameLen = strlen(fileName);
  /*if (fileNameLen > MAXSTRINGLENGTH) // Check input length
    DieWithUserMessage(fileName, "string too long");*/

  // Third arg : server port/service
  char *servPort = argv[3];// Third arg: Port number

  // Tell the system what kind(s) of address info we want
  struct addrinfo addrCriteria;                   // Criteria for address match
  memset(&addrCriteria, 0, sizeof(addrCriteria)); // Zero out structure
  addrCriteria.ai_family = AF_UNSPEC;             // Any address family
  // For the following fields, a zero value means "don't care"
  addrCriteria.ai_socktype = SOCK_DGRAM;          // Only datagram sockets
  addrCriteria.ai_protocol = IPPROTO_UDP;         // Only UDP protocol

  // Get address(es)
  struct addrinfo *servAddr; // List of server addresses
  int rtnVal = getaddrinfo(server, servPort, &addrCriteria, &servAddr);
  if (rtnVal != 0)
    DieWithUserMessage("getaddrinfo() failed", gai_strerror(rtnVal));



  // Create a datagram/UDP socket
  int sock = socket(servAddr->ai_family, servAddr->ai_socktype,servAddr->ai_protocol); // Socket descriptor for client
  if (sock < 0)
    DieWithSystemMessage("socket() failed");



  // Send the string to the server
  
  ssize_t numBytes = sendto(sock, fileName, fileNameLen, 0,servAddr->ai_addr, servAddr->ai_addrlen);
  if (numBytes < 0)
    DieWithSystemMessage("sendto() failed");
  else if (numBytes != fileNameLen)
    DieWithUserMessage("sendto() error", "sent unexpected number of bytes");



  int datagram_no=0;
  struct ack_so ack;

  struct sockaddr_storage fromAddr; // Source address of server
  // Set length of from address structure (in-out parameter)
  socklen_t fromAddrLen = sizeof(fromAddr);
  // void *buffer = malloc(512); // I/O buffer
  char buffer[512];
  //memset(buffer,0,sizeof(buffer));

  while(1){
  	memset(buffer,0,sizeof(buffer));
    numBytes = recvfrom(sock, buffer, sizeof(buffer), 0,(struct sockaddr *) &fromAddr, &fromAddrLen);                   // Criteria for address match

   // Verify reception from expected source
    if (!SockAddrsEqual(servAddr->ai_addr, (struct sockaddr *) &fromAddr)){
      printf("Received packet from Unknown Source.\n");
      continue;   
    }
    if(numBytes==5){
      printf("Got port number.\n");
      memset(&addrCriteria, 0, sizeof(addrCriteria)); // Zero out structure
      addrCriteria.ai_family = AF_UNSPEC;             // Any address family
      // For the following fields, a zero value means "don't care"
      addrCriteria.ai_socktype = SOCK_DGRAM;          // Only datagram sockets
      addrCriteria.ai_protocol = IPPROTO_UDP;         // Only UDP protocol
      freeaddrinfo(servAddr);
      // Get address(es)
      int rtnVal = getaddrinfo(server, buffer, &addrCriteria, &servAddr);
      if (rtnVal != 0)
        DieWithUserMessage("getaddrinfo() failed", gai_strerror(rtnVal));
      break;
    }
  }
  

  while(1){

      // Receive a response

  memset(buffer,0,sizeof(buffer));
  numBytes = recvfrom(sock, buffer, sizeof(buffer), 0,(struct sockaddr *) &fromAddr, &fromAddrLen);

   // Verify reception from expected source
  if (!SockAddrsEqual(servAddr->ai_addr, (struct sockaddr *) &fromAddr)){
    printf("Received packet from Unknown Source.\n");
    continue;
  }
  if(numBytes == 0){
    DieWithUserMessage("Reached end of file transfer",fileName);
    close(sock);
    break;
  }

  if(numBytes == 20){
    DieWithUserMessage("Error : File not found - ",fileName);
    close(sock);
    break;
  }

  if (numBytes < 0)
    DieWithSystemMessage("recvfrom() failed");
  /*else if (numBytes != sizeof(buffer))
    DieWithUserMessage("recvfrom() error - client", "received unexpected number of bytes");*/


  
  printf("Printing datagram_no - %d\n%s\n\n",datagram_no,(char *)buffer);


  ack.num=datagram_no;
  ack.len=0;



  ssize_t numBytes = sendto(sock, &ack, sizeof(ack), 0,servAddr->ai_addr, servAddr->ai_addrlen);
  if (numBytes < 0)
    DieWithSystemMessage("sendto() failed for ack");
  else if (numBytes != sizeof(ack))
    DieWithUserMessage("sendto() error - ack", "sent unexpected number of bytes");

  datagram_no++;

  printf("ACK SENT %d\n",datagram_no-1 );

  
    //DieWithUserMessage("recvfrom()", "received a packet from unknown source");
  }

  freeaddrinfo(servAddr);

  close(sock);
  exit(0);
}
