#include "requestData.h"
#include "utility.h"
#include "epoll.h"
#include <sys/epoll.h>
#include <unistd.h>
#include <sys/time.h>
#include <unordered_map>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <queue>

/*#include <opencv/cv.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
using namespace cv;*/

//test
#include <iostream>
#include <cstring>

using namespace std;

pthread_mutex_t MutexLockGuard::lock = PTHREAD_MUTEX_INITIALIZER;
priority_queue<mytimer*, deque<mytimer*>, timerCmp> myTimerQueue;

requestData::requestData(): 
    now_read_pos(0), 
    state(STATE_PARSE_URI),
    againTimes(0), 
    timer(NULL)
{
    cout << "requestData constructed !" << endl;
}

requestData::requestData(int _epollfd, int _fd):
    now_read_pos(0), 
    state(STATE_PARSE_URI),
    againTimes(0), 
    timer(NULL),
    fd(_fd), 
    epollfd(_epollfd)
{
    std::cout << "I am Inside this epoll event" << std::endl;
}

requestData::~requestData()
{
    cout << "~requestData()" << endl;
    struct epoll_event ev;
    //ev.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
    ev.events = EPOLLIN;
    ev.data.ptr = (void*)this;
    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, &ev);
    if (timer != NULL)
    {
        timer->clearReq();
        timer = NULL;
    }
    close(fd);
}

void requestData::addTimer(mytimer *mtimer)
{
    if (timer == NULL)
        timer = mtimer;
}

int requestData::getFd()
{
    return fd;
}
void requestData::setFd(int _fd)
{
    fd = _fd;
}

void requestData::reset()
{
    againTimes = 0;
    content.clear();
    request_type.clear();
    now_read_pos = 0;
    state = STATE_PARSE_URI;
}

void requestData::seperateTimer()
{
    if (timer)
    {
        timer->clearReq();
        timer = NULL;
    }
}

void requestData::handleRequest()
{
    std::cout << "I am here to process request" << std::endl;
    char buff[MAX_BUFF];
    bool isError = false;
    while (true)
    {
        int read_num = read(fd, buff, MAX_BUFF);
        if (read_num < 0)
        {
            perror("1");
            isError = true;
            break;
        }
        else if (read_num == 0)
        {
            perror("read_num == 0");
            if (errno == EAGAIN)
            {
                if (againTimes > AGAIN_MAX_TIMES)
                    isError = true;
                else
                    ++againTimes;
            }
            else if (errno != 0)
                isError = true;
            break;
        }
        string now_read(buff, buff + read_num);
        content += now_read;
        std::cout << content << endl;

        if (state == STATE_PARSE_URI)
        {
            int flag = parse_URI();
            if (flag == PARSE_URI_AGAIN)
            {
                break;
            }
            else if (flag == PARSE_URI_ERROR)
            {
                perror("2");
                isError = true;
                break;
            }
        }
        if (state == STATE_ANALYSIS)
        {
            int flag = this->analysisRequest();
            if (flag < 0)
            {
                isError = true;
                break;
            }
            else if (flag == ANALYSIS_SUCCESS)
            {

                state = STATE_FINISH;
                break;
            }
            else
            {
                isError = true;
                break;
            }
        }
    }

    if (isError)
    {
        delete this;
        return;
    }
    if (state == STATE_FINISH)
    {
        state = STATE_PARSE_URI;
        this->content = "";
        timer = NULL;
        now_read_pos = 0;
        againTimes = 0;

        //return;
    }
    mytimer *mtimer = new mytimer(this, 500);
    timer = mtimer;

    {
        MutexLockGuard();
        myTimerQueue.push(mtimer);
    }

    __uint32_t _epo_event = EPOLLIN | EPOLLET | EPOLLONESHOT;
    int ret = epoll_mod(epollfd, fd, static_cast<void*>(this), _epo_event);
    if (ret < 0)
    {
        delete this;
        return;
    }
}

int requestData::parse_URI()
{
    string &str = content;
    cout << "now content is:" <<  content << endl;
    int pos = str.find('\r', now_read_pos); // It was default set to 0
    if (pos < 0)
    {
        return PARSE_URI_AGAIN;
    }
    string request_line = content.substr(0, pos);
    pos = request_line.find("GET");  // Parse GET from request line
    method = METHOD_GET;
    std::cout << "we get a GET method" << std::endl;
    std::cout << "now request line is: " << request_line << std::endl;
    pos = request_line.find("/", 0);
    cout << "here one pos is " << pos << std::endl;
    if (pos < 0)
    {
        return PARSE_URI_ERROR;
    }
    else
    {
        int _pos = request_line.find('/', pos+1);
        cout << "here pos is " << _pos << std::endl;
        if (_pos < 0) {
            return PARSE_URI_ERROR;
        }

        else
        {
            if (_pos - pos > 1)
            {
                request_type = request_line.substr(pos + 1, _pos - pos - 1);
                std::cout << "request type is " << request_type << std::endl;
                int __pos = request_type.find('?');
                if (__pos >= 0)
                {
                    request_type = request_type.substr(0, __pos);
                }
            }
                
            else
                request_type = "Allocation";
        }
    }
    state = STATE_ANALYSIS;
    std::cout << "request type is: " << request_type << std::endl;
    return PARSE_URI_SUCCESS;
}


int requestData::analysisRequest()
{
    if (method == METHOD_POST)
    {
        return ANALYSIS_SUCCESS;
    }
    else if (method == METHOD_GET)
    {
        char header[MAX_BUFF];
        std::cout << "we are here inside GET method" << std::endl;
        std::string response;
        if(request_type == "Allocation")
        response = "response from Allocation";    //here the allocation info comes from GC
        else if(request_type == "Region") response = "response from region";
        else if(request_type == "Other") response = "response from Other";
        size_t send_len = write(fd, response.c_str(), response.length()); // here the region info comes from GC
        return ANALYSIS_SUCCESS;
    }
    else
        return ANALYSIS_ERROR;
}



mytimer::mytimer(requestData *_request_data, int timeout): deleted(false), request_data(_request_data)
{
    struct timeval now;
    gettimeofday(&now, NULL);
    expired_time = ((now.tv_sec * 1000) + (now.tv_usec / 1000)) + timeout;
}

mytimer::~mytimer()
{
    cout << "~mytimer()" << endl;
    if (request_data != NULL)
    {
        cout << "request_data=" << request_data << endl;
        delete request_data;
        request_data = NULL;
    }
}

void mytimer::update(int timeout)
{
    struct timeval now;
    gettimeofday(&now, NULL);
    expired_time = ((now.tv_sec * 1000) + (now.tv_usec / 1000)) + timeout;
}

bool mytimer::isvalid()
{
    struct timeval now;
    gettimeofday(&now, NULL);
    size_t temp = ((now.tv_sec * 1000) + (now.tv_usec / 1000));
    if (temp < expired_time)
    {
        return true;
    }
    else
    {
        this->setDeleted();
        return false;
    }
}

void mytimer::clearReq()
{
    request_data = NULL;
    this->setDeleted();
}

void mytimer::setDeleted()
{
    deleted = true;
}

bool mytimer::isDeleted() const
{
    return deleted;
}

size_t mytimer::getExpTime() const
{
    return expired_time;
}

bool timerCmp::operator()(const mytimer *a, const mytimer *b) const
{
    return a->getExpTime() > b->getExpTime();
}


MutexLockGuard::MutexLockGuard()
{
    pthread_mutex_lock(&lock);
}

MutexLockGuard::~MutexLockGuard()
{
    pthread_mutex_unlock(&lock);
}