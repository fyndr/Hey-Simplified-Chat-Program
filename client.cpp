#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "client.h"

void* receiving(void*);

using namespace std;

// no-arg constructor (unused)
Client::Client()
{

}

// main constructor
Client::Client(string host, int port, string nickname)
{
  this->host = host;
  this->port = port;
  this->nickname = nickname;
}

// destructor
Client::~Client()
{

}

int main(int argc, char* argv[])
{
  // argv[1] is host, argv[2] is port, argv[3] is nickname
  if (argc == 4)
  {
    // check nickname length
    int len = strlen(argv[3]);
    if (len >= 1 && len <= 24)
    {
      // instantiate Client instance
      Client* c = new Client(argv[1], atoi(argv[2]), argv[3]);
      // create socket and connect
      c->make_socket();
      c->connect_socket();
      // set up receive thread
      c->receive_socket();
      // set up chat loop
      c->chat();
      // closes the socket
      c->reset_socket();
      delete c;
    }
    else
    {
      // nickname out of range
      cout << "Nickname must be between 1 and 24 characters in length" << endl;
    }
  }
  else
  {
    // not enough/too many arguments
    cout << "Format is: client host port nick" << endl;
  }
  return 0;
}

// retrieves socket data
void Client::make_socket()
{
  this->socket_id = get_socket();
  this->server_addr = get_socket_address(this->port, this->host);
}

// connects to the socket
void Client::connect_socket()
{
  connect_socket(this->socket_id, this->server_addr);
  // wait for go-ahead
  char* go_ahead = receive_message(this->socket_id);
  // send nickname to server
  send_nickname();
  free(go_ahead);
}

// resets the socket
void Client::reset_socket()
{
  close(this->socket_id);
}

// connects the socket
void Client::connect_socket(int socket_id, struct sockaddr_in server_addr)
{
  int conn = connect(socket_id, (sockaddr*)&server_addr, sizeof(server_addr));
  if (conn < 0)
  {
    perror("ERROR: SOCKET COULD NOT CONNECT");
    exit(-3);
  }
  cout << "CONNECTION ESTABLISHED!" << endl;
}

// sends user nickname to the server
void Client::send_nickname()
{
  // prepare nickname string
  char* output = (char*)malloc(sizeof(char) * 56);
  strcpy(output, this->nickname.c_str());
  strcat(output, (char*)" has connected to the server.");
  strcat(output, (char*)"\0");
  // send nickname
  send_message(output, this->socket_id);
  free(output);
}

// allows the user to send messages
void Client::chat()
{
  char* output = (char*)malloc(sizeof(char) * 1024);
  string input;
  while (1)
  {
    // prefix all messages with nickname
    strcpy(output, this->nickname.c_str());
    strcat(output, (char*)": ");
    getline(cin, input);
    // enforce message length
    if (strlen(output) + input.length() <= 1024)
    {
      strcat(output, input.c_str());
      strcat(output, (char*)"\0");
      // send message
      send_message(output, this->socket_id);
    }
    else
    {
      // message length excessive
      cout << "Message is too long - try condensing your text" << endl;
    }
  }
  free(output);
}

// set up receiving thread
void Client::receive_socket()
{
  pthread_t t;
  pthread_create(&t, NULL, receiving, (void*)this);
}

// receive messages from other clients
void* receiving(void* ptr)
{
  Client* c = (Client*)ptr;
  while (1)
  {
    // receive message and print to console
    char* printput = c->receive_message(c->socket_id);
    cout << printput << endl;
    free(printput);
  }
}
