#ifndef _DBSERVER_H
#define _DBSERVER_H

#include "./define.h"
#include "./pakeage.h"
#include "./shmfifo.h"
#include "./userdb.h"
#include "./cardb.h"

class DBServer {
public:
    DBServer();
    ~DBServer() = default;

    void Start();

private:
    int m_msgid;
    int m_msgid_back;
    
    bool m_isClose;
};

#endif /* _DBSERVER_H */

