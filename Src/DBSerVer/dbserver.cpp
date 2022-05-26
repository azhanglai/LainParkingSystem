#include "./dbserver.h"

ShmFifo shm_fifo1(11, 10, 1024);
ShmFifo shm_fifo2(22, 10, 1024);

DBServer::DBServer() : m_isClose(false) {
    m_msgid = msgget(1234, IPC_CREAT | IPC_EXCL | 0666);
    if (m_msgid == -1) {
        m_msgid = msgget(1234, 0);
    }

    m_msgid_back = msgget(12345, IPC_CREAT | IPC_EXCL | 0666);
    if (m_msgid_back == -1) {
        m_msgid_back = msgget(12345, 0);
    }
}

void DBServer::Start() {
    if (!m_isClose) {
        printf("========== DBServer Start ==========\n");
    }
    
	while (!m_isClose) {
        size_t msgsize = sizeof(MYBUF) - sizeof(long);
        MYBUF* msg = (MYBUF*)malloc(sizeof(MYBUF));

        int ret = msgrcv(m_msgid, msg, msgsize, 1, 0);
        if (ret == -1) {
            perror("msgrcv");
        }

        char buffer[1024] = {0};
        memcpy(buffer, shm_fifo1.shm_read(), sizeof(buffer)); 
        PACK_HEAD* head = (PACK_HEAD*)malloc(sizeof(buffer));
        memcpy(head, buffer, sizeof(PACK_HEAD));

        // 注册
        if (head->bodyType == REGIST_TYPE) {
            PACK_REGIST_LOGIN* regist_login = (PACK_REGIST_LOGIN*)malloc(sizeof(PACK_REGIST_LOGIN));
            memcpy(regist_login, buffer + sizeof(PACK_HEAD), sizeof(PACK_REGIST_LOGIN));

            int res = UserDB::getInstance()->insertUser(regist_login->name, regist_login->pwd);

            PACK_RL_BACK result;
            if (res == 0) {
                result.result = 0;
            } else if (res == -1) {
                result.result = 1;
            }

            MYBUF msg_back;
            msg_back.mtype = 2;

            char send_buffer[sizeof(PACK_HEAD) + sizeof(PACK_RL_BACK)] = {0};
            
            memcpy(send_buffer, head, sizeof(PACK_HEAD));
            memcpy(send_buffer + sizeof(PACK_HEAD), &result, sizeof(PACK_RL_BACK));
            shm_fifo2.shm_write(send_buffer);
            int n = msgsnd(m_msgid_back, &msg_back, msgsize, 0);
            printf("DBServer: regist msgsnd\n");
        }
        // 登录
        else if (head->bodyType == LOGIN_TYPE) {
       		PACK_REGIST_LOGIN* regist_login = (PACK_REGIST_LOGIN*)malloc(sizeof(PACK_REGIST_LOGIN));
			memcpy(regist_login, buffer + sizeof(PACK_HEAD), sizeof(PACK_REGIST_LOGIN));

			int res = UserDB::getInstance()->searchUser(regist_login->name, regist_login->pwd);
			
			PACK_RL_BACK result;
			if (res == 0) {
				result.result = CarDB::getInstance()->totalCar(); 
			} else {
				result.result = -1; 
			}

			MYBUF msg_back;
			msg_back.mtype = 2;
			char send_buffer[sizeof(PACK_HEAD) + sizeof(PACK_RL_BACK)] = { 0 };
			memcpy(send_buffer, head, sizeof(PACK_HEAD));
			memcpy(send_buffer + sizeof(PACK_HEAD), &result, sizeof(PACK_RL_BACK));

			shm_fifo2.shm_write(send_buffer);
			int n = msgsnd(m_msgid_back, &msg_back, msgsize, 0);   
            printf("DBServer: login msgsnd\n");
        }
		// 车辆入场
		else if (head->bodyType == CAR_GETIN) {
			PACK_ENTER* car_enter = (PACK_ENTER*)malloc(sizeof(PACK_ENTER));
			memcpy(car_enter, buffer + sizeof(PACK_HEAD), sizeof(PACK_ENTER));

			int res = CarDB::getInstance()->insertCarMsg(car_enter->car_num, car_enter->now_time, car_enter->location,car_enter->photo_path);
			
			PACK_RL_BACK result;
			memset(&result, 0x0, sizeof(PACK_RL_BACK));
			if (res == 0) {
				result.result = 0;
			} else {
				result.result = 1;
			}

			MYBUF msg_back;
			msg_back.mtype = 2;

			char send_buffer[sizeof(PACK_HEAD) + sizeof(PACK_RL_BACK)] = { 0 };
			memcpy(send_buffer, head, sizeof(PACK_HEAD));
			memcpy(send_buffer + sizeof(PACK_HEAD), &result, sizeof(PACK_RL_BACK));

			shm_fifo2.shm_write(send_buffer);
			int n = msgsnd(m_msgid_back, &msg_back, msgsize, 0);	
            printf("DBServer: getin msgsnd\n");
		}
		// 车辆出场
		else if (head->bodyType == CAR_GETOUT) {
			PACK_EXIT* car_exit = (PACK_EXIT*)malloc(sizeof(PACK_EXIT));
			memcpy(car_exit, buffer + sizeof(PACK_HEAD), sizeof(PACK_EXIT));
			
			MYBUF msg_back;
			msg_back.mtype = 2;

			list<string> listtemp;
			listtemp = CarDB::getInstance()->select_intime(car_exit->car_num);
			PACK_EXIT_BACK car_exit_back;
			memset(&car_exit_back, 0x0, sizeof(PACK_EXIT_BACK));
			if (listtemp.size() == 0){
				car_exit_back.vip = -1;
			} else {
				string in_time = listtemp.front();
				int vip = stoi(listtemp.back());
				int in_year, in_month, in_day, in_hour, in_min, in_sec;
				int out_year, out_month, out_day, out_hour, out_min, out_sec;
				sscanf(in_time.c_str(), "%d-%2d-%2d %2d:%2d:%2d", &in_year, &in_month, &in_day, &in_hour, &in_min, &in_sec);

				sscanf(car_exit->out_time, "%d-%2d-%2d %2d:%2d:%2d", &out_year, &out_month, &out_day, &out_hour, &out_min, &out_sec);

				int year, month, day, hour, min, sec;
                year = out_year - in_year;
				month = out_month - in_month;
				day = out_day - in_day;
				hour = out_hour - in_hour;

				if (hour <= 0) {
					hour *= (-1);
				}	
				min = out_min - in_min;
				if (min < 0) {
					min *= (-1);
				}
				sec = out_sec - in_sec;
				if (sec < 0) {
					sec *= (-1);
				}	

				char stay_time[64] = { 0 };
				sprintf(stay_time,"%d-%d-%d %d:%d:%d",year,month,day,hour,min,sec);

				if (vip == 1) {
					car_exit_back.money = 0;
				}
				else {
					if (hour == 0) {
						car_exit_back.money = 10;
					} else {
						car_exit_back.money = hour * 10;
					}
				}
				strcpy(car_exit_back.total_time, stay_time);
				strcpy(car_exit_back.in_time, listtemp.front().c_str());

                printf("in_time: %s\n", car_exit_back.in_time);
                printf("stay_time: %s\n", car_exit_back.total_time);
                printf("money: %d\n", car_exit_back.money);

				int updatemsg = CarDB::getInstance()->updateCarmsg(car_exit->car_num, car_exit->out_time,car_exit_back.money);
			}

			char send_buffer[sizeof(PACK_HEAD) + sizeof(PACK_EXIT_BACK)] = { 0 };
			memcpy(send_buffer, head, sizeof(PACK_HEAD));
			memcpy(send_buffer + sizeof(PACK_HEAD), &car_exit_back, sizeof(PACK_EXIT_BACK));

			shm_fifo2.shm_write(send_buffer);
			int n = msgsnd(m_msgid_back, &msg_back, msgsize, 0);	
            printf("DBServer: getout msgsnd\n");
		}
        // 车辆信息
        else if (head->bodyType == CAR_MSG_TYPE) {
            PACK_CARMSG* car_msg = (PACK_CARMSG*)malloc(sizeof(PACK_CARMSG));
			memcpy(car_msg, buffer + sizeof(PACK_HEAD), sizeof(PACK_CARMSG));

			list<string> car_msglist;
			car_msglist.clear();
			if (car_msg->car_num != "" && car_msg->in_time!= "" &&car_msg->out_time!="")
			{
				car_msglist = CarDB::getInstance()->select_allcarmsg(car_msg->car_num, car_msg->in_time, car_msg->out_time, car_msg->page);
			}
			else if(car_msg->car_num == "" && car_msg->in_time != "" && car_msg->out_time != "")
			{
				car_msglist = CarDB::getInstance()->select_carmsg_time(car_msg->in_time, car_msg->out_time, car_msg->page);
			}
			else if (car_msg->car_num != "" && car_msg->in_time == "" && car_msg->out_time == "")
			{
				car_msglist = CarDB::getInstance()->select_carmsg(car_msg->car_num, car_msg->page);
			}
			else if (car_msg->car_num == "" && car_msg->in_time != "" && car_msg->out_time == "")
			{
				car_msglist = CarDB::getInstance()->select_carmsg_intime(car_msg->in_time, car_msg->page);
			}
			else if (car_msg->car_num == "" && car_msg->in_time == "" && car_msg->out_time != "")
			{
				car_msglist = CarDB::getInstance()->select_carmsg_outtime(car_msg->out_time, car_msg->page);
			}
			else if (car_msg->car_num == "" && car_msg->in_time == "" && car_msg->out_time == ""){}

			int i = 0;
			PACK_ALLCARMSG_BACK carmsg_allback;
			memset(&carmsg_allback, 0x0, sizeof(PACK_ALLCARMSG_BACK));

			list<string>::iterator car_it;
			for (car_it = car_msglist.begin(); car_it != car_msglist.end(); ) {
				strcpy(carmsg_allback.arr[i].car_num, (*car_it++).c_str());
				strcpy(carmsg_allback.arr[i].in_time, (*car_it++).c_str());
				strcpy(carmsg_allback.arr[i].out_time, (*car_it++).c_str());
				carmsg_allback.arr[i].money = stoi(*car_it++);
				i++;
			}
			carmsg_allback.realCount = car_msglist.size() / 4;

			MYBUF msg_back;
			msg_back.mtype = 2;

			char send_buffer[sizeof(PACK_HEAD) + sizeof(PACK_ALLCARMSG_BACK)] = { 0 };
			memcpy(send_buffer, head, sizeof(PACK_HEAD));
			memcpy(send_buffer + sizeof(PACK_HEAD), &carmsg_allback, sizeof(PACK_ALLCARMSG_BACK));

			shm_fifo2.shm_write(send_buffer);
			int n = msgsnd(m_msgid_back, &msg_back, msgsize, 0);
		}
        // 视频信息
		else if (head->bodyType == VIDEO_TYPE) {
			PACK_VIDEO* video = (PACK_VIDEO*)malloc(sizeof(PACK_VIDEO));
			memcpy(video, buffer + sizeof(PACK_HEAD), sizeof(PACK_VIDEO));

			int res = CarDB::getInstance()->insertNewVideo(video->video_name, video->totalFrameCount);
			if (res == 0) {
				//cout << "success" << endl;
			}
		}
        // 打开视频
		else if (head->bodyType == VIDEO_OPEN) {
			PACK_VIDEO* video = (PACK_VIDEO*)malloc(sizeof(PACK_VIDEO));
			memcpy(video, buffer + sizeof(PACK_HEAD), sizeof(PACK_VIDEO));

			int frame = CarDB::getInstance()->selectVideoFrame(video->video_name);
			PACK_VIDEO_BACK video_back;
			video_back.frameCount = frame;

			MYBUF msg_back;
			msg_back.mtype = 2;

			char send_buffer[sizeof(PACK_HEAD) + sizeof(PACK_VIDEO_BACK)] = { 0 };
			memcpy(send_buffer, head, sizeof(PACK_HEAD));
			memcpy(send_buffer + sizeof(PACK_HEAD), &video_back, sizeof(PACK_VIDEO_BACK));

			shm_fifo2.shm_write(send_buffer);
			int res = msgsnd(m_msgid_back, &msg_back, msgsize, 0);
        }
        // 关闭视频
		else if (head->bodyType == VIDEO_CLOSE) {
			PACK_VIDEO* video_close = (PACK_VIDEO*)malloc(sizeof(PACK_VIDEO));
			memcpy(video_close, buffer + sizeof(PACK_HEAD), sizeof(PACK_VIDEO));
			int res = CarDB::getInstance()->updateVideoFrame(video_close->video_name, video_close->totalFrameCount);
			if (res == 0) {
				//cout << "success" << endl;
			}
        }
    }
}

