#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/time.h>
#include <signal.h>
#include "Practical.h"

int port_number, client_cnt=0;

int main(int argc, char *argv[]) {

  FILE *fp;
  struct ack_so ack;

  if (argc != 2) // Test for correct number of arguments
    DieWithUserMessage("Parameter(s)", "<Server Port/Service>");

  char *service = argv[1]; // First arg:  local port/service
  port_number = atoi(service);
  char service_buffer[5];
  strcpy(service_buffer,service);  

  printf("---------------------------------\n");
  printf("File Service at port :  %d .\n",port_number );
  printf("---------------------------------\n");


    // Construct the server address structure
    struct addrinfo addrCriteria;                   // Criteria for address
    memset(&addrCriteria, 0, sizeof(addrCriteria)); // Zero out structure
    addrCriteria.ai_family = AF_UNSPEC;             // Any address family
    addrCriteria.ai_flags = AI_PASSIVE;             // Accept on any address/port
    addrCriteria.ai_socktype = SOCK_DGRAM;          // Only datagram socket
    addrCriteria.ai_protocol = IPPROTO_UDP;         // Only UDP socket

    struct addrinfo *servAddr; // List of server addresses
    int rtnVal = getaddrinfo(NULL, service_buffer, &addrCriteria, &servAddr);
    if (rtnVal != 0)
      DieWithUserMessage("getaddrinfo() failed", gai_strerror(rtnVal));

    // Create socket for incoming connections
    int sock = socket(servAddr->ai_family, servAddr->ai_socktype,
        servAddr->ai_protocol);
    if (sock < 0)
      DieWithSystemMessage("socket() failed");


    // Bind to the local address
    if (bind(sock, servAddr->ai_addr, servAddr->ai_addrlen) < 0)
      DieWithSystemMessage("bind() failed");

    /*// Free address list allocated by getaddrinfo()
    freeaddrinfo(servAddr);
    */for (;;) {

      port_number++;
      snprintf(service_buffer,5,"%d",port_number);


    

      // Run forever
      struct sockaddr_storage clntAddr; // Client address
      // Set Length of client address structure (in-out parameter)
      socklen_t clntAddrLen = sizeof(clntAddr);

      // Block until receive message from a client
      char buffer[MAXSTRINGLENGTH]; // I/O buffer

      memset(buffer,0,sizeof(buffer));

      // Size of received message
      ssize_t numBytesRcvd = recvfrom(sock, buffer, MAXSTRINGLENGTH, 0,(struct sockaddr *) &clntAddr, &clntAddrLen);
      
      if (numBytesRcvd < 0)
        DieWithSystemMessage("recvfrom() failed");

      fputs("Handling client ", stdout);
      PrintSocketAddress((struct sockaddr *) &clntAddr, stdout);
      fputc('\n', stdout);    
      
      //char service_buffer[5];
      ssize_t numBytesSent = sendto(sock, service_buffer, sizeof(service_buffer), 0,(struct sockaddr *) &clntAddr, sizeof(clntAddr));
      if (numBytesSent < 0)
        DieWithSystemMessage("sendto() failed)");
      else if (numBytesSent != sizeof(service_buffer))
        DieWithUserMessage("sendto()", "sent unexpected number of bytes");

      

      // create new child for every inbound request
      int ret = fork();
      if (ret == 0)
      {
        memset(&addrCriteria, 0, sizeof(addrCriteria)); // Zero out structure
        addrCriteria.ai_family = AF_UNSPEC;             // Any address family
        addrCriteria.ai_flags = AI_PASSIVE;             // Accept on any address/port
        addrCriteria.ai_socktype = SOCK_DGRAM;          // Only datagram socket
        addrCriteria.ai_protocol = IPPROTO_UDP;         // Only UDP socket

        freeaddrinfo(servAddr);
        rtnVal = getaddrinfo(NULL, service_buffer, &addrCriteria, &servAddr);
        if (rtnVal != 0)
          DieWithUserMessage("getaddrinfo() failed", gai_strerror(rtnVal));

        sock = socket(servAddr->ai_family, servAddr->ai_socktype,servAddr->ai_protocol);
        if (sock < 0)
          DieWithSystemMessage("socket() failed");

        // Bind to the local address
        if (bind(sock, servAddr->ai_addr, servAddr->ai_addrlen) < 0)
          DieWithSystemMessage("bind() failed");

        setsockopt(sock,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(struct timeval));
          
          int i;
          for (i = 0; i < 10; ++i)
          {
            //printf("FILENAME%s\n",buffer);
            if(!strcmp(filenames[i], buffer)){
              
              if (fp = fopen(filenames[i], "rb"))
              {

                char readbuffer[512];

                int datagram_no=0, blocksizecnt=0, timeoutcnt=0;;

                memset(readbuffer, 0, 512);
                while((blocksizecnt = fread(readbuffer, 1, 511, fp)) > 0){

                  // goto label for timeout construct
                  TIMEOUT_TRY:
                  //readbuffer[strlen(readbuffer)] = '\0';
                  //printf("Sending %s %d",readbuffer, strlen(readbuffer));
                  printf("Block Size %d Sending Datagram : %d\n",blocksizecnt, datagram_no);


                  ssize_t numBytesSent = sendto(sock, readbuffer, sizeof(readbuffer), 0,
                      (struct sockaddr *) &clntAddr, sizeof(clntAddr));
                  if (numBytesSent < 0)
                    DieWithSystemMessage("sendto() failed)");
                  else if (numBytesSent != sizeof(readbuffer))
                    DieWithUserMessage("sendto()", "sent unexpected number of bytes");

                  // timeout to check for concurrency and timeout trials
                  sleep(2);


                  ssize_t numBytesRcvd = recvfrom(sock, &ack, sizeof(ack), 0,(struct sockaddr *) &clntAddr, &clntAddrLen);


                  if(numBytesRcvd < 0){
                    printf("Socket timeout | Trying again.\n");
                    timeoutcnt++;
                    if(timeoutcnt!=4)
                      goto TIMEOUT_TRY;
                    else DieWithSystemMessage("Socket timeout exception | Exiting now");
                  }
                    

                  
                  if(ack.num != datagram_no){
                    printf("Ack no. %d not received for file - %s\n",datagram_no, filenames[i]);
                    break;

                  }

                  if(blocksizecnt!=511){

                    void *nullbuffer ;
                    ssize_t numBytesSent = sendto(sock, nullbuffer, 0, 0,(struct sockaddr *) &clntAddr, sizeof(clntAddr));
                    break;
                    
                  }


                  datagram_no++;     
                  memset(readbuffer, 0, 512);

                }

                fclose(fp);
                break;


              }
            }

          }

          if(i==10){
            char errordtgm[20];
            snprintf(errordtgm,20,"%s","Error: File not found");
            ssize_t numBytesSent = sendto(sock, errordtgm, sizeof(errordtgm), 0,
                      (struct sockaddr *) &clntAddr, sizeof(clntAddr));

          }
          printf("Done with this client\n");
          exit(0);

        
      }

  }

}




