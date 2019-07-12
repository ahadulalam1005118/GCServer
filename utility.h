//
// Created by ahad on 7/11/19.
//

#ifndef GCEVENTSERVER_UTILITY_H
#define GCEVENTSERVER_UTILITY_H

#endif //GCEVENTSERVER_UTILITY_H

#include <errno.h>
#include <stdarg.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <iostream>

class NetStream {
public:
    int _socket;
    NetStream(int socket);
    size_t read(void *buf, size_t len);
    size_t write(void *buf, size_t len);
    //void handle_for_sigpipe();
    int set_socket_nonblocking(int fd);
};