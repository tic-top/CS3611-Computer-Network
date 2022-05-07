/*
   tcp client
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <netdb.h>
#include <iostream>
#define PORT "3490"     // port we're listening on
#define MAXDATASIZE 100 // max number of bytes we can get at once

void *get_in_addr(struct sockaddr *sa);
int GetLocalIP(char *ip);
int server();
int client(char argv[]);
int istherserver();

int main()
{
    istherserver();
}

int istherserver()
{
    char argv[] = "10.0.0.0";
    for (int i = 1; i <= 4; i++)
    {
        argv[7] = i + '0';
        int sockfd, numbytes;
        char buf[MAXDATASIZE];
        struct addrinfo hints, *servinfo, *p;
        int rv;
        char s[INET6_ADDRSTRLEN];

        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;

        if ((rv = getaddrinfo(argv, PORT, &hints, &servinfo)) != 0)
        {
            fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
            return 1;
        }
        for (p = servinfo; p != NULL; p = p->ai_next)
        {
            if ((sockfd = socket(p->ai_family, p->ai_socktype,
                                 p->ai_protocol)) == -1)
            {
                // perror("client: socket");
                continue;
            }

            if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1)
            {
                close(sockfd);
                // perror("client: connect");
                continue;
            }
            return client(argv);
        }

        freeaddrinfo(servinfo);
        close(sockfd);
    }
    // std::cout<<"server\n";
    return server();
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}
int GetLocalIP(char *ip)
{
    int MAXINTERFACES = 16;
    int fd, intrface = 0;
    struct ifreq buf[MAXINTERFACES];
    struct ifconf ifc;
    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) >= 0)
    {
        ifc.ifc_len = sizeof buf;
        ifc.ifc_buf = (caddr_t)buf;
        if (!ioctl(fd, SIOCGIFCONF, (char *)&ifc))
        {
            intrface = ifc.ifc_len / sizeof(struct ifreq);
            while (intrface-- > 0)
            {
                if (!(ioctl(fd, SIOCGIFADDR, (char *)&buf[intrface])))
                {
                    sprintf(ip, "%s", inet_ntoa(((struct sockaddr_in *)(&buf[intrface].ifr_addr))->sin_addr));
                    break;
                }
            }
        }
        close(fd);
    }
    return 0;
}
int client(char argv[])
{
    /*
      keep doing two things:
      1. listen to server(argv)
      2. send message to server
    */
    int sockfd, numbytes;
    char buf[MAXDATASIZE];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(argv, PORT, &hints, &servinfo)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and connect to the first we can
    for (p = servinfo; p != NULL; p = p->ai_next)
    {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1)
        {
            perror("client: socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(sockfd);
            perror("client: connect");
            continue;
        }

        break;
    }

    if (p == NULL)
    {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }

    freeaddrinfo(servinfo); // all done with this structure

    // now we get sockfd(the socket between the client and the server)

    fd_set master;    // master file descriptor list
    fd_set read_fds;  // temp file descriptor list for select()
    int fdmax;        // maximum file descriptor number
    FD_ZERO(&master); // clear the master and temp sets
    FD_ZERO(&read_fds);

    FD_SET(0, &master);
    FD_SET(sockfd, &master);
    fdmax = sockfd;

    for (;;)
    {
        read_fds = master; // copy it
        if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) == -1)
        {
            perror("select");
            exit(4);
        }
        // receive data from the server
        if (FD_ISSET(sockfd, &read_fds))
        {
            // std::cout<<"cccccccccccccccccccccccccccoming!\n";
            if ((numbytes = recv(sockfd, buf, MAXDATASIZE - 1, 0)) == -1)
            {
                perror("recv");
                exit(1);
            }

            printf("%s \n", buf);
        }
        // receive data from keyboard
        if (FD_ISSET(0, &read_fds))

        {
            char info[256];
            std::cin.getline(info, 255);
            if (send(sockfd, info, strlen(info), 0) == -1)
            {
                perror("send");
            }
        }
    }
    close(sockfd);
    return 0;
    std::cout << "222\n";
}

