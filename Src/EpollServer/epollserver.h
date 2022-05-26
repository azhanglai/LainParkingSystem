#ifndef _EPOLLSERVER_H
#define _EPOLLSERVER_H

#include "./define.h"
#include "./pakeage.h"
#include "./shmfifo.h"
#include "./epoller.h"
#include "./threadpool.h" 
#include <memory>
#include <map>
using namespace std;

class EpollServer {
public:
    EpollServer(int port, bool OptLinger, int trigMode, int threadNum); 
    ~EpollServer();
    void Start();							

private:
    int m_port;								
    bool m_openLinger;						
    bool m_isClose;							
    int m_listenFd;
    
    int m_msgid;
    int m_msgid_back;
    
    uint32_t m_listenEvent; 				
    uint32_t m_connEvent;					
    
    static map<int, PACK_HEART> conntimer;
    static mutex mtx;
    
    unique_ptr<Epoller> m_epoller;		
    unique_ptr<ThreadPool> m_threadpool;		

    bool _Init_Socket();					
    void _Init_EventMode(int trigMode);		
    static int Set_FdNonblock(int fd);		
    
    void _Deal_Listen();
    void _Deal_Read(int fd);				
    void _Deal_Write(int fd);				
    
	void _Thread_Read(int fd); 		
    void _Thread_Write(int fd); 			
    void _Thread_Heart();
    
    void _Close_Conn(int fd);
};

#endif /* _EPOLLSERVER_H */

