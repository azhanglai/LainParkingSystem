#include "./cardb.h"
#include "./define.h"

CarDB* CarDB::cardb;

CarDB* CarDB::getInstance() {
    if (CarDB::cardb == NULL) {
        CarDB::cardb = new CarDB();
    }
    return CarDB::cardb;
}

// 将汽车的入场信息插入到tbl_msg表中
int CarDB::insertCarMsg(string car_num, string in_time, string local, string path) {
    // 1. sql 语句拼接
    char sql[256];
    sprintf(sql, "insert into tbl_msg values(null, '%s', 'null', '%s', 'null', '%s', 'null', 0, 1, '%s', 0)", car_num.c_str(), in_time.c_str(), path.c_str(), local.c_str());

    // 2. 数据准备 
    char** qres;
    int row, col;
    
    // 3. 打开数据库, 执行语句 
    int res = SingalDb::getInstance()->dosql(sql, qres, row, col);
    if (res == 0) { return 0; }
    else { return -1; }
}

// 删除tbl_msg表
int CarDB::deletecar() {
    char sql[256];
    sprintf(sql, "delete from tbl_msg");

    char** qres;
    int row, col;

    int res = SingalDb::getInstance()->dosql(sql, qres, row, col);
    if (res == 0) { return 0; }
    else { return -1; }
}

// 根据车牌号查询汽车入场时间和是否是vip
list<string> CarDB::select_intime(string car_num) {
    char sql[256];
    sprintf(sql, "select goin_time, vip from tbl_msg where car_num='%s' and state=1", car_num.c_str());
    
    char** qres;
    int row, col;
    list<string> list;

    int res = SingalDb::getInstance()->dosql(sql, qres, row, col);
    if (res == 0) {
        if (row == 0) { return list; }
        else {
            // 第一行为字段，故需要从第二行开始
            // 将数据存到链表中
            for (int i = col; i < (row + 1) * col; i++) {
                list.push_back(qres[i]);
            }
            return list;        
        }
    }
}

// 根据车牌号更新汽车的部分信息（出场时用）
int CarDB::updateCarmsg(string car_num, string out_time, int money){
	char sql[256];
	sprintf(sql, "update tbl_msg set state=0,take_money=%d,goout_time='%s' where car_num='%s'",money, out_time.c_str(),car_num.c_str());

	char** qres;
	int row, col;
	list<string> list;

	int res = SingalDb::getInstance()->dosql(sql, qres, row, col);
	if (res == 0) { return 0; }
	else { return  -1; }
}

// 统计停车场内的汽车数量
int CarDB::totalCar() {
	char sql[256];
	sprintf(sql, "select COUNT(*) from tbl_msg where state=1");

	char** qres;
	int row, col;
	list<string> list;
	
	int res = SingalDb::getInstance()->dosql(sql, qres, row, col);
	if (res == 0) {
		if (row == 0) { return 0; }
		else { return atoi(qres[1]); }
	}
}

// 根据车牌号查询汽车信息，并将结果存到链表中
list<string> CarDB::select_carmsg(string car_num, int page) {
	char sql[256];
	sprintf(sql, "select car_num,goin_time,goout_time,take_money from tbl_msg where car_num='%s' ORDER BY carmsg_id ASC limit 4 OFFSET %d", car_num.c_str(),page);

	char** qres;
	int row, col;
	list<string> list;
	
	int res = SingalDb::getInstance()->dosql(sql, qres, row, col);
	if (res == 0) {
		if (row == 0) {
			return list;
		} else {
			for (int i = col; i < (row + 1) * col; i++) {
				list.push_back(qres[i]);
			}
			return list;
		}
	}
}