int server()
{
    char IP[16] = {0};
    GetLocalIP(IP);
    int map[100];
    map[0] = IP[7] - '0';
    fd_set master;   // master file descriptor list
    fd_set read_fds; // temp file descriptor list for select()
    int fdmax;       // maximum file descriptor number

    int listener;                       // listening socket descriptor
    int newfd;                          // newly accept()ed socket descriptor
    struct sockaddr_storage remoteaddr; // client address
    socklen_t addrlen;

    char buf[256]; // buffer for client data
    int nbytes;

    char remoteIP[INET6_ADDRSTRLEN];

    int yes = 1; // for setsockopt() SO_REUSEADDR, below
    int i, j, rv;

    struct addrinfo hints, *ai, *p;

    FD_ZERO(&master); // clear the master and temp sets
    FD_ZERO(&read_fds);

    // get us a socket and bind it
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0)
    {
        fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
        exit(1);
    }

    for (p = ai; p != NULL; p = p->ai_next)
    {
        listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listener < 0)
        {
            continue;
        }

        // lose the pesky "address already in use" error message
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        if (bind(listener, p->ai_addr, p->ai_addrlen) < 0)
        {
            close(listener);
            continue;
        }

        break;
    }

    // if we got here, it means we didn't get bound
    if (p == NULL)
    {
        fprintf(stderr, "selectserver: failed to bind\n");
        exit(2);
    }

    freeaddrinfo(ai); // all done with this

    // listen
    if (listen(listener, 10) == -1)
    {
        perror("listen");
        exit(3);
    }

    // add the listener to the master set
    FD_SET(0, &master);
    FD_SET(listener, &master);

    // keep track of the biggest file descriptor
    fdmax = listener; // so far, it's this one

    // main loop
    for (;;)
    {
        read_fds = master; // copy it
        if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) == -1)
        {
            perror("select");
            exit(4);
        }

        // run through the existing connections looking for data to read
        for (i = 0; i <= fdmax; i++)
        {
            if (FD_ISSET(i, &read_fds))
            {
                // we got one!!
                if (i == listener)
                {
                    // handle new connections
                    addrlen = sizeof remoteaddr;
                    newfd = accept(listener,
                                   (struct sockaddr *)&remoteaddr,
                                   &addrlen);

                    if (newfd == -1)
                    {
                        perror("accept");
                    }
                    else
                    {
                        FD_SET(newfd, &master); // add to master set
                        if (newfd > fdmax)
                        { // keep track of the max
                            fdmax = newfd;
                        }
                        // printf("selectserver: new connection from %s on "
                        //     "socket %d\n",
                        inet_ntop(remoteaddr.ss_family,
                                  get_in_addr((struct sockaddr *)&remoteaddr),
                                  remoteIP, INET6_ADDRSTRLEN);
                        map[newfd] = remoteIP[7] - '0';
                    }
                }
                else
                {
                    // handle data from a client
                    if (i > 0 && (nbytes = recv(i, buf, sizeof buf, 0)) <= 0)
                    {
                        // got error or connection closed by client
                        if (nbytes == 0)
                        {
                            // connection closed
                            printf("selectserver: socket %d hung up\n", i);
                        }
                        else
                        {
                            perror("recv");
                        }
                        close(i);           // bye!
                        FD_CLR(i, &master); // remove from master set
                    }
                    else
                    {
                        // we got some data from a client
                        if (i == 0)
                        {
                            std::cin.getline(buf, 255);
                            // std::cout<<"i==0 "<<buf<<"\n";
                            nbytes = strlen(buf);
                        }
                        int destination = buf[4] - '0';
                        char message[256];
                        char part[30];
                        strcpy(part, " From h");
                        for (int pos = 0; pos < nbytes - 6 + 8; pos++)
                            if (pos + 6 < nbytes)
                                message[pos] = buf[pos + 6];
                            else if (pos + 1 < nbytes - 6 + 8)
                                message[pos] = part[pos + 6 - nbytes];
                            else
                                message[pos] = map[i] + '0';
                        message[nbytes - 6 + 8] = '\0';
                        if (destination == map[0])
                            std::cout << message << '\n';
                        else
                        {
                            for (j = 0; j <= fdmax; j++)
                            {
                                if (map[j] == destination && FD_ISSET(j, &master))
                                {
                                    // std::cout<<message[nbytes+1]<<'\n';
                                    if (send(j, message, nbytes - 6 + 8 + 1, 0) == -1)
                                    {
                                        perror("send");
                                    }
                                }
                            }
                        }
                    }
                } // END handle data from client
            }     // END got new incoming connection
        }         // END looping through file descriptors
    }             // END for(;;)--and you thought it would never end!

    return 0;
}
