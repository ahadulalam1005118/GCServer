//
// Created by ahad on 7/11/19.
//

#ifndef GCEVENTSERVER_GC2APP_H
#define GCEVENTSERVER_GC2APP_H

#endif //GCEVENTSERVER_GC2APP_H

#include <netdb.h>
#include <cstring>
#include<signal.h>
#include "utility.h"
#include "threadpool.h"
#include "requestData.h"

/*void handle_for_sigpipe() {
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = SIG_IGN;
    sa.sa_flags = 0;
    if(sigaction(SIGPIPE, &sa, NULL))
        return;
}*/
class GC2App {
public:
    //struct epoll_event* events;
    //std::priority_queue<mytimer*, std::deque<mytimer*>, timerCmp> my_timer_queue;
    int _port;
    int _fd;
    int _listen_fd;
    int _accept_fd;
    threadpool_t *threadpool;
    NetStream *stream;
    pthread_mutex_t lock;
    int _epoll_fd;
    std::string recv_string;
    void send_int(uint64_t to_send);
    void send_buf(void *buf, size_t len);

    GC2App(int client_port);
    int socket_bind_listen(int port);
    int set_socket_nonblocking(int fd);
    void server_run();
    void handle_events(int epoll_fd, int listen_fd, struct epoll_event* events, int events_num, threadpool_t* tp);
    void accept_connection(int listen_fd, int epoll_fd);
    static void my_handler(void *args);
    void handle_for_sigpipe();

};
