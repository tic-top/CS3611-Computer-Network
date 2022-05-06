/*
udp
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <iostream>
#include <signal.h> 
#include <sys/ioctl.h>   
#include <linux/if.h>  
#define SERVERPORT 4930 
#define MYPORT "4930" 
#define MAXBUFLEN 100

int GetLocalIP(char *ip)
{
	int  MAXINTERFACES = 16;
	int fd, intrface = 0;
	struct ifreq buf[MAXINTERFACES];   
	struct ifconf ifc; 
	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) >= 0) 
	{
		ifc.ifc_len = sizeof buf;
		ifc.ifc_buf = (caddr_t)buf;
		if (!ioctl(fd, SIOCGIFCONF, (char *)&ifc))  
		{
			intrface = ifc.ifc_len / sizeof (struct ifreq);
			while (intrface-- > 0)
			{
				if (!(ioctl(fd, SIOCGIFADDR, (char *)&buf[intrface])))
				{
					sprintf(ip, "%s", inet_ntoa(((struct sockaddr_in*)(&buf[intrface].ifr_addr))->sin_addr));
					break;
				}
 
			}
		}
		close(fd);
	}
	return 0;
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in*)sa)->sin_addr);
  }

  return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main()
{
  int sockfd;
  struct addrinfo hints, *servinfo, *p;
  struct sockaddr_in their_addr; 
  struct sockaddr_storage recv_addr;
  socklen_t addr_len;
  struct hostent *he;
  int rv;
  char buf[MAXBUFLEN];
  char s[INET6_ADDRSTRLEN];
  int numbytes;
  int broadcast = 1;

  if ((he=gethostbyname("10.0.0.0")) == NULL) { 
    perror("gethostbyname");
    exit(1);
  }

  memset(&hints, 0, sizeof hints);

  hints.ai_family = AF_UNSPEC; 
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_flags = AI_PASSIVE; 

  if ((rv = getaddrinfo(NULL, MYPORT, &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return 1;
  }

  for(p = servinfo; p != NULL; p = p->ai_next) {

    if ((sockfd = socket(p->ai_family, p->ai_socktype,
      p->ai_protocol)) == -1) {
      perror("listener: socket");
      continue;
    }

    if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      close(sockfd);
      perror("listener: bind");
      continue;
    }

    break;
  }

  if (p == NULL) {
    fprintf(stderr, "listener: failed to bind socket\n");
    return 2;
  }

  freeaddrinfo(servinfo);

  if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast,
    sizeof broadcast) == -1) {
    perror("setsockopt (SO_BROADCAST)");
    exit(1);
  }

  their_addr.sin_family = AF_INET; // host byte order
  their_addr.sin_port = htons(SERVERPORT); // short, network byte order
  their_addr.sin_addr = *((struct in_addr *)he->h_addr);
  memset(their_addr.sin_zero, '\0', sizeof their_addr.sin_zero);
  
  
  fd_set master;
  fd_set read_fds;
  FD_ZERO(&master);
  FD_ZERO(&read_fds);
  FD_SET(0,&master);
  FD_SET(sockfd,&master);
  int fdmax=sockfd;

for(;;)
  { 
    read_fds=master;
    int ret=select(sockfd+1,&read_fds,NULL,NULL,NULL);
    if(FD_ISSET(0,&read_fds))
    {
	char* message=new char[100];
  	std::cin.getline(message,99);
  	int bytes_sent;
	if ((numbytes = sendto(sockfd, message, strlen(message), 0,
    	(struct sockaddr *)&their_addr, sizeof their_addr)) == -1) 
	{
    	perror("talker: sendto");
    	exit(1);
  	}
        delete []message;
  
    }
    else if(FD_ISSET(sockfd,&read_fds))
    {
	addr_len=sizeof recv_addr;
  	if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0, 
        (struct sockaddr *)&recv_addr, &addr_len)) == -1) {

    	perror("recvfrom");
    	exit(1);
     	}

  	inet_ntop(recv_addr.ss_family,

  	get_in_addr((struct sockaddr *)&recv_addr), s, sizeof s);

  	buf[numbytes] = '\0';
        char IP[16] = {0};
  	GetLocalIP(IP);
	if(s[7]!=IP[7]) printf("%s \n", buf);
     	}

  }
  close(sockfd);
  return 0;
}
