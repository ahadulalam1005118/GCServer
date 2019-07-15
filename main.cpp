#include <iostream>
#include "threadpool.h"
#include "requestData.h"
#include<queue>
// #include "utility.h"
#include "GC2App.h"
/*struct epoll_event* events;
std::priority_queue<mytimer*, std::deque<mytimer*>, timerCmp> my_timer_queue;*/
//extern struct epoll_event* events;
//extern std::priority_queue<mytimer*, std::deque<mytimer*>, timerCmp> my_timer_queue;
int main() {
    GC2App app = GC2App(5333);
    app.handle_for_sigpipe();
    app.server_run();


}