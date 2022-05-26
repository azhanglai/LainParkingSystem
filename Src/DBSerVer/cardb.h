#ifndef __CARDB_H
#define __CARDB_H

#include <string>
#include <iostream>
#include <list>
#include <vector>
#include "./singaldb.h"

using namespace std;

class CarDB {
public:
    list<string> select_intime(string car_num);
    list<string> select_carmsg(string car_num, int page);
    list<string> select_allcarmsg(string car_num, string in_time, string out_time, int page);
    list<string> select_carmsg_time(string in_time, string out_time, int page);
    list<string> select_carmsg_intime(string in_time, int page);
    list<string> select_carmsg_outtime(string out_time, int page);

    static CarDB* getInstance();

    int insertCarMsg(string car_num, string in_time, string local, string path);
    int deletecar();
    int updateCarmsg(string car_num, string out_time, int money);
    int totalCar();
    int selectVideoFrame(string name);
    int updateVideoFrame(string name, int count);
    int insertNewVideo(string name, int totalframe);

private:
    CarDB() = default;
    static CarDB* cardb;
};

#endif /* __CARDB_H */

