#include "userdb.h"
#include "./define.h"

UserDB* UserDB::userdb;

UserDB* UserDB::getInstance() {
    if (UserDB::userdb == NULL) {
        UserDB::userdb = new UserDB();
    }
    return UserDB::userdb;
}

int UserDB::insertUser(string name, string pwd) {
    // 1. sql语句拼接
    char sql[256];
    sprintf(sql, "insert into tbl_user values(null, '%s', '%s', 1)", name.c_str(), pwd.c_str());

    // 2. 数据准备
    char** qres;
    int row, col;

    // 3. 打开数据库
    int res = SingalDb::getInstance()->dosql(sql, qres, row, col);
    if (res == 0) { return 0; }
    else { return -1; }
} 

int UserDB::searchUser(string name, string pwd) {
    // 1. sql语句拼接 
    char sql[256];
    sprintf(sql, "select * from tbl_user where user_name='%s' and user_pwd='%s'", name.c_str(), pwd.c_str());

    // 2. 数据准备 
    char** qres;
    int row, col;
    // 3. 打开数据库 
    int res = SingalDb::getInstance()->dosql(sql, qres, row, col);
    if (res == 0) {
        if (row > 0) { return 0; }
        else { return 1;}
    } else { return -1; }
}

int UserDB::changeState(int state, string name) {
    // 1. sql语句拼接
    char sql[256];
    sprintf(sql, "update tbl_user set state=%d where user_name='%s'", state, name.c_str());

    // 2. 数据准备 
    char** qres;
    int row, col;

    // 3. 打开数据库 
    int res = SingalDb::getInstance()->dosql(sql, qres, row, col);
    if (res == 0) { return 0; }
    else { return -1; }
}

