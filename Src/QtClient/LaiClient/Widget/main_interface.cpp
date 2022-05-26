#include "main_interface.h"
#include "ui_main_interface.h"

#include "Widget/login.h"
#include "Thread/thread.h"

Main_Interface::Main_Interface(QWidget *parent) : QWidget(parent), ui(new Ui::Main_Interface) {
    ui->setupUi(this);
    this->mark = 0;
    ui->car_amount_label->setText(QString("剩余车位：") + QString::number(500 - Thread::getInstance()->get_car_amount()));
    ui->user_label->setText(QString("当前用户：") + Login::user);

    this->inwin = new Main_In();
    ui->stackedWidget->addWidget(inwin);

    this->outwin = new Main_Out();
    ui->stackedWidget->addWidget(outwin);

    this->inmanagewin = new Main_Inmanage();
    ui->stackedWidget->addWidget(inmanagewin);

    this->equerywin = new Main_Equery();
    ui->stackedWidget->addWidget(equerywin);

    this->playbackwin = new Main_Playback();
    ui->stackedWidget->addWidget(playbackwin);

    ui->stackedWidget->setCurrentWidget(inwin);
    this->time = new QTimer(this);
    this->time->start(500);
    init_connect();
}

Main_Interface::~Main_Interface() {
    delete ui;
}

void Main_Interface::init_connect() {
    connect(this->time, &QTimer::timeout, this, &Main_Interface::qtimeSlot);
    connect(this->inwin, SIGNAL(update_car_amount(int)), this, SLOT(update_car_amount(int)));
    connect(this->outwin, SIGNAL(update_car_amount(int)), this, SLOT(update_car_amount(int)));
}

// 获取当前时间函数
void Main_Interface::qtimeSlot() {
   QDateTime date_time = QDateTime::currentDateTime();
   QString cur_date = date_time.toString("当前系统时间：yyyy-MM-dd hh:mm:ss");
   ui->time_label->setText(cur_date);
}

void Main_Interface::on_in_out_btn_clicked() {
    if (this->mark == 0) {
        ui->stackedWidget->setCurrentWidget(outwin);
        ui->in_out_btn->setText("进场模式");
        this->mark = 1;
    } else if (this->mark == 1) {
        ui->stackedWidget->setCurrentWidget(inwin);
        ui->in_out_btn->setText("出场模式");
        this->mark = 0;
    }
}
void Main_Interface::on_inside_btn_clicked() {
    ui->stackedWidget->setCurrentWidget(inmanagewin);
    inmanagewin->update_car();
}

void Main_Interface::on_query_btn_clicked() {
    ui->stackedWidget->setCurrentWidget(equerywin);
}

void Main_Interface::on_playback_btn_clicked() {
    ui->stackedWidget->setCurrentWidget(playbackwin);
}

void Main_Interface::on_set_btn_clicked() {
    this->setwin = new Set();
    this->setwin->show();
}

void Main_Interface::update_car_amount(int num) {
    ui->car_amount_label->setText(QString("剩余车位：") + QString::number(num));
}
