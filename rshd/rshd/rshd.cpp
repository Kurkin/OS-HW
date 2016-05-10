//
//  rshd.cpp
//  Rshd daemon
//
//  Created by Kurkin Dmitry on 03.05.16.
//  Copyright © 2015 Kurkin Dmitry. All rights reserved.
//

#include <iostream>
#include <assert.h>
#include <sys/errno.h>
#include <map>
#include <fcntl.h>
#include <sys/socket.h>
#include <signal.h>

#include "rshd.hpp"
#include "throw_error.h"

namespace
{
    void write_some(tcp_client& dest, io_queue& queue, uintptr_t ident)
    {
        if (dest.msg_queue.empty()) {
            queue.delete_event_handler(ident, EVFILT_WRITE);
            return;
        }
        write_part part = dest.msg_queue.front();
        dest.msg_queue.pop_front();
        size_t writted = ::write(static_cast<int>(ident), part.get_part_text(), part.get_part_size());
        if (writted == -1) {
            if (errno != EPIPE) {
                throw_error(errno, "write()");
            } else {
                if (part.get_part_size() != 0) {
                    dest.msg_queue.push_front(part); // no strong exception guarantee
                }
            }
        } else {
            part.writted += writted;
            if (part.get_part_size() != 0) {
                dest.msg_queue.push_front(part);
            }
        }
    }
}

sshd_daemon::sshd_daemon(io_queue& queue, int port): server(server_socket(port)), queue(queue)
{
    server.bind_and_listen();
    
    funct_t connect_client = [this](struct kevent event) {
    
        std::unique_ptr<daemon_tcp_connection> cc(new daemon_tcp_connection(*this, this->queue, tcp_client(client_socket(server))));
        daemon_tcp_connection* pcc = cc.get();
        connections.emplace(pcc, std::move(cc));
        
        pcc->set_client_on_read_write(
                                      [pcc](struct kevent event)
                                      { pcc->client_on_read(event); },
                                      [pcc](struct kevent event)
                                      { pcc->client_on_write(event); });
    };
    
    queue.add_event_handler(server.getfd(), EVFILT_READ, connect_client);
}

sshd_daemon::~sshd_daemon() {
    queue.delete_event_handler(server.getfd(), EVFILT_READ);
}

sshd_daemon::daemon_tcp_connection::daemon_tcp_connection(sshd_daemon& daemon, io_queue& queue, tcp_client client)
: tcp_connection(queue, std::move(client))
, daemon(daemon)
{
    
    ptymfd = posix_openpt(O_RDWR);
    
    if (ptymfd == -1)
        throw_error(errno, "posix_openpt(O_RDWR)");
    if (grantpt(ptymfd) == -1)
        throw_error(errno, "grantpt()");
    if (unlockpt(ptymfd) == -1)
        throw_error(errno, "unlockpt()");
    
    char *slavename = ptsname(ptymfd);
    
    queue.add_event_handler(ptymfd, EVFILT_READ, [this](struct kevent event){
        char buff[event.data];
        size_t red = read(ptymfd, buff, event.data);
        write_to_client({buff, red});
    });
    
    child = fork();
    
    if (child == -1)
        throw_error(errno, "fork()");
    
    if (!child) { //child
        if (setsid() == -1)
            throw_error(errno, "setsid()");
        ptysfd = open(slavename, O_RDWR);
        if (ptysfd == -1)
            throw_error(errno, "open()");
        if (dup2(ptysfd, 0) == -1)
            throw_error(errno, "dup2()");
        if (dup2(ptysfd, 1) == -1)
            throw_error(errno, "dup2()");
        if (dup2(ptysfd, 2) == -1)
            throw_error(errno, "dup2()");
        close(ptymfd);
        close(ptysfd);
        if (execlp("sh", "sh"))
            throw_error(errno, "exec()");
    }
    
    close(ptysfd);
    
}

sshd_daemon::daemon_tcp_connection::~daemon_tcp_connection() {
    queue.delete_event_handler(ptymfd, EVFILT_READ);
    if (kill(child, SIGHUP))
        throw_error(errno, "kill()");
    int status;
    wait(&status);
    close(ptymfd);
}

void sshd_daemon::daemon_tcp_connection::client_on_read(struct kevent event)
{
    if (event.flags & EV_EOF)
    {
        std::cout << "EV_EOF from " << event.ident << " client\n";
        daemon.connections.erase(this);
    }
        else
    {
        char buff[event.data + 1];
        
        size_t size = read(get_client_socket() , buff, event.data);
        
        if (size == static_cast<size_t>(-1))
            throw_error(errno, "read()");
        
        size_t skip = write(ptymfd, buff, size);
        if (skip == -1)
            throw_error(errno, "write()");
        skip++;
        while (skip) {
            size_t t = read(ptymfd, buff, skip);
            if (t == -1)
                throw_error(errno, "read()");
            skip -= t;
        }
    }
}

void sshd_daemon::daemon_tcp_connection::client_on_write(struct kevent event)
{
    write_some(client, queue, event.ident);
}