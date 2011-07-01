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
#include "server.h"

using namespace std;

// no-arg constructor (unused)
Server::Server()
{

}

// main constructor
Server::Server(int port)
{
  this->port = port;
  FD_ZERO(&(this->master));
  FD_ZERO(&(this->temp_fds));
  this->max_fd = 0;
}

// destructor
Server::~Server()
{

}

int main(int argc, char* argv[])
{
  // argv[1] is listening port
  if (argc == 2)
  {
    // instantiate Server instance
    Server* serv = new Server(atoi(argv[1]));
    // sets up the server
    serv->setup();
    // wait for a connection and relay messages accordingly
    serv->process_messages();
    // closes the socket
    serv->close_socket();
    delete serv;
  }
  else
  {
    // not enough/too many arguments
    cout << "Format is: server port" << endl;
  }
  return 0;
}

// sets the socket option to allow/disallow reuse of addresses
void Server::reuse_addresses(int* trigger)
{
  int reuse = setsockopt(this->socket_id, SOL_SOCKET, SO_REUSEADDR, trigger, sizeof(int));
  if (reuse < 0)
  {
    perror("ERROR: SOCKET OPERATION FAILED");
    exit(-6);
  }
}

// binds the server socket
void Server::bind_socket(struct sockaddr_in server_addr)
{
  int try_bind = bind(this->socket_id, (struct sockaddr*)&server_addr, sizeof(struct sockaddr));
  if (try_bind < 0)
  {
    perror("ERROR: SOCKET COULD NOT BIND");
    exit(-3);
  }
}

// listens for connections to the socket
void Server::listen_socket(int num_conns)
{
  int hear = listen(this->socket_id, num_conns);
  if (hear < 0)
  {
    perror("ERROR: INVALID LISTEN");
    exit(-4);
  }
  // add the socket to the master list and set the maximum file descriptor
  FD_SET(this->socket_id, &master);
  this->max_fd = this->socket_id;
}

// accepts client connections
int Server::accept_socket()
{
  struct sockaddr_in client_addr;
  int client_len = sizeof(client_addr);
  int client_id = accept(this->socket_id, (struct sockaddr*)&client_addr, (socklen_t*)&client_len);
  if (client_id < 0)
  {
    perror("ERROR: INVALID CLIENT REQUEST");
    exit(-5);
  }
  return client_id;
}

// sets up the server
void Server::setup()
{
  // specify maximum number of connections
  int num_connections = 24;
  // allow address reuse
  int reuse = 1;
  // get the id for the socket
  this->socket_id = get_socket();
  reuse_addresses(&reuse);
  // retrieve host information
  char host[255];
  gethostname(host, 255);
  struct sockaddr_in server_addr = get_socket_address(this->port, host);
  // bind to and listen on socket
  bind_socket(server_addr);
  listen_socket(num_connections);
}

// closes the socket
void Server::close_socket()
{
  close(this->socket_id);
}

// accepts connections and relays messages across clients
void Server::process_messages()
{
  while (1)
  {
    int i;
    this->temp_fds = this->master;
    // wait on incoming messages
    int try_read = select(this->max_fd + 1, &(this->temp_fds), NULL, NULL, NULL);
    if (try_read < 0)
    {
      perror("ERROR: CONNECTION REFUSED");
      exit(-7);
    }
    // for all file descriptors in range
    for (i = 0; i < this->max_fd + 1; i++)
    {
      // if read on this client is valid
      if (FD_ISSET(i, &(this->temp_fds)))
      {
        // if the client is already connected
        if (i != this->socket_id)
        {
          // receive a message
          char* msg = receive_message(i);
          // client disconnects
          if (strlen(msg) == 0)
          {
            // close connection to client and remove from master list
            close(i);
            FD_CLR(i, &(this->master));
            // prepare and send out client disconnect string
            char* output = (char*)malloc(sizeof(char) * 61);
            strcpy(output, this->nicks[i].c_str());
            strcat(output, (char*)" has disconnected from the server.");
            strcat(output, (char*)"\0");
            send_all(output);
            free(output);
          }
          else
          {
            // pull nickname from user string if applicable
            string message = msg;
            if (message.find(": ") == string::npos)
            {
              this->nicks[i] = message.substr(0, message.find(" "));
            }
            // relay message
            send_all(msg);
          }
          free(msg);
        }
        else
        {
          // accept client on socket and add to master list
          int cl_id = this->accept_socket();
          FD_SET(cl_id, &(this->master));
          // adjust max file descriptor as necessary
          if (cl_id > this->max_fd)
          {
            this->max_fd = cl_id;
          }
          // send acknowledgement string
          char* msg = (char*)"connected";
          this->send_message(msg, cl_id);
        }
      }
    }
  }
}

// send a message to all members of the file descriptor set
void Server::send_all(char* msg)
{
  int j;
  // for all file descriptors in range
  for (j = 0; j < this->max_fd + 1; j++)
  {
    // if read is valid
    if (FD_ISSET(j, &(this->master)))
    {
      // prevent sending message back to itself
      if (j != this->socket_id)
      {
        send_message(msg, j);
      }
    }
  }
}
