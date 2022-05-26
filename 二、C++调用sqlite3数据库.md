[TOC]



### 1、C/C++调用sqlite3接口介绍

- sqlite3的C/C++接口用法可分为两种：回调形式与非回调形式。
- 回调形式：是通过回调的方式处理sql语句执行结果。
- 非回调形式：是待sql语句执行完毕后再通过返回值和相关函数来判断、获取执行结果。

#### 1.1 sqlite3非回调形式接口用法

##### 1.1.1 sqlite3_open()函数简介

~~~c++
// 功能：打开sqlite3数据库的连接
// 函数原型：int sqlite3_open(const char *filename, sqlite3 **ppDb);
// 参数 filename：要打开的sqlite3数据库的路径名
// 参数 ppDB：二级指针，用来保存打开的数据库的连接对象
// 返回值：成功返回SQLITE_OK,失败返回其他值。

~~~

##### 1.1.2  sqlite3_close() 函数简介

~~~c++
// 功能：关闭数据库连接对象
// 函数原型int sqlite3_close(sqlite3*);
~~~

##### 1.1.3  sqlite3_prepare_v2()函数简介

~~~c++
// 功能：编译SQL语句，并创建一个SQL语句对象
int sqlite3_prepare_v2(
	sqlite3 *db,           // 数据库连接对象
    const char *zSql,      // 指向原始sql语句(要编译的sql语句)，可以包含变量名的
    int nByte,             // < 0: 编译zSql到第一个\0为止
                           // >0:  编译zSql中前面nByte个字符
                           // =0:  什么也不编译
	sqlite3_stmt **ppStmt, // *ppStmt用来保存编译好的sql语句对象
    const char **pzTail    // *pzTail如果不为空，则*pzTail指向zSql中编译的第一条完整sql语句后面的第一个字符。                 
    );
// 返回值:成功返回SQLITE_OK,失败返回其他值.
~~~

##### 1.1.4  sqlite3_step()函数简介

~~~c++
// 功能：用来执行编译好的SQL语句对象(由sqlite3_stmt指定)，每次返回一行执行结果。
// 函数原型：int sqlite3_step(sqlite3_stmt*);
// 参数 sqlite3_stmt: 指向编译好的要执行的SQL语句对象
// 返回值：SQLITE_BUSY: 没获取到锁，语句没执行。
//        SQLITE_DONE: sql语句执行完成。
//        SQLITE_ERROR: 出错啦
//        SQLITE_MISUSE: 使用方法不当
//        SQLITE_ROW:  假如SQL语句执行有返回结果，SQLITE_ROW返回。
~~~

##### 1.1.5 sqlite3_get_table()函数简介

~~~c++
// 功能：直接获取整个结果表
int sqlite3_get_table(
  	sqlite3 *db,          /* An open database */
  	const char *zSql,     /* SQL to be evaluated */
  	char ***pazResult,    /* Results of the query */
  	int *pnRow,           /* Number of result rows written here */
  	int *pnColumn,        /* Number of result columns written here */
  	char **pzErrmsg       /* Error msg written here */
);
// 第1个参数：数据库连接对象。
// 第2个参数：是sql 语句，跟sqlite3_exec 里的sql 是一样的。是一个很普通的以\0结尾的char*字符串。
// 第3个参数：是查询结果（可理解为二维数组的地址）。
// 第4个参数：是查询出多少条记录（即查出多少行，不包括字段名那行）。
// 第5个参数：是多少个字段（多少列）。
// 第6个参数：是错误信息
// pazResult返回的字符串数量实际上是(*pnRow+1)*(*pnColumn),因为前(*pnColumn)个是字段名
~~~

##### 1.1.6 sqlite3_reset()函数简介

~~~c++
// 功能：用来复位sql语句对象，以便下一轮的参数赋值
// 函数原型：int sqlite3_reset(sqlite3_stmt *pStmt);
~~~

##### 1.1.7 sqlite3_finalize()函数简介

~~~c++
// 功能：用来销毁一个SQL语句对象，与sqlite3_prepare_v2创建的SQL语句对象对应
// 函数原型：int sqlite3_finalize(sqlite3_stmt *pStmt);
~~~

#### 1.2 sqlite3回调形式接口用法

##### 1.2.1 sqlite3_exec()函数简介

~~~c++
// 功能：是sqlite3_prepare_v2、sqlite3_step、sqlite3_finalize 三个函数的组合

int sqlite3_exec( sqlite3* , 			//指向数据库连接对象
                  const char *sql , 	//指向要执行的SQL语句,一般不带参数。
                  int (*callback) (void *, int, char **, char **), //回调函数
                  void *arg, 			//这个函数将作为callback的第一个参数传入
                  char **errmsg 		//用来保存出错的信息        
                );
