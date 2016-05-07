//
//  rshd.hpp
//  Rshd daemon
//
//  Created by Kurkin Dmitry on 03.05.16.
//  Copyright © 2015 Kurkin Dmitry. All rights reserved.
//

#ifndef rshd_hpp
#define rshd_hpp

#include <map>

#include "socket.hpp"

struct sshd_daemon
{
private:
    struct daemon_tcp_connection;
    
    std::map<daemon_tcp_connection*, std::unique_ptr<daemon_tcp_connection>> connections;
    server_socket server;
    io_queue& queue;
    
public:
    sshd_daemon(io_queue& queue, int port);
    ~sshd_daemon();
    
private:
    struct daemon_tcp_connection : tcp_connection
    {
        daemon_tcp_connection(sshd_daemon& daemon, io_queue& queue, tcp_client client);
        ~daemon_tcp_connection();
        
        void client_on_write(struct kevent event);
        void client_on_read(struct kevent event);
        
        sshd_daemon& daemon;
        int ptymfd, ptysfd;
        pid_t child;
    };
};

#endif