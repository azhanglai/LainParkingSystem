[TOC]

### 1、使用线程池的原因

- 目前的大多数网络服务器，包括Web服务器、Email服务器以及数据库服务器等都具有一个共同点，就是单位时间内必须处理数目巨大的连接请求，但处理时间却相对较短。
- 多线程方案中我们采用的服务器模型则是一旦接受到请求之后，即创建一个新的线程，由该线程执行任务。任务执行完毕后，线程退出，这就是是“即时创建，即时销毁”的策略。尽管与创建进程相比，创建线程的时间已经大大的缩短，但是如果提交给线程的任务是执行时间较短，而且执行次数极其频繁，那么服务器将处于不停的创建线程，销毁线程的状态。
- 线程本身的开销所占的比例为(线程创建时间+线程销毁时间) / (程创建时间+线程执行时间+线程销毁时间)。如果线程执行的时间很短的话，这比开销可能占到20%-50%左右。如果任务执行时间很长的话，这笔开销将是忽略的。
- 线程池能够减少创建的线程个数。通常线程池所允许的并发线程是有上界的，如果同时需要并发的线程数超过上界，那么一部分线程将会等待。而传统方案中，如果同时请求数目为2000，那么最坏情况下，系统可能需要产生2000个线程。尽管这不是一个很大的数目，但是也有部分机器可能达不到这种要求。
- 线程池的出现是为了减少线程本身带来的开销。线程池采用预创建的技术，在应用程序启动之后，将立即创建一定数量的线程，放入空闲队列中。这些线程都是处于阻塞状态，不消耗CPU，但占用较小的内存空间。当任务到来后，缓冲池选择一个空闲线程，把任务传入此线程中运行。在任务执行完毕后线程也不退出，而是继续保持在池中等待下一次的任务。

### 2、线程池适合场景

-  单位时间内处理任务频繁而且任务处理时间短；
-  对实时性要求较高。如果接受到任务后在创建线程，可能满足不了实时要求，因此必须采用线程池进行预创建。

### 3、编写 ThreadPool 类

~~~c++
#ifndef _THREADPOOL_H
#define _THREADPOOL_H

#include <mutex>
#include <condition_variable>
#include <queue>
#include <thread>
#include <functional>

class ThreadPool {
public:
	explicit ThreadPool(size_t threadCount = 8): m_pool(std::make_shared<Pool>()) {				
        assert(threadCount > 0);
        // 1.创建threadCount条线程，线程传入匿名函数
        // 2.detach： 从thread对象分离执行线程，允许执行独立地持续。一旦该线程退出，
        // 则释放任何分配的资源。调用 detach 后 *this 不再占有任何线程。
        for(size_t i = 0; i < threadCount; i++) {
            std::thread([pool = m_pool] {
                std::unique_lock<std::mutex> locker(pool->mtx); 	// 给线程池上锁
                while(true) {
                    if(!pool->tasks.empty()) {						// 任务队列有任务
                        auto task = std::move(pool->tasks.front());	// 从任务队列头部取任务
                        pool->tasks.pop();
                        locker.unlock();							// 任务取出来了，给池子解锁
                        task();										// 运行任务
                        locker.lock();								// 给池子再次上锁，因为循环取任务的
                    } 
                    else if(pool->isClosed) break; 					// 线程池没有开启
                    else pool->cond.wait(locker);					// 没有任务会阻塞，等待有任务通知了，才会唤醒
                }
            }).detach();
        }
    }

    ThreadPool() = default;
    ThreadPool(ThreadPool&&) = default;
    
