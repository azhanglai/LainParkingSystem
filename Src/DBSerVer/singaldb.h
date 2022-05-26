#ifndef __SINGALDB_H
#define __SINGALDB_H

#include <sqlite3.h>

class SingalDb {
public:
    static SingalDb* getInstance(const char* dbPath = "root.db");
    
    void openDataBase(const char* dbPath);
    void closeDataBase();
    int dosql(char* sql, char**& result, int& row, int& col);

private:
    SingalDb(const char* dbPath);

    static SingalDb* mydb;
    sqlite3* sqldb;
    char* dbPath;
    char* errmsg;
};


#endif /* __SINGALDB_H */

