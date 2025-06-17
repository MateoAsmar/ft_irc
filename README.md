# ft_irc  
*(42 Network Project — IRC server in C++)*

# Grade: 100/100
![Screenshot from 2025-06-17 12-45-46](https://github.com/user-attachments/assets/0863852f-2af5-435a-9d05-de74eda211ce)


## Overview

**ft_irc** is a student‐built implementation of a simplified Internet Relay Chat (IRC) server, conforming to the RFC 1459 specification. It supports multiple simultaneous clients, channel creation and management, private and channel messaging, and the core IRC command set.

Key features:

- **Non-blocking I/O & multiplexing**  
  Uses TCP sockets in non-blocking mode and `poll(2)` for scalable client handling without threads.  
- **Core IRC commands**  
  Implements `PASS`, `NICK`, `USER`, `JOIN`, `PART`, `PRIVMSG`, `TOPIC`, `MODE`.
- **User and channel management**  
  Tracks nicknames, registration state, channel membership, and operator privileges.  
- **Protocol compliance**  
  Robust IRC message parsing (prefix, command, parameters, trailing) and correct numeric replies / error codes.

## Repository Layout
```
ft_irc/

├── Makefile

├── README.md

├── include/

│ ├── commands.hpp

│ ├── server.hpp

│ ├── state.hpp

│ └── util.hpp

└── src/

├── main.cpp

├── server.cpp

├── state.cpp

└── util.cpp

│ └── commands

├── cmd_INVITE.cpp

├── cmd_JOIN.cpp

├── cmd_KICK.cpp

├── cmd_MODE.cpp

├── cmd_NAMES.cpp

├── cmd_NICK.cpp

├── cmd_PART.cpp

├── cmd_PASS.cpp

├── cmd_PRIVMSG.cpp

├── cmd_TOPIC.cpp

├── cmd_USER.cpp
```

## Build & Run

```bash
git clone REPO
cd ft_irc
make
```

### Usage:
To run the server:

```bash
./ircserv <port> <password>
```

To connect clients:
```bash
nc <IP address where server is running> <port>
```

## What I Learned
- UNIX socket programming — Creating, binding, and listening on non-blocking TCP sockets.

- Event-driven architecture — Using poll(2) for multiplexed I/O instead of multi-threading.

- Text-based protocol parsing — Splitting IRC messages into prefix, command, parameters, and trailing arguments.

- State management — Handling partial packets, client registration, nick/channel lists, and operator flags.

- Error handling & compliance — Emitting the correct numeric replies (e.g. ERR_NEEDMOREPARAMS, RPL_NAMREPLY) and rejecting malformed commands.

## Conclusion
The ft_irc project deepened my understanding of low-level network APIs, non-blocking I/O, and the design of text-based protocols. Building an IRC server from scratch reinforced best practices in scalable server design, stateful client management, and protocol compliance.
