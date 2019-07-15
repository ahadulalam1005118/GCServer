//
// Created by ahad on 7/11/19.
//

#include <sys/epoll.h>
#include <zconf.h>
//#include <bits/fcntl-linux.h>
#include <fcntl.h>
#include<queue>
#include<string>
#include "GC2App.h"
#include "requestData.h"
#include "epoll.h"

const int THREADPOOL_THREAD_NUM = 8;
const int QUEUE_SIZE = 65535;
const int TIMER_TIME_OUT = 500;
extern struct epoll_event* events;
std::priority_queue<mytimer*, std::deque<mytimer*>, timerCmp> my_timer_queue;
GC2App::GC2App(int server_port) {
    {
        this->_port = server_port;
        pthread_mutex_init(&this->lock, NULL);
    }
}

int GC2App::socket_bind_listen(int port) {
    if (port < 1024 || port > 65535)
        return -1;
    int listen_fd = 0;
    if((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        return -1;
    int optval = 1;
    if(setsockopt(listen_fd, SOL_SOCKET,  SO_REUSEADDR, &optval, sizeof(optval)) == -1)
        return -1;
    struct sockaddr_in server_addr;
    bzero((char*)&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons((unsigned short)port);
    if(bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
        return -1;
    if(listen(listen_fd, LISTENQ) == -1)
        return -1;
    if(listen_fd == -1)
    {
        close(listen_fd);
        return -1;
    }

    return listen_fd;
}

int GC2App::set_socket_nonblocking(int fd) {
    int flag = fcntl(fd, F_GETFL, 0);
    if(flag == -1)
        return -1;

    flag |= O_NONBLOCK;
    if(fcntl(fd, F_SETFL, flag) == -1)
        return -1;
    return 0;
}

void GC2App::handle_events(int epoll_fd, int listen_fd, struct epoll_event *events, int events_num, threadpool_t *tp) {
    std::cout << "passed handle events" << std::endl;
    for(int i = 0; i < events_num; i++)
    {
        requestData* request = (requestData*)(events[i].data.ptr);
        std::cout << "passed request data creation" << std::endl;
        int fd = request->getFd();
        //int fd = events[i].data.fd;
        if(fd == listen_fd)
        {
            std::cout << "connection acceptance request" << std::endl << std::endl << std::endl;
            this->accept_connection(listen_fd, epoll_fd);
        }
        else
        {
            std::cout << "different event" << std::endl;
            if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP)
                || (!(events[i].events & EPOLLIN)))
            {
                delete request;
                continue;
            }
            request->seperateTimer();
            int rc = threadpool_add(tp, my_handler, events[i].data.ptr, 0);
            //std::cout<< "output value of rc is: " + rc << std::endl;
        }
    }
}

void GC2App::accept_connection(int listen_fd, int epoll_fd) {
    struct sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(struct sockaddr_in));
    socklen_t client_addr_len = 0;
    int accept_fd = 0;
    while((accept_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &client_addr_len)) > 0)
    {
        int ret = this->set_socket_nonblocking(accept_fd);
        if (ret < 0)
        {
            perror("Set non block failed!");
            return;
        }
        std::cout<< "I am here after connected" << std::endl;

        requestData *req_info = new requestData(epoll_fd, accept_fd);
        __uint32_t _epo_event = EPOLLIN | EPOLLET | EPOLLONESHOT;
        epoll_add(epoll_fd, accept_fd, static_cast<void*>(req_info), _epo_event);
        mytimer *mtimer = new mytimer(req_info, TIMER_TIME_OUT);
        std::cout << "Set separate timer for new event" << std::endl;
        req_info->addTimer(mtimer);
        MutexLockGuard();
        my_timer_queue.push(mtimer);
    }
}

void GC2App::my_handler(void *args) {
    //std::cout << "I am inside request data's my handler" << std::endl;
    requestData *req_data =  (requestData*) args;
    req_data->handleRequest();
}

void GC2App::handle_for_sigpipe() {
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = SIG_IGN;
    sa.sa_flags = 0;
    if(sigaction(SIGPIPE, &sa, NULL))
        return;
}

void GC2App::server_run() {
    this->_epoll_fd = epoll_init();
    if (this->_epoll_fd < 0)
    {
        perror("epoll init failed");
    }
    this->_listen_fd = this->socket_bind_listen(this->_port);
    this->set_socket_nonblocking(_listen_fd);
    __uint32_t event = EPOLLIN | EPOLLET;
    requestData *req = new requestData();
    req->setFd(this->_listen_fd);
    epoll_add(this->_epoll_fd, this->_listen_fd, static_cast<void*>(req), event);
    this->threadpool = threadpool_create(THREADPOOL_THREAD_NUM, QUEUE_SIZE, 0);
    while(true) {
        int events_num = my_epoll_wait(this->_epoll_fd, events, MAXEVENTS, -1);
        if (events_num == 0)
            continue;
        //printf("%d\n", events_num);
        std::cout << "number of events are " << events_num << std::endl;
        this->handle_events(this->_epoll_fd, this->_listen_fd, events, events_num, this->threadpool);

    }

}