    ~ThreadPool() {
        if(static_cast<bool>(m_pool)) {
            {
                // lock_guard: 作用域锁， 析构池子，把池子的状态设置为关闭
                std::lock_guard<std::mutex> locker(m_pool->mtx);
                m_pool->isClosed = true;
            }
            m_pool->cond.notify_all();	// 池子关闭了，通知所有线程关闭
        }
    }
	// 给池子的任务队列添加任务
    template<class F>
    void AddTask(F&& task) {
        {
            std::lock_guard<std::mutex> locker(m_pool->mtx);
            m_pool->tasks.emplace(std::forward<F>(task));	// 添加任务函数
        }
        m_pool->cond.notify_one();							// 有任务了，通知一个线程去抢
    }

private:
    // Pool: 存放线程的池子
    struct Pool {
        std::mutex mtx;							 // 用于保护共享数据免受从多个线程同时访问的同步原语
        std::condition_variable cond;			 // 用于阻塞一个线程，或同时阻塞多个线程，直至另一线程修改共享变量（条件）并通知cond
        bool isClosed;							 // 标记线程池是否开启状态
        std::queue<std::function<void()>> tasks; // 任务队列
    };
    std::shared_ptr<Pool> m_pool;
};

#endif /* _THREADPOOL_H */

~~~

### 4、Epoll的引出

- 内核接受网络数据过程：进程调用read阻塞，进程PCB从工作队列放到等待队列，期间计算机收到了对端传送的数据，数据经由网卡传送到内存，然后网卡通过中断信号通知CPU处理器有数据到达，CPU处理器执行中断程序。此中断程序主要有两项功能：①先将网络数据写入到对应socket的接收缓冲区里面。②再唤醒被阻塞的进程，重新将进程放入工作队列中，可以被CPU处理器调度。

- 一个socket对应着一个Port，网络数据包中包含IP和Port的信息，内核可以通过Port找到对应的socket。为了提高处理速度，操作系统会维护Port到socket的索引结构，以快速读取。网卡中断CPU后，CPU的中断函数从网卡存数据的内存拷贝数据到对应socket的接收缓冲区，具体是哪一个socket，CPU会检查Port，放到对应的socket。

- 服务端需要管理多个客户端连接，而read只能监视单个socket。这时，人们开始寻找监视多个socket的方法。epoll的要义是高效的监视多个socket。

- 假如能够预先传入一个socket列表，如果列表中的socket都没有数据，挂起进程，直到有一个socket收到数据，唤醒进程。这种方法很直接，也是select的设计思想。

- select的缺点：①每次调用select都需要将进程加入到所有监视socket的等待队列，每次唤醒都需要从每个队列中移除。这里涉及了两次遍历。②而且每次都要将整个fds列表拷贝给内核，有一定的开销。正是因为遍历操作开销大，出于效率的考量，才会规定select的最大监视数量，默认只能监视1024个socket。③进程被唤醒后，程序并不知道哪些socket收到数据，还需要遍历一次（这一次遍历是在应用层）。

- 当程序调用select时，内核会先遍历一遍socket，如果有一个以上的socket接收缓冲区有数据，那么select直接返回，不会阻塞。这也是为什么select的返回值有可能大于1的原因之一。如果没有socket有数据，进程才会阻塞。

- epoll是select和poll的增强版本。epoll通过一些措施来改进效率：措施①：功能分离。select低效的原因之一是将“维护等待队列”和“阻塞进程”两个步骤合二为一，然而大多数应用场景中，需要监视的socket相对固定，并不需要每次都修改。epoll将这两个操作分开，先用epoll_ctl维护等待队列，再调用epoll_wait阻塞进程（解耦）。措施②：就绪列表。select低效的另一个原因在于程序不知道哪些socket收到数据，只能一个个遍历，如果内核维护一个“就绪列表”，引用收到数据的socket，就能避免遍历。epoll使用双向链表来实现就绪队列，双向链表是能够快速插入和删除的数据结构，收到数据的socket被就绪列表所引用，只要获取就绪列表的内容，就能够知道哪些socket收到数据。

- epoll 索引结构。既然epoll将“维护监视队列”和“进程阻塞”分离，也意味着需要有个数据结构来保存监视的socket。至少要方便的添加和移除，还要便于搜索，以避免重复添加。红黑树是一种自平衡二叉查找树，搜索、插入和删除时间复杂度都是O(log(N))，效率较好。epoll使用了红黑树作为索引结构。

  ps：因为操作系统要兼顾多种功能，以及由更多需要保存的数据，就绪列表并非直接引用socket，而是通过epitem间接引用，红黑树的节点也是epitem对象。同样，文件系统也并非直接引用着socket。

