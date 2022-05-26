#include "singledb_sqlite.h"
#include <QDebug>

Singledb_Sqlite *Singledb_Sqlite::myDB = nullptr;

Singledb_Sqlite::Singledb_Sqlite() {
    this->openDataBase("../LaiClient/data/client.db");
}

Singledb_Sqlite *Singledb_Sqlite::getInstance() {
    if(Singledb_Sqlite::myDB == nullptr) {
        Singledb_Sqlite::myDB = new Singledb_Sqlite();
    }
    return Singledb_Sqlite::myDB;
}

// 数据库执行 （成功返回0）
int Singledb_Sqlite::dosql(char *sql,char **&result,int &row, int &col) {
    int res = sqlite3_get_table(sqldb, sql, &result, &row, &col, &errmsg);
    if (res != SQLITE_OK) {
        return res;
    }
    return 0;
}

// 打开数据库
void Singledb_Sqlite::openDataBase(const char *dbPath) {
    int res = sqlite3_open(dbPath, &sqldb);
    if (res != SQLITE_OK) {
        qDebug() << sqlite3_errmsg(sqldb) << "\n";
        qDebug() << sqlite3_errcode(sqldb) << "\n";
    }
}

// 关闭数据库
void Singledb_Sqlite::closeDataBase() {
    int res = sqlite3_close(sqldb);
    if (res != SQLITE_OK) {
        qDebug() << sqlite3_errmsg(sqldb) << "\n";
        qDebug() << sqlite3_errcode(sqldb) << "\n";
    }
}
