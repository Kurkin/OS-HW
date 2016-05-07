//
//  main.cpp
//  rshd
//
//  Created by Kurkin Dmitry on 5/3/16.
//  Copyright © 2016 Kurkin Dmitry. All rights reserved.
//

#include <iostream>
#include <string>
#include <fcntl.h>

#include "throw_error.h"
#include "rshd.hpp"

void make_daemon()
{
    pid_t parent = fork();
    if (parent == -1)
        throw_error(errno, "fork()");
    
    if (parent)
    {
        exit(0);
    } else {
        pid_t sid = setsid(); // make new session
        
        if (sid == -1)
            throw_error(errno, "setsid()");
        
        parent = fork();
        
        if (parent == -1)
            throw_error(errno, "fork()");
    
        if (parent) {
            int info = open("/tmp/rshd.pid", O_RDWR | O_CREAT);
            if (info == -1)
                throw_error(errno, "open()");
            if (write(info, std::to_string(parent).c_str(), std::to_string(parent).length()) == -1)
                throw_error(errno, "write()");
            close(info);
            exit(0);
        } else {
            return; // we are daemon!
        }
    }
}

int main(int argc, const char * argv[]) {
    
    if (argc == 1) {
        std::cout << "Usage: rshd [port]" << std::endl;
        return 0;
    }
        
    try {
        make_daemon();
        io_queue queue;
        sshd_daemon deamon(queue, std::stoi(argv[1]));
        queue.watch_loop();
    }  catch (std::runtime_error const& error) {
        std::cout << error.what() << std::endl;
    }
    
    return 0;
    
}