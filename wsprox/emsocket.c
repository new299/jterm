#include <emscripten.h>
#include <stdio.h>
#include "emsocket.h"
#include <errno.h>

ssize_t recv(int sock, void *buffer, size_t length, int flags) {
  //printf("emscripen recv called\n");
  int ret = EM_ASM_INT({
    return jss_recv($0,$1,$2,$3);
  }, sock,buffer,length,flags);

  if(ret < 0) errno = -ret;

  //printf("emscripten recv buffer[0]: %d\n",((char *)buffer)[0]);

  //printf("emscripten recv returned: %d\n",ret);
  return ret;
}

ssize_t send(int sock, const void *buffer, size_t length, int flags) {
  //printf("emscripten send called\n");
  int ret = EM_ASM_INT({
    return jss_send($0,$1,$2,$3);
  }, sock,buffer,length,flags);
  
  if(ret < 0) errno = -ret;

  //printf("emscripten send returned: %d\n",ret);
  return ret;
}


int socket(int domain, int type, int protocol) {

  //printf("emscripten socket called\n");
  if(domain != AF_INET) return 0;
  if(type   != SOCK_STREAM) return 0;

  int ret = EM_ASM_INT({
    return jss_socket($0,$1,$2);
  }, domain,type,protocol);
  
  if(ret < 0) errno = -ret;

  return ret;
}

int connect(int sockfd, const struct sockaddr *addr,
                   socklen_t addrlen) {

  //printf("emscripten connect called\n");
  int ret = EM_ASM_INT({
    return jss_connect($0,$1,$2);
  }, sockfd,addr,addrlen);
  
  if(ret < 0) errno = -ret;

  return ret;
}