- 什么时候select优于epoll？

  如果在并发量低，socket都比较活跃的情况下，select效率更高，也就是说活跃socket数目与监控的总的socket数目之比越大，select效率越高，因为select反正都会遍历所有的socket，如果比例大，就没有白白遍历。加之于select本身实现比较简单，导致总体现象比epoll好

### 5、Epoll的原理和流程

#### 5.1 epoll_create 创建epoll对象

- 某个进程调用epoll_create方法时，内核会创建一个eventpoll对象。
- eventpoll对象也是文件系统中的一员，和socket一样，它也会有等待队列（有线程会等待其事件触发，比如调用epoll_wait的线程就会阻塞在其上）。

#### 5.2 epoll_ctl 维护监视列表

- 创建epoll对象后，可以用epoll_ctl添加或删除所要监听的socket。
- epoll_ctl添加操作，内核会将eventpoll添加到socket的等待队列中。
- 当socket收到数据后，中断程序会操作eventpoll对象，而不是直接操作进程（也就是调用epoll的进程）。

#### 5.3 接受数据

- 当socket收到数据后，中断程序会给eventpoll的“就绪列表”添加socket引用。
- eventpoll对象相当于是socket和进程之间的中介，socket的数据接收并不直接影响进程，而是通过改变eventpoll的就绪列表来改变进程状态。
- 当程序执行到epoll_wait时，如果就绪列表已经引用了socket，那么epoll_wait直接返回，如果就绪列表为空，阻塞进程。

#### 5.4 epoll_wait 阻塞进程

- 假设计算机中正在运行进程A和进程B，在某时刻进程A运行到了epoll_wait语句。内核会将进程A放入eventpoll的等待队列中，阻塞进程。
- 当socket接收到数据，中断程序一方面修改就绪队列，另一方面唤醒eventpoll等待队列中的进程，进程A再次进入运行状态。也因为就绪列表的存在，进程A可以知道哪些socket发生了变化。

### 6、 编写 Epoller 类

#### 6.1 define.h 头文件

~~~c++
#ifndef _DEFINE_H
#define _DEFINE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <errno.h>
#include <time.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/stat.h>   
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#endif /*  _DEFINE_H */

~~~

#### 6.2 Epoller.h 头文件

~~~c++
#ifndef _EPOLLER_H
#define _EPOLLER_H

#include "./define.h"
#include <vector>

class Epoller {
public:
    explicit Epoller(int maxEvent = 1024);
    ~Epoller();

    bool AddFd(int fd, uint32_t events);
    bool ModFd(int fd, uint32_t events);
    bool DelFd(int fd);
    int Wait(int timeout = -1);
    int GetEventFd(size_t i) const;
    uint32_t GetEvents(size_t i) const;

private:
    int m_epollFd;
    std::vector<struct epoll_event> m_events;    
};

#endif /* _EPOLLER_H */

~~~

#### 6.3 Epoller.cpp 源文件

~~~c++
#include "./epoller.h"

// epoll_create: 创建epoll红黑树索引结构
// 并且初始化epoll_event 数组
Epoller::Epoller(int maxEvent):m_epollFd(epoll_create(1)), m_events(maxEvent){
    assert(m_epollFd >= 0 && m_events.size() > 0);
}

Epoller::~Epoller() {
    close(m_epollFd);
}

// 将fd的events事件 注册到 epoll
bool Epoller::AddFd(int fd, uint32_t events) {
    if(fd < 0) return false;
    epoll_event ev = {0};
    ev.data.fd = fd;
    ev.events = events;
    return 0 == epoll_ctl(m_epollFd, EPOLL_CTL_ADD, fd, &ev);
}

// 更改epoll中 fd的events事件
bool Epoller::ModFd(int fd, uint32_t events) {
    if(fd < 0) return false;
    epoll_event ev = {0};
    ev.data.fd = fd;
    ev.events = events;
    return 0 == epoll_ctl(m_epollFd, EPOLL_CTL_MOD, fd, &ev);
}

