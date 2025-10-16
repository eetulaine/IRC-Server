# ft_irc
## 📌 Overview

ft_irc is a custom IRC (Internet Relay Chat) server written in C++ as part of the Hive Helsinki core curriculum. It implements the essential features of the IRC protocol — allowing multiple clients to connect, join channels, exchange private messages, and manage channels with operator privileges. While no IRC client was developed for this project, we used Irssi as a reference client to connect, test, and verify the server’s functionality.

## 🎯 Project Focus

  This project is designed to explore key aspects of network programming and real-time communication:
  - TCP socket programming
  - Event-driven I/O using epoll()
  - Parsing and processing text-based commands
  - Managing communication between multiple concurrent clients

## ✨ Features
#### Core IRC Server Functionality

   - Supports multiple simultaneous client connections (non-blocking sockets).
   - Handles authentication and registration via standard IRC commands: PASS, NICK, and USER.
   - Maintains active connections through continuous PING/PONG handshakes.
   - Properly handles client disconnections (QUIT, signal termination, or network failure).
   - Supports nickname changes with live updates to other connected users.
   - Allows private messaging between users.
   - Enables channel creation and group communication.

####  Channel Management

 - Supports channel topics and topic modification.
 - Manages operator privileges for channel owners.
 - Implements channel modes, including:
    - Invite-only (+/-i)
     - Key-protected (+/-k)
   - Topic restriction (+/-t)
   - User limit (+l)
 - Supports channel invitations and user kicks.

####  Logging

 - Clean, informative, and color-coded server activity log for easier monitoring and debugging.

## ⏳ Project Status
Completed

## 🤝 Contributors
[Eetu](https://github.com/eetulaine)

[Hager](https://github.com/imhaqer)

[Shahnaj](https://github.com/shahnajsc)

## Project Status
Submission and peer evaluation is done.
