/*
  tcpserver.c - A simple TCP echo server
 * usage: tcpserver <port>
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include<time.h>
#include<ctype.h>
#define BUFSIZE 1024


/*
 * error - wrapper for perror
 */
void error(char *msg) {
  perror(msg);
  exit(1);
}

int main(int argc, char **argv) {
  int parentfd; /* parent socket */
  int childfd; /* child socket */
   int portno; /* port to listen on */
  int clientlen; /* byte size of client's address */
  struct sockaddr_in serveraddr; /* server's addr */
  struct sockaddr_in clientaddr; /* client addr */
  struct hostent *hostp; /* client host info */
  char buf[BUFSIZE]; /* message buffer */
  char *hostaddrp; /* dotted decimal host addr string */
  int optval; /* flag value for setsockopt */
  int n; /* message byte size */
  time_t rawtime;
  struct tm * timeinfo;
  int sockfd;
  char buf1[BUFSIZE];


  /*
   * check command line arguments
   */
  if (argc != 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }
  portno = atoi(argv[1]);

  /*
   * socket: create the parent socket
   */
  parentfd = socket(AF_INET, SOCK_STREAM, 0);
  if (parentfd < 0)
    error("ERROR opening socket");

  /* setsockopt: Handy debugging trick that lets
   * us rerun the server immediately after we kill it;
   * otherwise we have to wait about 20 secs.

  * Eliminates "ERROR on binding: Address already in use" error.
   */
  optval = 1;
  setsockopt(parentfd, SOL_SOCKET, SO_REUSEADDR,
             (const void *)&optval , sizeof(int));

  /*
   * build the server's Internet address
   */
  bzero((char *) &serveraddr, sizeof(serveraddr));

  /* this is an Internet address */
  serveraddr.sin_family = AF_INET;

  /* let the system figure out our IP address */
  serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

  /* this is the port we will listen on */
  serveraddr.sin_port = htons((unsigned short)portno);

  /*
   * bind: associate the parent socket with a port
   */
  if (bind(parentfd, (struct sockaddr *) &serveraddr,
           sizeof(serveraddr)) < 0)
    error("ERROR on binding");

  /*
   * listen: make this socket ready to accept connection requests
        */
  printf("listen...before\n");
  if (listen(parentfd, 5) < 0) /* allow 5 requests to queue up */
    error("ERROR on listen");
  printf("listen...after\n");

  /*
   * main loop: wait for a connection request, echo input line,
   * then close connection.
   */
  clientlen = sizeof(clientaddr);
  while(1)
{
    /*
     * accept: wait for a connection request
     */
    printf("accept... \n");
    childfd = accept(parentfd, (struct sockaddr *) &clientaddr, &clientlen);
    if (childfd < 0)
      error("ERROR on accept");

    /*
     * gethostbyaddr: determine who sent the message
     */
    hostp = gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr,
                          sizeof(clientaddr.sin_addr.s_addr), AF_INET);

    if (hostp == NULL)
      error("ERROR on gethostbyaddr");
   hostaddrp = inet_ntoa(clientaddr.sin_addr);
    if (hostaddrp == NULL)
      error("ERROR on inet_ntoa\n");

    printf("server established connection with %s (%s)\n",
           hostp->h_name, hostaddrp);
    printf("port number: %d\n",
           clientaddr.sin_port);

   while(1)
   {
     bzero(buf, BUFSIZE);
     n = read(childfd, buf, BUFSIZE);
     if (n<0)
	error("ERROR reading from socket");
     printf("server received %d bytes: %s",n,buf);

     strcpy(buf1,buf); 
    for(int i=0;buf1[i];i++)
     {
     buf1[i] = toupper(buf1[i]);
     }

    buf1[strlen(buf1)-1] = '\0';

    if(strcmp(buf1,"TIME") ==0)
    {
	time (&rawtime);
	timeinfo = localtime(&rawtime);
	strcpy(buf,asctime(timeinfo));
	//printf("%s",buf);
      break;
    }

     n = write(childfd, buf,strlen(buf));
     if(n < 0 )
         error("ERROR writing to socket");
 
   }

     n = write(childfd, buf,strlen(buf));
     if(n < 0 )
         error("ERROR writing to socket");
    close(childfd);
  }
}
