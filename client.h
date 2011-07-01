#include <string>
#include "process.h"

using namespace std;

class Client : public Process
{
  public:
    Client();
    Client(string, int, string);
    ~Client();
    void make_socket(void);
    void reset_socket(void);
    void connect_socket(void);
    void connect_socket(int, struct sockaddr_in);
    void send_nickname(void);
    void chat(void);
    void receive_socket(void);
    int socket_id;
    string nickname;
    struct sockaddr_in server_addr;
};
