//
// Created by ahad on 7/11/19.
//

#include "utility.h"
#include <signal.h>
#include <string.h>
#include <fcntl.h>

NetStream::NetStream(int socket) {
    this->_socket = socket;
}

size_t NetStream::read(void *buf, size_t len) {
    int res = recv(this->_socket, buf, len, 0);
    return res;
}

size_t NetStream::write(void *buf, size_t len) {
    int res = send(this->_socket, buf, len, 0);
}

/*
int NetStream::set_socket_nonblocking(int fd) {
    int flag = fcntl(fd, F_GETFL, 0);
    if(flag == -1)
        return -1;

    flag |= O_NONBLOCK;
    if(fcntl(fd, F_SETFL, flag) == -1)
        return -1;
    return 0;
}*/
