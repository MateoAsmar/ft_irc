#ifndef STATE_HPP
#define STATE_HPP

#include <string>
#include <vector>
#include <map>
#include <set>
#include <deque>
#include <sys/poll.h>

class Client {
public:
    int fd;
    std::string nick;
    std::string user;
    std::string realname;
    bool got_pass;
    bool registered;
    std::string recv_buf;
    std::deque<std::string> send_q;
};

class Channel {
public:
    std::string name;
    std::string topic;
    std::string key;
    bool invite_only;
    bool topic_locked;
    int limit;
    std::vector<Client*> members;
    std::set<std::string> operators;
    std::set<std::string> invited;
};

extern int listen_fd;
extern std::string server_pass;
extern std::string server_name;
extern std::vector<struct pollfd> fds;
extern std::vector<Client*> clients;
extern std::map<std::string,Client*>  nick_map;
extern std::map<std::string,Channel*> channels;


// Global flag for server shutdown
extern bool g_server_shutdown;

// Cleanup function
void cleanup_server();

#endif