// 将注册到epoll中的fd删除
bool Epoller::DelFd(int fd) {
    if(fd < 0) return false;
    epoll_event ev = {0};
    return 0 == epoll_ctl(m_epollFd, EPOLL_CTL_DEL, fd, &ev);
}

// epoll_wait: 等待timeout时间，注册到epoll里面的事件如果有就绪的会添加到m_events中 
int Epoller::Wait(int timeout) {
    return epoll_wait(m_epollFd, &m_events[0], static_cast<int>(m_events.size()), timeout);
}

// 得到m_events[i]中的fd 
int Epoller::GetEventFd(size_t i) const {
    assert(i < m_events.size() && i >= 0);
    return m_events[i].data.fd;
}

// 得到m_events[i]中的事件
uint32_t Epoller::GetEvents(size_t i) const {
    assert(i < m_events.size() && i >= 0);
    return m_events[i].events;
}
~~~

### 7、 实现EpollServer 框架

#### 7.1 epollserver.h

~~~c++
#ifndef _EPOLLSERVER_H
#define _EPOLLSERVER_H

#include "./define.h"
#include "./epoller.h"
#include "./threadpool.h" 
#include <memory>

class EpollServer {
public:
    EpollServer(int port, bool OptLinger, int trigMode, int threadNum); // 服务器初始化
    ~EpollServer();
    void Start();							// 服务器开始运行

private:
    int m_port;								// 服务器端口号
    bool m_openLinger;						// 是否优雅关闭连接的标记
    bool m_isClose;							// 服务器是否开启状态标记
    int m_listenFd;							// TCP连接监听套接字fd
    
    uint32_t m_listenEvent; 				// 监听套接字epoll监听事件 
    uint32_t m_connEvent;					// 客户连接套接字epoll监听事件
    
    std::unique_ptr<Epoller> m_epoller;		// epoll指针对象

    bool _Init_Socket();					// 初始化TCP连接
    void _Init_EventMode(int trigMode);		// 初始化epoll触发模式
    static int Set_FdNonblock(int fd);		// 设置非阻塞模式
    
    void _Deal_Listen();
    void _Deal_Read(int fd);				// 处理客户端读事件
    void _Deal_Write(int fd);				// 处理客户端写事件
    
	void _Thread_Read(int fd); 				// 线程任务函数
    void _Thread_Write(int fd); 			// 线程任务函数
};

#endif /* _EPOLLSERVER_H */

~~~

#### 7.2 epollserver.cpp

~~~c++
#include "./epollserver.h"

EpollServer::EpollServer(int port, bool OptLinger, int trigMode, int threadNum)
    : m_port(port), m_openLinger(OptLinger),  m_isClose(false),
    m_epoller(new Epoller()), m_threadpool(new ThreadPool(threadNum))
{    
    _Init_EventMode(trigMode);
    if (!_Init_Socket()) {
        m_isClose = true;
    }
}

EpollServer::~EpollServer() {
    close(m_listenFd);
    m_isClose = true;
}

void EpollServer::Start() {
    if (!m_isClose) {
        printf("========== Server Start ==========\n");
    }
    while (!m_isClose) {
        // m_epoller->Wait: 阻塞等待就绪事件，返回就绪事件的个数
        int nfd = m_epoller->Wait();
        for (int i = 0; i < nfd; ++i) {
            int fd = m_epoller->GetEventFd(i);			// 取得就绪事件的fd
            uint32_t events = m_epoller->GetEvents(i); 	// 取得就绪事件
            // 如果是监听套接字的事件就绪，说明有客户要连接
            if (fd == m_listenFd) {
                _Deal_Listen();
            }
            // 事件挂起或出错
            else if (events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                assert(fd > 0);
                close(fd);
            }
            // 客户读事件就绪
            else if (events & EPOLLIN) {
                assert(fd > 0);
                _Deal_Read(fd);
            }
            // 客户写事件就绪
            else if (events & EPOLLOUT) {
                assert(fd > 0);
                _Deal_Write(fd);
            }
            else {
                printf("Unexpected event\n");
            }
        }
    }   
} 

