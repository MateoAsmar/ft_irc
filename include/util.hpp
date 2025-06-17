#ifndef UTIL_HPP
#define UTIL_HPP

#include <string>
#include <vector>
#include <fcntl.h>
#include <algorithm>
#include "state.hpp"

// Basic helpers:
std::vector<std::string> split(const std::string& s, char delim);
void set_nb(int fd);

// state lookup
Client*  find_client(int fd);
Client*  find_nick(const std::string& n);
Channel* get_chan(const std::string& name);

// sending & queuing
void queue_raw(Client* c, const std::string& line);
void send_err(Client* c, const std::string& code, const std::string& tgt, const std::string& txt);
void send_rpl(Client* c, const std::string& code, const std::string& tgt, const std::string& txt);

bool is_number(const std::string& s);

#endif
