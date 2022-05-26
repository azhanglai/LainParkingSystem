#ifndef SINGLEDB_SQLITE_H
#define SINGLEDB_SQLITE_H

#include <sqlite3.h>

class Singledb_Sqlite {
public:
    ~Singledb_Sqlite();

    static Singledb_Sqlite *getInstance();
    // 数据库执行
    int dosql(char *sql,char **&result,int &row, int &col);
    // 打开数据库
    void openDataBase(const char *dbPath);
    // 关闭数据库
    void closeDataBase();
private:
    Singledb_Sqlite();              // 构造私有化
    static Singledb_Sqlite *myDB;   // 静态私成员变量
    sqlite3 *sqldb;
    char *errmsg;                   // 用来存储错误信息字符串
};

#endif // SINGLEDB_SQLITE_H
