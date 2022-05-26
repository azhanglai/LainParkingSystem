#include "./epollserver.h"

ShmFifo shm_fifo1(11, 10, 1024);
ShmFifo shm_fifo2(22, 10, 1024);

map<int, PACK_HEART> EpollServer::conntimer;
mutex EpollServer::mtx;

EpollServer::EpollServer(int port, bool OptLinger, int trigMode, int threadNum)
    : m_port(port), m_openLinger(OptLinger),  m_isClose(false),
    m_epoller(new Epoller()), m_threadpool(new ThreadPool(threadNum))
{  
	m_msgid = msgget(1234, IPC_CREAT | IPC_EXCL | 0666);
    if (m_msgid == -1) {
        m_msgid = msgget(1234, 0);
    }

	m_msgid_back = msgget(12345, IPC_CREAT | IPC_EXCL | 0666);
	if (m_msgid_back == -1) {
        m_msgid_back = msgget(12345, 0);
	}

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
        printf("========== EpollServer Start ==========\n");
    }
    m_threadpool->AddTask(std::bind(&EpollServer::_Thread_Heart, this));
    while (!m_isClose) {
        // m_epoller->Wait: 阻塞等待就绪事件，返回就绪事件的个数
        int nfd = m_epoller->Wait();
        for (int i = 0; i < nfd; ++i) {
            int fd = m_epoller->GetEventFd(i);			// 取得就绪事件的fd
            uint32_t events = m_epoller->GetEvents(i); 	// 取得就绪事件
            // 如果是监听套接字的事件就绪，说明有客户要连接
            if (fd == m_listenFd) {
                printf("===== _Deal_Listen =====\n");
                _Deal_Listen();
            }
            // 事件挂起或出错
            else if (events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                assert(fd > 0);
                printf("===== _Close_Conn =====\n");
                _Close_Conn(fd);
            }
            // 客户读事件就绪
            else if (events & EPOLLIN) {
                assert(fd > 0);
                printf("===== _Deal_Read =====\n");
                _Deal_Read(fd);
            }
            // 客户写事件就绪
            else if (events & EPOLLOUT) {
                assert(fd > 0);
                printf("===== _Deal_Write =====\n");
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
        PACK_HEART heart;
        heart.fd = fd;
        heart.time = 128;
        conntimer.insert(pair<int, PACK_HEART>(fd, heart));
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

void EpollServer::_Thread_Heart() {
    while (true) {
        sleep(1);
        for (auto it = EpollServer::conntimer.begin(); it != EpollServer::conntimer.end(); ++it) {
            (*it).second.time--;
            if ((*it).second.time == 0) {
                {
                    lock_guard<mutex> locker(EpollServer::mtx);
                    _Close_Conn((*it).second.fd);
                    EpollServer::conntimer.erase(it);
                }

            }
        }
    }
}

void EpollServer::_Thread_Read(int fd) {  
    time_t nowTime;
    nowTime = time(NULL);

    // 获取当前系统时间
    struct tm* sysTime = localtime(&nowTime);
    char nowtime_buf[50] = {0};
    char nowtime_buffer[50] = {0};
    sprintf(nowtime_buf, "./log/%d-%02d-%02d.txt", sysTime->tm_year + 1900, sysTime->tm_mon + 1, sysTime->tm_mday);

    sprintf(nowtime_buffer, "%d-%02d-%02d/%02d:%02d:%02d", sysTime->tm_year + 1900, sysTime->tm_mon + 1, sysTime->tm_mday, sysTime->tm_hour, sysTime->tm_min, sysTime->tm_sec);

    // 有客户端发送数据过来
    MYBUF msg;
    msg.mtype = 1;
    size_t msgsize = sizeof(MYBUF) - sizeof(long);
    PACK_HEAD head;
    bzero(&head, sizeof(PACK_HEAD));

    int ret = read(fd, &head, sizeof(head));
    if (ret > 0) {
        // 注册
        if (head.bodyType == REGIST_TYPE) {
            PACK_REGIST_LOGIN regist_login;
            int res = read(fd, &regist_login, sizeof(PACK_REGIST_LOGIN));
            printf("EpollServer recv: regist\n");
            char buffer[sizeof(PACK_HEAD) + sizeof(PACK_REGIST_LOGIN)] = {0};
            memcpy(buffer, &head, sizeof(PACK_HEAD));
            memcpy(buffer + sizeof(PACK_HEAD), &regist_login, sizeof(PACK_REGIST_LOGIN));

            if (res > 0) {
                shm_fifo1.shm_write(buffer);
                int res = msgsnd(m_msgid, &msg, msgsize, 0);
                printf("EpollServer: regist msgsnd\n");
            }
        }
        // 登录
        else if (head.bodyType == LOGIN_TYPE) {
            PACK_REGIST_LOGIN regist_login;
            int res = read(fd, &regist_login, sizeof(PACK_REGIST_LOGIN));
            printf("EpollServer recv: login\n");

            char buffer[sizeof(PACK_HEAD) + sizeof(PACK_REGIST_LOGIN)] = {0};
            memcpy(buffer, &head, sizeof(PACK_HEAD));
            memcpy(buffer + sizeof(PACK_HEAD), &regist_login, sizeof(PACK_REGIST_LOGIN));

            if (res > 0) {
                shm_fifo1.shm_write(buffer);
                int res = msgsnd(m_msgid, &msg, msgsize, 0);
                printf("EpollServer: login msgsnd\n");
            }
        }
        // 汽车入停车场
        else if (head.bodyType == CAR_GETIN) {
            PACK_ENTER car_enter;
            int res = read(fd, &car_enter, sizeof(PACK_ENTER));
            printf("EpollServer recv: getin\n");

            char buffer[sizeof(PACK_HEAD) + sizeof(PACK_ENTER)] = {0};
            memcpy(buffer, &head, sizeof(PACK_HEAD));
            memcpy(buffer + sizeof(PACK_HEAD), &car_enter, sizeof(PACK_ENTER));

            if (res > 0) {
                shm_fifo1.shm_write(buffer);
                int res = msgsnd(m_msgid, &msg, msgsize, 0);
                printf("EpollServer: getin msgsnd\n");
            }
        }
        // 汽车出停车场
        else if (head.bodyType == CAR_GETOUT) {
            PACK_EXIT car_exit;
            int res = read(fd, &car_exit, sizeof(PACK_EXIT));
            printf("EpollServer recv: getout\n");
            
            char buffer[sizeof(PACK_HEAD) + sizeof(PACK_EXIT)] = {0};
            memcpy(buffer, &head, sizeof(PACK_HEAD));
            memcpy(buffer + sizeof(PACK_HEAD), &car_exit, sizeof(PACK_EXIT));

            if (res > 0) {
                shm_fifo1.shm_write(buffer);
                int res = msgsnd(m_msgid, &msg, msgsize, 0);
                printf("EpollServer: getout msgsnd\n");
            }
        }
        // 心跳
        else if (head.bodyType == HEART_TYPE) {
            auto it = EpollServer::conntimer.find(fd);
            if (it != EpollServer::conntimer.end()) {
                {
                    lock_guard<mutex> locker(EpollServer::mtx);
                    (*it).second.time = 128;
                }
            }
        }
        // 图片
        else if (head.bodyType == PHOTO_TYPE) {
            PACK_PHOTO photo_pack;
            PACK_PHOTO_BACK photo_result;
            int res = read(fd, &photo_pack, sizeof(PACK_PHOTO));
            printf("EpollServer recv: photo\n");
            char fileName[64] = {0};
            sprintf(fileName, "./photo/%s", photo_pack.filename);

            FILE* fp;
            fp = fopen(fileName, "a+");
            fwrite(photo_pack.context, photo_pack.realSize, 1, fp);
            fclose(fp);

            photo_result.result = 1;

            char sendbuffer[sizeof(PACK_HEAD) + sizeof(PACK_PHOTO_BACK)] = {0};
            memcpy(sendbuffer, &head, sizeof(PACK_HEAD));
            memcpy(sendbuffer + sizeof(PACK_HEAD), &photo_result, sizeof(PACK_PHOTO_BACK));
            int n = write(fd, sendbuffer, sizeof(sendbuffer));
            if (photo_pack.num != photo_pack.sum) {
                m_epoller->ModFd(fd, m_connEvent | EPOLLIN);
                return ;
            }
        }
        // 车辆信息
		else if (head.bodyType == CAR_MSG_TYPE) {
			PACK_CARMSG car_msg;
			int res = read(fd, &car_msg, sizeof(PACK_CARMSG));
            printf("EpollServer recv: carmsg\n");

			char buffer[sizeof(PACK_HEAD) + sizeof(PACK_CARMSG)] = { 0 };
			memcpy(buffer, &head, sizeof(PACK_HEAD));
			memcpy(buffer + sizeof(PACK_HEAD), &car_msg, sizeof(PACK_CARMSG));

			if (res > 0) {
				shm_fifo1.shm_write(buffer);
				int res = msgsnd(m_msgid, &msg, msgsize, 0);
                printf("EpollServer: carmsg msgsnd\n");
            }
		}
        // 视频信息
		else if (head.bodyType == VIDEO_TYPE) {
			PACK_VIDEO video;
			int res = read(fd, &video, sizeof(PACK_VIDEO));
            printf("EpollServer recv: video\n");
			char buffer[sizeof(PACK_HEAD) + sizeof(PACK_VIDEO)] = { 0 };
			memcpy(buffer, &head, sizeof(PACK_HEAD));
			memcpy(buffer + sizeof(PACK_HEAD), &video, sizeof(PACK_VIDEO));

			if (res > 0) {
				shm_fifo1.shm_write(buffer);
				int res = msgsnd(m_msgid, &msg, msgsize, 0);
                printf("EpollServer: video msgsnd\n");
            }
		}
        // 打开视频
		else if (head.bodyType == VIDEO_OPEN) {
			PACK_VIDEO video;
			int res = read(fd, &video, sizeof(PACK_VIDEO));
            printf("EpollServer recv: open video\n");
			
			char buffer[sizeof(PACK_HEAD) + sizeof(PACK_VIDEO)] = { 0 };
			memcpy(buffer, &head, sizeof(PACK_HEAD));
			memcpy(buffer + sizeof(PACK_HEAD), &video, sizeof(PACK_VIDEO));

			if (res > 0) {
				shm_fifo1.shm_write(buffer);
				int res = msgsnd(m_msgid, &msg, msgsize, 0);
                printf("EpollServer: open video msgsnd\n");
            }
		}
        // 关闭视频
		else if (head.bodyType == VIDEO_CLOSE) {
			PACK_VIDEO video_close;
			int res = read(fd, &video_close, sizeof(PACK_VIDEO));
            printf("EpollServer recv: close video\n");
			char buffer[sizeof(PACK_HEAD) + sizeof(PACK_VIDEO)] = { 0 };
			memcpy(buffer, &head, sizeof(PACK_HEAD));
			memcpy(buffer + sizeof(PACK_HEAD), &video_close, sizeof(PACK_VIDEO));

			if (res > 0) {
				shm_fifo1.shm_write(buffer);
				int res = msgsnd(m_msgid, &msg, msgsize, 0);
                printf("EpollServer: close video msgsnd\n");
            }
		}
	}
	else {
		// 客户端断开连接
		printf("client error close\n");
		_Close_Conn(fd);
		return;
    }
    m_epoller->ModFd(fd, m_connEvent | EPOLLOUT);
}

void EpollServer::_Thread_Write(int fd) { 
	while (true) {
        size_t msgsize = sizeof(MYBUF) - sizeof(long); 
        MYBUF* msg = (MYBUF*)malloc(sizeof(MYBUF));
        int ret = msgrcv(m_msgid_back, msg, msgsize, 2, 0);
        if (ret == -1) {
            perror("msgrcv");
        }
        
        char rec_buffer[1024] = {0};
        memcpy(rec_buffer, shm_fifo2.shm_read(), sizeof(rec_buffer));

        PACK_HEAD* head = (PACK_HEAD*)malloc(sizeof(PACK_HEAD));
        memcpy(head, rec_buffer, sizeof(PACK_HEAD));
        // 注册
        if (head->bodyType == REGIST_TYPE) {
            char body_buffer[sizeof(PACK_RL_BACK)] = { 0 };
			memcpy(body_buffer, rec_buffer + sizeof(PACK_HEAD), sizeof(PACK_RL_BACK));

			char total_buffer[sizeof(PACK_HEAD) + sizeof(PACK_RL_BACK)] = { 0 };
			memcpy(total_buffer, head, sizeof(PACK_HEAD));
			memcpy(total_buffer + sizeof(PACK_HEAD), body_buffer, sizeof(body_buffer));
			int res = write(fd, total_buffer, sizeof(total_buffer));
            printf("EpollServer send: regist\n");
		}
        // 登录
		else if (head->bodyType == LOGIN_TYPE) {
			char body_buffer[sizeof(PACK_RL_BACK)] = { 0 };
			memcpy(body_buffer, rec_buffer + sizeof(PACK_HEAD), sizeof(PACK_RL_BACK));

			char total_buffer[sizeof(PACK_HEAD) + sizeof(PACK_RL_BACK)] = { 0 };
			memcpy(total_buffer, head, sizeof(PACK_HEAD));
			memcpy(total_buffer + sizeof(PACK_HEAD), body_buffer, sizeof(body_buffer));
			int res = write(fd, total_buffer, sizeof(total_buffer));
            printf("EpollServer send: login\n");
		}
        // 汽车入场
		else if (head->bodyType == CAR_GETIN) {
			char body_buffer[sizeof(PACK_RL_BACK)] = { 0 };
			memcpy(body_buffer, rec_buffer + sizeof(PACK_HEAD), sizeof(PACK_RL_BACK));

			char total_buffer[sizeof(PACK_HEAD) + sizeof(PACK_RL_BACK)] = { 0 };
			memcpy(total_buffer, head, sizeof(PACK_HEAD));
			memcpy(total_buffer + sizeof(PACK_HEAD), body_buffer, sizeof(body_buffer));
			int res = write(fd, total_buffer, sizeof(total_buffer));
            printf("EpollServer send: getin\n");
		}
        // 汽车出场
		else if (head->bodyType == CAR_GETOUT) {
			char body_buffer[sizeof(PACK_EXIT_BACK)] = { 0 };
			memcpy(body_buffer, rec_buffer + sizeof(PACK_HEAD), sizeof(PACK_EXIT_BACK));

			char total_buffer[sizeof(PACK_HEAD) + sizeof(PACK_EXIT_BACK)] = { 0 };
			memcpy(total_buffer, head, sizeof(PACK_HEAD));
			memcpy(total_buffer + sizeof(PACK_HEAD), body_buffer, sizeof(body_buffer));
			int res = write(fd, total_buffer, sizeof(total_buffer));
            printf("EpollServer send: getout\n");
		}
        // 汽车信息
		else if (head->bodyType == CAR_MSG_TYPE) {
			char body_buffer[sizeof(PACK_ALLCARMSG_BACK)] = { 0 };
			memcpy(body_buffer, rec_buffer + sizeof(PACK_HEAD), sizeof(PACK_ALLCARMSG_BACK));

			char total_buffer[sizeof(PACK_HEAD) + sizeof(PACK_ALLCARMSG_BACK)] = { 0 };
			memcpy(total_buffer, head, sizeof(PACK_HEAD));
			memcpy(total_buffer + sizeof(PACK_HEAD), body_buffer, sizeof(body_buffer));
			int res = write(fd, total_buffer, sizeof(total_buffer));
            printf("EpollServer send: carmsg\n");
		}
        // 打开视频
		else if (head->bodyType == VIDEO_OPEN) {
			char body_buffer[sizeof(PACK_VIDEO_BACK)] = { 0 };
			memcpy(body_buffer, rec_buffer + sizeof(PACK_HEAD), sizeof(PACK_VIDEO_BACK));

			char total_buffer[sizeof(PACK_HEAD) + sizeof(PACK_VIDEO_BACK)] = { 0 };
			memcpy(total_buffer, head, sizeof(PACK_HEAD));
			memcpy(total_buffer + sizeof(PACK_HEAD), body_buffer, sizeof(body_buffer));
			int res = write(fd, total_buffer, sizeof(total_buffer));
            printf("EpollServer send: open video\n");

		}
		free(msg); msg = NULL;
		free(head); head = NULL;
        m_epoller->ModFd(fd, m_connEvent | EPOLLIN);
    }
}

void EpollServer::_Close_Conn(int fd) {
    assert(fd > 0);
    m_epoller->DelFd(fd);
    close(fd);
}