// 第一个参数：数据库连接对象
// 第二个参数：要执行的sql语句，可以执行多条语句以;分开
// 第三个参数，函数指针，回调函数。一般在sql语句为select语句时，需要回调。每查询到一条结果时(一行),就调用该回调函数。
int (*callback)(  void *,     	// sqlite3_exec的第四个参数
                  int,    		// 结果中有多少列 
                  char **, 		// char *column_values[],这一行每列的值(转化成字符串)
                  char **  		// char *column_names[],这一行每列的名字(字符串)
           ), 
// 回调函数的返回值: 返回0表示成功，其他值表示失败，回调函数执行失败了(返回非0值),sqlite3_exec就不执行下面的语句了。
// 第四个参数: 将作为回调函数的第一个参数
// 第五个参数: *errmsg将保存执行过程中的错误信息

// 返回值:  成功返回0， 失败返回其他值。
~~~

### 2、实现C++调用sqlite3类

#### 2.1 singaldb.h 头文件

~~~c++
#ifndef __SINGALDB_H
#define __SINGALDB_H

#include <sqlite3.h>

class SingalDb {
public:
    static SingalDb* getInstance(const char* dbPath);
    
    void openDataBase(const char* dbPath);
    void closeDataBase();
    int dosql(char* sql, char**& result, int& row, int& col);

private:
    SingalDb() = default;
    SingalDb(const char* dbPath);

    static SingalDb* mydb;
    sqlite3* sqldb;
    char* dbPath;
    char* errmsg;
};

#endif /* __SINGALDB_H */

~~~

#### 2.2  singaldb.cpp 源文件

~~~c++
#ifndef __SINGALDB_H
#define __SINGALDB_H

#include <sqlite3.h>

class SingalDb {
public:
    static SingalDb* getInstance(const char* dbPath);
    
    void openDataBase(const char* dbPath);
    void closeDataBase();
    int dosql(char* sql, char**& result, int& row, int& col);

private:
    SingalDb() = default;
    SingalDb(const char* dbPath);

    static SingalDb* mydb;
    sqlite3* sqldb;
    char* dbPath;
    char* errmsg;
};


#endif /* __SINGALDB_H */

lai@lai-VirtualBox:~/myProject/Test$ cat singaldb.cpp 
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
  	// sqlite3_open: 打开sqlite3数据库的连接
    int res = sqlite3_open(dbPath, &sqldb);
    if (res != SQLITE_OK) {
        std::cout << sqlite3_errmsg(sqldb) << std::endl;
        std::cout << sqlite3_errcode(sqldb) << std::endl;
    }
}

void SingalDb::closeDataBase() {
    // sqlite3_close:关闭数据库连接对象
    int res = sqlite3_close(sqldb);
    if (res != SQLITE_OK) {
        std::cout << sqlite3_errmsg(sqldb) << std::endl;
        std::cout << sqlite3_errcode(sqldb) << std::endl;
    }
}

int SingalDb::dosql(char* sql, char**& result, int& row, int& col) {
    // sqlite3_get_table:执行sql, 直接获取整个结果表
    int res = sqlite3_get_table(sqldb, sql, &result, &row, &col, &errmsg);
    if (res != SQLITE_OK) {
        std::cout << sqlite3_errmsg(sqldb) << std::endl;
        std::cout << sqlite3_errcode(sqldb) << std::endl;
        return sqlite3_errcode(sqldb);
    }
    return 0;
}

~~~

### 3、测试

#### 3.1 创建一个数据库 test.db

![img](https://s2.loli.net/2022/05/18/hrLGEo3WPqvk6IO.png)

#### 3.2 在数据库中创建一个表 tbl_test

![img](https://s2.loli.net/2022/05/18/gSlz3qNIaetU79K.png)

#### 3.3 编写主函数 sqltest.cpp

~~~c++
#include "./singaldb.h"
#include <cstdio>
#include <iostream>
using namespace std;
int main() {
    int ret;
    char sql[256];
    char** qres;
    int row, col;

    sprintf(sql, "insert into tbl_test values('%d','%s', '%s', '%d')",
           1, "zhanglai", "男", 18);
    
    const char* dbPath = "test.db";
    ret = SingalDb::getInstance(dbPath)->dosql(sql, qres, row, col);
    if (ret == 0) {
        cout << "OK\n";
    } else {
        cout << "Bad\n";
    }

    return 0;
}
~~~

#### 3.4 编写编译工具 Makefile文件

~~~makefile
.PHONY: all clean

CC := g++
CFLAGS := -std=c++11
LIB := -lsqlite3

all: testsql.o singaldb.o
	${CC} ${CFLAGS} $^ -o testsql ${LIB}

testsql.o: testsql.cpp
	${CC} ${CFLAGS} -c $< -o $@ ${LIB}

singaldb.o: singaldb.cpp
	${CC} ${CFLAGS} -c $< -o $@ ${LIB}

clean:
	rm -rf ./*.o ./testsql 
~~~

#### 3.5 结果图片

![img](https://s2.loli.net/2022/05/18/BtXIPrp6hyG8i4s.png)