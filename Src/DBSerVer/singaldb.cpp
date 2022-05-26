#include "./singaldb.h"
#include <iostream>

SingalDb* SingalDb::mydb;

SingalDb::SingalDb(const char* dbPath) {
    this->openDataBase(dbPath);
}

SingalDb* SingalDb::getInstance(const char* dbPath) {
    if (SingalDb::mydb == NULL) {
        SingalDb::mydb = new SingalDb(dbPath);
    }
    return SingalDb::mydb;
}

void SingalDb::openDataBase(const char* dbPath) {
    int res = sqlite3_open(dbPath, &sqldb);
    if (res != SQLITE_OK) {
        std::cout << sqlite3_errmsg(sqldb) << std::endl;
        std::cout << sqlite3_errcode(sqldb) << std::endl;
    }
}

void SingalDb::closeDataBase() {
    int res = sqlite3_close(sqldb);
    if (res != SQLITE_OK) {
        std::cout << sqlite3_errmsg(sqldb) << std::endl;
        std::cout << sqlite3_errcode(sqldb) << std::endl;
    }
}

int SingalDb::dosql(char* sql, char**& result, int& row, int& col) {
    int res = sqlite3_get_table(sqldb, sql, &result, &row, &col, &errmsg);
    if (res != SQLITE_OK) {
        std::cout << sqlite3_errmsg(sqldb) << std::endl;
        std::cout << sqlite3_errcode(sqldb) << std::endl;
        return sqlite3_errcode(sqldb);
    }
    return 0;
}

