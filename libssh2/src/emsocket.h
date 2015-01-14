#ifndef EMSCRIPTEN_SOCKETS_H
#define EMSCRIPTEN_SOCKETS_H

#include <emscripten.h>
#include <stdio.h>
#include <sys/socket.h>

//typedef int socklen_t;

//struct in_addr {
//    unsigned long s_addr;  // load with inet_aton()
//};

//struct sockaddr {
//    short            sin_family;   // e.g. AF_INET
//    unsigned short   sin_port;     // e.g. htons(3490)
//    struct in_addr   sin_addr;     // see struct in_addr, below
//    char             sin_zero[8];  // zero this if you want to
//};

ssize_t recv(int sock, void *buffer, size_t length, int flags);

ssize_t send(int sock, const void *buffer, size_t length, int flags);

int socket(int domain, int type, int protocol);

int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

#endif