bool EpollServer::_Init_Socket() {
    int ret;
    struct sockaddr_in addr;
    if (m_port > 65535 || m_port < 1024) {
        printf("Port:%d error!\n", m_port);
        return false;
    }
    addr.sin_family = AF_INET;
    addr.sin_port = htons(m_port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    struct linger optLinger = {0};
    if (m_openLinger) {
        // 优雅关闭: 直到所剩数据发送完毕或超时
        optLinger.l_onoff = 1;
        optLinger.l_linger = 1;
    }

    m_listenFd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_listenFd < 0) {
        printf("socket error!\n");
        return false;
    }

    ret = setsockopt(m_listenFd, SOL_SOCKET, SO_LINGER, &optLinger, sizeof(optLinger));
    if (ret < 0) {
        close(m_listenFd);
        printf("setsockopt1 error!\n");
        return false;
    }
    // 设置端口复用
    int val = 1;
    ret = setsockopt(m_listenFd, SOL_SOCKET, SO_REUSEADDR, (const void*)&val, sizeof(int));
    if (ret < 0) {
        close(m_listenFd);
        printf("setsockopt2 error!\n");
        return false;
    }
    // 绑定端口
    ret = bind(m_listenFd, (struct sockaddr*)&addr, sizeof(addr));
    if (ret < 0) {
        printf("bind error!\n");
        close(m_listenFd);
        return false;
    }
    // 监听套接字 
    ret = listen(m_listenFd, SOMAXCONN);
    if (ret < 0) {
        printf("listen error!\n");
        close(m_listenFd);
        return false; 
    }
    // 把监听fd注册到epoll
    ret = m_epoller->AddFd(m_listenFd, m_listenEvent | EPOLLIN);
    if (ret == 0) {
        close(m_listenFd);
        return false;
    }
    // 把监听fd 设置成非阻塞 
    Set_FdNonblock(m_listenFd);
    return true;
}

void EpollServer::_Init_EventMode(int trigMode) {
    // EPOLLONESHOT: 只监听一次事件，如还需要监听的话，需要再次把fd注册到epoll
    m_listenEvent = EPOLLRDHUP;
    m_connEvent = EPOLLRDHUP | EPOLLONESHOT;
    // 0：默认水平触发LT
    // 1：listen 为 LT， conn 为ET(边沿触发)
    // 2：listen 为 ET， conn 为LT
    // 3：listen 为 ET， conn 为ET
    switch (trigMode) {
        case 0: { break; }
        case 1: {
            m_connEvent |= EPOLLET;
            break;
        }
        case 2: {
            m_listenEvent |= EPOLLET;
            break;
        }
        case 3: {
            m_listenEvent |= EPOLLET;
            m_connEvent |= EPOLLET;
            break;
        }
        defalut: {
            m_listenEvent |= EPOLLET; 
            m_connEvent |= EPOLLET;
            break;
        }
    }
}

int EpollServer::Set_FdNonblock(int fd) {
    assert(fd > 0);
    return fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);
}

void EpollServer::_Deal_Listen() {
    struct sockaddr_in cli_addr;
    socklen_t len = sizeof(cli_addr);
    do {
        int fd = accept(m_listenFd, (struct sockaddr*)&cli_addr,&len);
        if (fd <= 0) { return ; }
        m_epoller->AddFd(fd, m_connEvent | EPOLLIN);
        Set_FdNonblock(fd);
    } while (m_listenEvent & EPOLLET);
}

void EpollServer::_Deal_Read(int fd) {
    m_threadpool->AddTask(std::bind(&EpollServer::_Thread_Read, this, fd));
}

void EpollServer::_Deal_Write(int fd) {
    m_threadpool->AddTask(std::bind(&EpollServer::_Thread_Write, this, fd));
}

void EpollServer::_Thread_Read(int fd) {  
    m_epoller->ModFd(fd, m_connEvent | EPOLLOUT);
}

void EpollServer::_Thread_Write(int fd) { 
    m_epoller->ModFd(fd, m_connEvent | EPOLLIN);
}

~~~