list<string> CarDB::select_allcarmsg(string car_num, string in_time, string out_time, int page) {
	
	char sql[256];
	sprintf(sql, "select car_num,goin_time,goout_time,take_money from tbl_msg where goin_time between '%s' and '%s' and car_num='%s' ORDER BY carmsg_id ASC limit 4 OFFSET %d", in_time.c_str(),out_time.c_str(),car_num.c_str(),page);

	char** qres;
	int row, col;
	list<string> list;

	int res = SingalDb::getInstance()->dosql(sql, qres, row, col);
	if (res == 0) {
		if (row == 0) {
			return list;
		} else {
			for (int i = col; i < (row + 1) * col; i++) {
				list.push_back(qres[i]);
			}
			return list;
		}
	}
}

// 根据入场时间和出场时间，查询汽车信息
list<string> CarDB::select_carmsg_time(string in_time, string out_time, int page) {
	char sql[256];
	sprintf(sql, "select car_num,goin_time,goout_time,take_money from tbl_msg where goin_time between '%s' and '%s' ORDER BY carmsg_id ASC limit 4 OFFSET %d", in_time.c_str(), out_time.c_str(), page);
	
	char** qres;
	int row, col;
	list<string> list;
	
	int res = SingalDb::getInstance()->dosql(sql, qres, row, col);
	if (res == 0) {
		if (row == 0) {
			return list;
		} else {
			for (int i = col; i < (row + 1) * col; i++) {
				list.push_back(qres[i]);
			}
			return list;
		}
	}
}

// 根据入场时间，查询汽车信息
list<string> CarDB::select_carmsg_intime(string in_time, int page) {
	char sql[256];
	sprintf(sql, "select car_num,goin_time,goout_time,take_money from tbl_msg where goin_time >= '%s' ORDER BY carmsg_id ASC limit 4 OFFSET %d", in_time.c_str(), page);

	char** qres;
	int row, col;
	list<string> list;

	int res = SingalDb::getInstance()->dosql(sql, qres, row, col);
	if (res == 0) {
		if (row == 0) {
			return list;
		} else {
			for (int i = col; i < (row + 1) * col; i++) {
				list.push_back(qres[i]);
			}
			return list;
		}
	}
}

// 根据出场时间， 查询汽车信息
list<string> CarDB::select_carmsg_outtime(string out_time, int page) {
	char sql[256];
	sprintf(sql, "select car_num,goin_time,goout_time,take_money from tbl_msg where goout_time <= '%s' ORDER BY carmsg_id ASC limit 4 OFFSET %d", out_time.c_str(), page);

	char** qres;
	int row, col;
	list<string> list;

	int res = SingalDb::getInstance()->dosql(sql, qres, row, col);
	if (res == 0) {
		if (row == 0) {
			return list;
		} else {
			for (int i = col; i < (row + 1) * col; i++) {
				list.push_back(qres[i]);
			}
			return list;
		}
	}
}

// 更新视频记录帧
int CarDB::updateVideoFrame(string name, int count) {
	char sql[256];
	sprintf(sql, "update tbl_record set record_nowframes=%d where record_name='%s'",count,name.c_str());

	char** qres;
	int row, col;

	int res = SingalDb::getInstance()->dosql(sql, qres, row, col);
	if (res == 0) {
		return 0;
	} else {
		return  -1;
	}
}

// 根据名字查询视频记录
int CarDB::selectVideoFrame(string name) {
	char sql[256];
	sprintf(sql, "select record_nowframes from tbl_record where record_name='%s'",name.c_str());

	char** qres;
	int row, col;

	int res = SingalDb::getInstance()->dosql(sql, qres, row, col);
	if (res == 0) {
		if (row == 0) {
			return 0;
		} else {
			return atoi(qres[1]);
		}
	}
}

// 插入视频记录帧到 tbl_record表中
int CarDB::insertNewVideo(string name, int totalframe) {
	char sql[256];
	sprintf(sql, "insert into tbl_record values(null,'%s',0,%d)",
		name.c_str(), totalframe);
	
	char** qres;
	int row, col;

	int res = SingalDb::getInstance()->dosql(sql, qres, row, col);
	if (res == 0) {
		return 0;
	} else {
		return  -1;
	}
}

