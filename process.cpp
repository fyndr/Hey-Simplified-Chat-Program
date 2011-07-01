#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "process.h"

using namespace std;

// sends a message through the socket
void Process::send_message(char* msg, int sock)
{
  int w = send(sock, msg, strlen(msg), 0);
  if (w < 0)
  {
    perror("ERROR: SOCKET WRITE FAILURE");
    exit(-4);
  }
}

// receives a message
char* Process::receive_message(int sock)
{
  char* buff = (char*)malloc(sizeof(char) * 4096);
  int i;
  for(i = 0; i < 4096; i++)
  {
   buff[i] = '\0';
  }
  int r = recv(sock, buff, 4096, 0);
  if (r < 0)
  {
    perror("ERROR: SOCKET READ FAILURE");
    exit(-5);
  }
  return buff;
}

// retrieves the socket file descriptor
int Process::get_socket()
{
  int socket_id = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_id < 0)
  {
    perror("ERROR: SOCKET CANNOT BE ESTABLISHED");
    exit(-1);
  }
  return socket_id;
}

// obtains the socket address
struct sockaddr_in Process::get_socket_address(int port, string host)
{
  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  struct hostent* hostname;
  hostname = gethostbyname((const char*)host.c_str());
  if (hostname == NULL)
  {
    perror("ERROR: INVALID HOST NAME");
    exit(-2);
  }
  server_addr.sin_addr = *((struct in_addr*)hostname->h_addr);
  return server_addr;
}
