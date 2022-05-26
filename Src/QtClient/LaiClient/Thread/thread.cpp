#include "thread.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <QDebug>

#include "Tool/singledb.h"
#include <QDebug>

Thread* Thread::myThread = nullptr;

Thread::Thread() {
    //定时器：每两分钟发一次心跳包
    heartbeat = new QTimer();
    connect(heartbeat, &QTimer::timeout, this, &Thread::send_heartbeat);
    heartbeat->start(120000);
}

Thread* Thread::getInstance() {
    if(Thread::myThread==nullptr) {
        Thread::myThread=new Thread();
    }
    return Thread::myThread;
}

void Thread::send_heartbeat() {
    // 发到服务器
    PACK_HEAD head;
    char buffer[sizeof(PACK_HEAD)] = { 0 };
    memset(&head, 0, sizeof(PACK_HEAD));
    head.bodyType = HEART_TYPE; // 类型是心跳包

    memcpy(buffer, &head, sizeof(PACK_HEAD));
    write(SingleDB::getInstance()->get_fd(), buffer, sizeof(PACK_HEAD));
    qDebug() << "send HEART_TYPE\n";
}

void Thread::run() {
    // 1.定义包头
    PACK_HEAD head;
    memset(&head, 0, sizeof(PACK_HEAD));
    while (true) {
        // 读服务器发来的包头，没有就阻塞
        read(SingleDB::getInstance()->get_fd(), &head, sizeof(PACK_HEAD));
        // 登录包
        if (head.bodyType == LOGIN) {
            REGIST_LOGIN_RESULT result;
            // 读服务器发来的包体
            read(SingleDB::getInstance()->get_fd(), &result, sizeof(REGIST_LOGIN_RESULT));
            qDebug() << "fd:" << SingleDB::getInstance()->get_fd() << " client recv: login\n";
            // 根据结果判断登录是否成功，发送成功或者失败信号
            if(result.result == -1) {
                emit Login(false);
            } else {
                // 登录成功后会返回数据库没出车库的车数量
                car_amount = result.result;
                emit Login(true);
            }
            //清空
            memset(&result, 0x0, sizeof(REGIST_LOGIN_RESULT));
        }
        // 注册包
        else if (head.bodyType == REGISTER) {
            REGIST_LOGIN_RESULT result;
            read(SingleDB::getInstance()->get_fd(), &result, sizeof(REGIST_LOGIN_RESULT));
            qDebug() << "fd:" << SingleDB::getInstance()->get_fd() << " client recv: regist\n";
            // 根据结果判断注册是否成功，发送成功或者失败信号
            if(result.result == -1) {
                emit Register(false);
            }
            else {
                emit Register(true);
            }
            memset(&result, 0x0, sizeof(REGIST_LOGIN_RESULT));
        }
        // 出场包
        else if (head.bodyType == CAR_GETOUT) {
            PACK_EXIT_BACK result;
            read(SingleDB::getInstance()->get_fd(), &result, sizeof(PACK_EXIT_BACK));
            qDebug() << "fd:" << SingleDB::getInstance()->get_fd() << " client recv: getout\n";
            // 发送带出场包的信号
            emit out(result);
            memset(&result, 0x0, sizeof(PACK_EXIT_BACK));
        }
        // 车辆信息包
        else if (head.bodyType == CAR_MSG_TYPE) {
            PACK_ALLCARMSG_BACK result;
            read(SingleDB::getInstance()->get_fd(), &result, sizeof(PACK_ALLCARMSG_BACK));
            qDebug() << "fd:" << SingleDB::getInstance()->get_fd() << " client recv: carmsg\n";
            // 发送带车辆信息包的信号
            emit car_msg(result);
            memset(&result, 0x0, sizeof(PACK_ALLCARMSG_BACK));
        }
        // 图片包
        else if (head.bodyType == PHOTO_TYPE) {
            PACK_PHOTO_BACK result;
            read(SingleDB::getInstance()->get_fd(), &result, sizeof(PACK_PHOTO_BACK));
            qDebug() << "fd:" << SingleDB::getInstance()->get_fd() << " client recv: photo\n";
            emit photo_continue_in(result.result);
            memset(&result, 0x0, sizeof(PACK_ALLCARMSG_BACK));
        }
    }
}
