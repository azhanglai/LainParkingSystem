#ifndef __USERDB_H
#define __USERDB_H

#include <iostream>
#include <string>
#include <list>
#include "./singaldb.h"

using namespace std;

class UserDB {
public:
    static UserDB* getInstance();
    int insertUser(string name, string pwd);
    int searchUser(string name, string pwd);
    int changeState(int state, string name);

private:
    UserDB() = default;

    static UserDB* userdb;

};

#endif /* __USERDB_H */

