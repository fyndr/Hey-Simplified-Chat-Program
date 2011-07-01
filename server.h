#include <unistd.h>
#include <map>
#include "process.h"

using namespace std;

class Server : public Process
{
  public:
    Server();
    Server(int);
    ~Server();
    void reuse_addresses(int*);
    void bind_socket(struct sockaddr_in);
    void listen_socket(int);
    int accept_socket(void);
    void setup(void);
    void close_socket(void);
    void process_messages(void);
    void send_all(char*);
    map<int, string> nicks;
    int socket_id;
    struct sockaddr_in client_addr;
    fd_set master;
    fd_set temp_fds;
    int max_fd;
};
