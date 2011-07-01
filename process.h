using namespace std;

class Process
{
  public:
    void send_message(char*, int);
    char* receive_message(int);
    int get_socket(void);
    struct sockaddr_in get_socket_address(int, string);
    string host;
    int port;
};
