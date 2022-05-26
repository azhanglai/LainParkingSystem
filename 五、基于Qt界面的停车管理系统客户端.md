[TOC]

### 1、制作客户端开机欢迎画面

#### 1.1 main.cpp

~~~c++
#include "laiwidget.h"
#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    LaiWidget w;
    w.show();

    return a.exec();
}
~~~

#### 1.2 laiwidget 窗口

##### laiwidget.h 头文件

~~~c++
#ifndef LAIWIDGET_H
#define LAIWIDGET_H

#include <QWidget>
#include <QBrush>
#include <QIcon>
#include <QPixmap>
#include <QTimer>
#include <QGraphicsView>
#include <QGraphicsScene>

#include "myitem.h"

class LaiWidget : public QGraphicsView {
    Q_OBJECT

public:
    LaiWidget();

    void init_UI();
    void init_control();
    void init_connect();

    QGraphicsScene *scene;
    MyItem *item1, *item2;
    QTimer *timer;

public slots:
    void watchStatus();
};

#endif // LAIWIDGET_H
~~~

##### laiwidget.cpp 源文件

~~~c++
#include "laiwidget.h"

LaiWidget::LaiWidget() {
    init_UI();
    init_control();
    init_connect();
}

void LaiWidget::init_UI() {
    // 1.设置窗口标题
    this->setWindowTitle("Lai监控系统");
    // 2.设置窗口大小
    this->setFixedSize(800, 600);
    // 3.设置背景图
    this->setBackgroundBrush(QBrush(QPixmap(":png/background.jpeg")));
}

void LaiWidget::init_control() {
    // 1.创建场景
    this->scene = new QGraphicsScene();
    this->setScene(scene);
    // 2.设置场景坐标和视图一致
    this->setSceneRect(0, 0, this->width() - 2, this->height() - 2);
    // 3.创建图元
    this->item1 = new MyItem(":png/1.png", 1);
    this->item2 = new MyItem(":png/2.png", 2);
    // 4.设置图元开始位置
    item1->setPos(125, this->height() / 2);
    item2->setPos(638, this->height() / 2);
    // 5.场景添加图元
    scene->addItem(item1);
    scene->addItem(item2);
    // 6.定时器
    timer = new QTimer();
    timer->start(25);
}

void LaiWidget::init_connect() {
    // 场景的槽函数: advance 作用:触发每一个图元的advance(int)-->图元移动
    connect(timer, &QTimer::timeout, scene, &QGraphicsScene::advance);
    // 场景的槽函数: watchStatus 作用:观察图元状态--状态改变就停止
    connect(timer, &QTimer::timeout, this, &LaiWidget::watchStatus);
}

void LaiWidget::watchStatus() {
    // 判断状态图元状态，变成1就停止
    if(this->item1->status == 1) {
        this->timer->stop();
    }
}
~~~

#### 1.3 myitem 自定义图元类

##### myitem.h 头文件

~~~c++
#ifndef MYITEM_H
#define MYITEM_H

#include <QObject>
#include <QGraphicsItem>
#include <QString>
#include <QRectF>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QWidget>
#include <QPixmap>
#include <QPainter>

class MyItem : public QObject, public QGraphicsItem {
    Q_OBJECT
public:
    MyItem(QString path, int move);

    QRectF boundingRect() const;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*);
    void advance(int);

    int status;

private:
    QString itempath;
    QPixmap img;
    int move;

signals:
    void end();
};

#endif // MYITEM_H
~~~

##### myitem.cpp 源文件

~~~c++
#include "myitem.h"
#include <unistd.h>

MyItem::MyItem(QString path, int move) {
    this->itempath = path;
    this->img.load(this->itempath);
    this->move = move;
    this->status = 0;
}

// 图元边界设定函数
QRectF MyItem::boundingRect() const {
    return QRectF(-this->img.width() / 2,
                  -this->img.height() / 2,
                  this->img.width(),
                  this->img.height());
}

// 图元绘制事件
void MyItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
    painter->drawPixmap(-this->img.width() / 2, -this->img.height() / 2, this->img);
}

// 图元移动判断函数
void MyItem::advance(int) {
    if(this->move == 1) {
        //碰撞检测
        if(this->collidingItems().count() > 0) {
            //图元状态改变
            this->status = 1;
        }
        //没有碰撞就继续走
        this->setPos(mapToScene(1, 0));
    }
    if(this->move == 2) {
        //碰撞检测
        if(this->collidingItems().count() > 0) {
            //图元状态改变
            this->status = 1;
            //发送信号碰撞信号，准备跳转页面
            sleep(2);
            emit end();
        }
        //没有碰撞就继续走
        this->setPos(mapToScene(-1, 0));
    }
}
~~~

#### 1.4 添加源文件（图片）

![img](https://s2.loli.net/2022/05/23/Gu847whrAExpZIB.png)

#### 1.5 编译配置.pro文件

~~~c++
QT      += core gui
QT      += multimedia
CONFIG  += c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = LaiClient
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS


SOURCES += \
        main.cpp \
        laiwidget.cpp \
        myitem.cpp

HEADERS += \
        laiwidget.h \
        myitem.h

RESOURCES += \
        png.qrc
~~~

#### 1.6 测试结果

![img](https://s2.loli.net/2022/05/23/X4RNBfeKJtF2uEh.png)

### 2、使用单例模式，编写客户端与服务端的连接类 SingleDB 类

#### 2.1 singledb.h 头文件

~~~c++
#ifndef SINGLEDB_H
#define SINGLEDB_H

#include <QString>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <QDebug>

#define ERR_EXIT(m) {\
    do{\
        perror(m);\
        exit(EXIT_FAILURE);\
    } while (0);\
}

class SingleDB {
public:
    static SingleDB* getInstance();

    int socket_connect();
    int get_fd();

    QString get_image_path();
    QString get_video_path();
    int get_size();

private:
    SingleDB();

    static SingleDB* myDB;
    int socketfd;
    char IP[64];
    int port;

    QString image_path;
    QString video_path;
    int size;
};

#endif // SINGLEDB_H

~~~

#### 2.2 singledb.cpp 源文件

~~~c++
#include "singledb.h"
#include <QFileDialog>

SingleDB* SingleDB::myDB = nullptr;

SingleDB::SingleDB() {
    QFile file("../LaiClient/data/set_data.txt");
    if (!file.open(QIODevice::ReadWrite)) { return ; }
    else {
        int line = 0;
        // 判断文件是否已经读到末尾
        while (!file.atEnd()) {
            line++;
            QByteArray byte = file.readLine().trimmed();
            char* str = byte.data();
            // 读取IP数据
            if(line == 1) {
                strcpy(this->IP, str);
            }
            // 读取端口数据
            else if(line == 2) {
                QString num(str);
                this->port = num.toInt();
            }
            // 读取图片路径
            else if(line == 3) {
                this->image_path =QString(str);
            }
            // 读取视频路径
            else if(line == 4) {
                this->video_path = QString(str);
            }
            // 读取文件大小
            else if(line == 5) {
                QString num(str);
                this->size = num.toInt();
                file.close();
                return;
            }
        }
        file.close();
    }
}
// 单例模式
SingleDB* SingleDB::getInstance() {
    if (SingleDB::myDB == nullptr) {
        SingleDB::myDB = new SingleDB();
    }
    return SingleDB::myDB;
}
// 连接服务器，成功放回0，失败返回-1
int SingleDB::socket_connect() {
    // 1.创建套接字
    int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd == -1) {
        ERR_EXIT("socket");
        return -1;
    }
    // 2.绑定IP和端口
    struct sockaddr_in ser_addr;
    ser_addr.sin_family = AF_INET; // 协议家族 ipv4
    ser_addr.sin_port = htons(this->port); // 网络字节序 主机字节序
    ser_addr.sin_addr.s_addr = inet_addr(this->IP); // 手动获取服务器IP
    int ret = connect(sockfd, (struct sockaddr*)&ser_addr, sizeof(struct sockaddr_in));
    if (ret == -1) {
        ERR_EXIT("connect");
        return -1;
    }
    qDebug() << "connect success\n";
    this->socketfd = sockfd;
    return 0;
}
// 获取连接fd
int SingleDB::get_fd() {
    return socketfd;
}
// 获取图片保存路径
QString SingleDB::get_image_path() {
    return image_path;
}
// 获取视频保存路径
QString SingleDB::get_video_path() {
    return video_path;
}
// 获取文件大小
int SingleDB::get_size() {
    return size;
}
~~~

### 3、使用Qt线程，编写客户端和服务器连接交互线程

#### 3.1 thread.h 头文件

~~~c++
#ifndef THREAD_H
#define THREAD_H

#include <QThread>
#include <QTimer>
#include "Tool/package.h"

class Thread : public QThread {
    Q_OBJECT

public:
    static Thread* getInstance();

    int get_car_amount() { return car_amount; }

private:
    Thread();
    static Thread* myThread;
    int car_amount = 0;
    QTimer* heartbeat;

protected:
    void run();
signals:
    void Login(bool success);
    void Register(bool success);
    void out(PACK_EXIT_BACK );
    void car_msg(PACK_ALLCARMSG_BACK);
    void photo_continue_in(int result);
public slots:
    void send_heartbeat();
};

#endif // THREAD_H
~~~

#### 3.2 thread.cpp 源文件

~~~c++
#include "thread.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <QDebug>

#include "Tool/singledb.h"
#include <QDebug>

Thread* Thread::myThread = nullptr;
// 连接定时器
Thread::Thread() {
    //定时器：每两分钟发一次心跳包
    heartbeat = new QTimer();
    connect(heartbeat, &QTimer::timeout, this, &Thread::send_heartbeat);
    heartbeat->start(120000);
}
// 单例模式
Thread* Thread::getInstance() {
    if(Thread::myThread==nullptr) {
        Thread::myThread=new Thread();
    }
    return Thread::myThread;
}
// 发送心跳到服务器
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
// 线程run函数
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
            // 发送带出场包的信号
            emit out(result);
            memset(&result, 0x0, sizeof(PACK_EXIT_BACK));
        }
        // 车辆信息包
        else if (head.bodyType == CAR_MSG_TYPE) {
            PACK_ALLCARMSG_BACK result;
            read(SingleDB::getInstance()->get_fd(), &result, sizeof(PACK_ALLCARMSG_BACK));
            // 发送带车辆信息包的信号
            emit car_msg(result);
            memset(&result, 0x0, sizeof(PACK_ALLCARMSG_BACK));
        }
        // 图片包
        else if (head.bodyType == PHOTO_TYPE) {
            PACK_PHOTO_BACK result;
            read(SingleDB::getInstance()->get_fd(), &result, sizeof(PACK_PHOTO_BACK));
            emit photo_continue_in(result.result);
            memset(&result, 0x0, sizeof(PACK_ALLCARMSG_BACK));
        }
    }
}

~~~

### 4、制作客户端连接设置界面

#### 2.1 set.ui UI界面

![img](https://s2.loli.net/2022/05/23/9kaizvAjmcDCWSB.png)

#### 2.2 set.h 头文件

~~~c++
#ifndef SET_H
#define SET_H
#include <QWidget>

namespace Ui {
    class Set;
}

class Set : public QWidget {
    Q_OBJECT
public:
    explicit Set(QWidget *parent = 0);
    ~Set();

    void init_connect();
private:
    Ui::Set *ui;
private slots:
    void on_OK_btn_clicked();
    void Select_VideoPath();
    void Select_ImagePath();
};
#endif // SET_H
~~~

#### 2.3 set.cpp 源文件

~~~c++
#include "set.h"
#include "ui_set.h"
#include <QTextStream>
#include <QMessageBox>
#include <QRegExpValidator>
#include <QFileDialog>

#include "Tool/singledb.h"
#include "Thread/thread.h"

Set::Set(QWidget *parent) : QWidget(parent), ui(new Ui::Set) {
    ui->setupUi(this);
    init_connect();
}

Set::~Set() {
    delete ui;
}
// 连接信号与槽函数
void Set::init_connect() {
    connect(ui->VideoPath_btn, &QPushButton::clicked, this, &Set::Select_VideoPath);
    connect(ui->ImagePath_btn, &QPushButton::clicked, this, &Set::Select_ImagePath);
}

void Set::on_OK_btn_clicked() {
    if (ui->IP_edit->text().trimmed() == tr("") ||
            ui->Port_edit->text().trimmed() == tr("") ||
            ui->ImagePath_edit->text().trimmed() == tr("") ||
            ui->VideoPath_edit->text().trimmed() == tr(""))
    {
        QMessageBox::warning(this, tr("提示！"), tr("内容不能为空！"), QMessageBox::Yes);
        return ;
    } else {
         QFile file("../LaiClient/data/set_data.txt");
         // 已读写方式打开文件， 如果文件不存在会自动创建文件
         if(!file.open(QIODevice::WriteOnly)) {
             return;
         } else {
            QTextStream in(&file);
            in << ui->IP_edit->text() << "\n"; 			// 获取输入的IP
            in << ui->Port_edit->text() << "\n";		// 获取输入的Port
            in << ui->ImagePath_edit->text() << "\n";	// 获取输入的图片保存路径
            in << ui->VideoPath_edit->text() << "\n";	// 获取输入的视频保存路径
            file.close();
         }
         // 根据输入的IP和port, 连接服务器
         int res = SingleDB::getInstance()->socket_connect();
         // 启动连接线程，实现客户端与服务器的交互
         Thread::getInstance()->start();
         if(res == 0) {
             qDebug() << "ok\n";
             QMessageBox::warning(this, tr("提示！"), tr("连接服务器成功！"), QMessageBox::Yes);
         } else {
             QMessageBox::warning(this, tr("提示！"), tr("连接服务器失败！"), QMessageBox::Yes);
         }
    }
}

// 视频路径选择窗口函数
void Set::Select_VideoPath() {
    QString Video_path = QFileDialog::getExistingDirectory(this, "选择视频存储路径", "./");
    if(Video_path != nullptr) {
        // 显示到编辑框上
        ui->VideoPath_edit->setText(Video_path);
    }
}

// 图片路径选择窗口函数
void Set::Select_ImagePath() {
    QString Image_path=QFileDialog::getExistingDirectory(this, "选择图片存储路径", "./");
    if(Image_path != nullptr) {
        // 显示到编辑框上
        ui->ImagePath_edit->setText(Image_path);
    }
}
~~~

### 5、制作客户端登录和注册界面

#### 5.1 用户登录UI界面

![img](https://s2.loli.net/2022/05/23/ou5COFvXDj1IKGW.png)

#### 5.2 login.h 头文件

~~~c++
#ifndef LOGIN_H
#define LOGIN_H

#include <QWidget>
#include <QPaintEvent>
#include <QPainter>
#include <QTime>
#include <QMessageBox>
#include <QDialog>

#include "Tool/singledb.h"
#include "Tool/package.h"
#include "Thread/thread.h"
#include "Widget/register.h"

namespace Ui {
    class Login;
}

class Login : public QWidget {
    Q_OBJECT
public:
    explicit Login(QWidget *parent = 0);
    ~Login();

    void init_connect();
    QString getcode();
    void paintEvent(QPaintEvent* );
    void mousePressEvent(QMouseEvent *event); // 鼠标点击事件

    static QString user;
private:
    Ui::Login *ui;

    QString code;
    Register* regWin;
    int mark;
signals:
    void tomain();

private slots:
    void on_login_btn_clicked();
    void on_register_btn_clicked();
    void on_pwdeye_btn_clicked();
    void backLogin();
    void dologin(bool success);
    void tomainwin();
};

#endif // LOGIN_H
~~~

#### 5.3 login.cpp 头文件

~~~c++
#include "login.h"
#include "ui_login.h"
#include <QDebug>

QString Login::user;

Login::Login(QWidget *parent) : QWidget(parent), ui(new Ui::Login) {
    ui->setupUi(this);
    this->mark = 0; 				// 密码明码标记
    this->code = this->getcode();	

    ui->pwd_edit->setEchoMode(QLineEdit::Password);
    this->regWin = new Register();
    init_connect();
    this->update();		// 绘画事件刷新
}

Login::~Login() {
    delete ui;
}

void Login::init_connect() {
    // 注册窗口关闭， 显示登陆窗口
    connect(this->regWin, &Register::back, this, &Login::backLogin);
    // 登录信号，执行登录槽函数
    connect(Thread::getInstance(), SIGNAL(Login(bool)), this, SLOT(dologin(bool)));
    // 登录成功信号，显示主画面窗户
    connect(this, &Login::tomain, this, &Login::tomainwin);

}

// 生成验证码
QString Login::getcode() {
    update();
    QString ncode;
    // 随机数字
    for (int i = 0; i < 4; ++i) {
        int num = qrand() % 3;
        if (num == 0) {
            // 数字
            ncode += QString::number(qrand() % 10);
        } else if(num == 1) {
            // 大写字母
            int temp = 'A';
            ncode += static_cast<QChar>(temp + qrand() % 26);
        } else if(num == 2) {
            // 小写字母
            int temp = 'a';
            ncode += static_cast<QChar>(temp + qrand() % 26);
        }
    }
    return ncode;
}

// 验证码干扰元素
void Login::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    QPen pen;
    painter.drawPixmap(this->rect(), QPixmap(":image/pt.jpg"));
    // 画点
    for(int i = 0;i < 100;++i) {
        pen = QPen(QColor(qrand()%256, qrand()%256, qrand()%256));
        painter.setPen(pen);
        painter.drawPoint(360 + qrand()%110, 250 + qrand()%45);
    }
    // 画线
    for(int i = 0;i < 10;++i){
        painter.drawLine(360 + qrand()%110, 250 + qrand()%45,
                         360 + qrand()%110, 250 + qrand()%45);
    }
    pen = QPen(QColor(255, 0, 0, 100));
    QFont font("楷体", 20, QFont::Bold, true);
    painter.setFont(font);
    painter.setPen(pen);
    // 绘画字
    for(int i = 0;i < 4;++i) {
        painter.drawText(360+25*i, 250, 30, 40, Qt::AlignCenter, QString(code[i]));
    }
}

// 鼠标点击事件
void Login::mousePressEvent(QMouseEvent *event) {
    //验证码区域做重写鼠标点击事件
    int x = event->x();
    int y = event->y();
    if(x > 360 && x < 460 && y > 230 && y < 280) {
        this->code = this->getcode();
    }
}

// 登录验证函数
void Login::on_login_btn_clicked() {
    QString captcha = ui->code_edit->text();
    if(captcha.toLower() == this->code.toLower()) {
        PACK_HEAD head;
        PACK_REGIST_LOGIN login_body;
        char buffer[sizeof(PACK_HEAD) + sizeof(PACK_REGIST_LOGIN)] = { 0 };
        memset(&head, 0, sizeof(PACK_HEAD));
        memset(&login_body, 0, sizeof(PACK_REGIST_LOGIN));

        head.bodyType=2;
        strcpy(login_body.name, ui->name_edit->text().toStdString().c_str());
        strcpy(login_body.pwd, ui->pwd_edit->text().toStdString().c_str());

        memcpy(buffer, &head, sizeof(PACK_HEAD));
        memcpy(buffer + sizeof(PACK_HEAD), &login_body, sizeof(PACK_REGIST_LOGIN));
        write(SingleDB::getInstance()->get_fd(), buffer, sizeof(PACK_HEAD) + sizeof(PACK_REGIST_LOGIN));
        return;
    } else {
        QMessageBox::warning(this, tr("警告！"), tr("验证码错误！"), QMessageBox::Yes);
        this->code = this->getcode();
    }
}

// 跳转到注册
void Login::on_register_btn_clicked() {
    this->hide();

    this->regWin->show();
    // 清空输入框内容
    ui->name_edit->clear();
    ui->pwd_edit->clear();
    ui->code_edit->clear();
}

void Login::on_pwdeye_btn_clicked() {
    // 0是密文 1是明文
    if(this->mark == 0) {
        ui->pwd_edit->setEchoMode(QLineEdit::Normal);
        this->mark = 1;
    } else if(this->mark == 1) {
        ui->pwd_edit->setEchoMode(QLineEdit::Password);
        this->mark = 0;
    }
}

// 返回到登录
void Login::backLogin() {
    this->regWin->close();
    this->show();
}

// 根据服务器返回的布尔类型判断是否登陆成功
void Login::dologin(bool success) {
    if(success == true) {
        this->user = ui->name_edit->text();
        emit tomain();
    } else {
        QMessageBox::warning(this, tr("提示！"), tr("帐号或密码错误！"), QMessageBox::Yes);
        // 清空输入框内容
        ui->name_edit->clear();
        ui->pwd_edit->clear();
        ui->code_edit->clear();
        this->update();
    }
}
// 跳转到主画面窗口
void Login::tomainwin() {
    QMessageBox::warning(this, tr("提示！"), tr("登录成功！"), QMessageBox::Yes);
}
~~~

#### 5.4 用户注册UI界面

![img](https://s2.loli.net/2022/05/23/kCwB3pL9n45Ebai.png)

#### 5.5 register.h 头文件

~~~c++
#ifndef REGISTER_H
#define REGISTER_H

#include <QWidget>

#include "Tool/singledb.h"
#include "Tool/package.h"
#include "Thread/thread.h"

namespace Ui {
    class Register;
}

class Register : public QWidget {
    Q_OBJECT
public:
    explicit Register(QWidget *parent = 0);
    ~Register();

    void init_connect();
private:
    Ui::Register *ui;
    int mark,mark2;
signals:
    void back();
private slots:
    void on_eye_btn_clicked();
    void on_eye2_btn_clicked();
    void on_register_btn_clicked();
    void doregister(bool success);
};

#endif // REGISTER_H
~~~

#### 5.6 register.cpp 源文件

~~~c++
#include "register.h"
#include "ui_register.h"

#include <QMessageBox>

Register::Register(QWidget *parent) : QWidget(parent), ui(new Ui::Register) {
    ui->setupUi(this);
    this->mark = 0;
    this->mark2 = 0;
    ui->pwd_edit->setEchoMode(QLineEdit::Password);
    ui->pwd2_edit->setEchoMode(QLineEdit::Password);

    init_connect();
}

Register::~Register() {
    delete ui;
}

void Register::init_connect() {
    // 返回按钮触发 back信号，回到登录界面
    connect(ui->esc_btn, &QPushButton::clicked, this, &Register::back);
    // 交互线程触发注册信号，执行注册结果槽函数
    connect(Thread::getInstance(), SIGNAL(Register(bool)), this, SLOT(doregister(bool)));
}
// 密码是明文还是密文的按钮槽函数
void Register::on_eye_btn_clicked() {
    // 0是密文 1是明文
    if (this->mark == 0) {
        ui->pwd_edit->setEchoMode(QLineEdit::Normal);
        this->mark = 1;
    } else if (this->mark == 1) {
        ui->pwd_edit->setEchoMode(QLineEdit::Password);
        this->mark = 0;
    }
}
// 密码是明文还是密文的按钮槽函数
void Register::on_eye2_btn_clicked() {
    // 0是密文 1是明文
    if (this->mark2 == 0) {
        ui->pwd2_edit->setEchoMode(QLineEdit::Normal);
        this->mark2 = 1;
    } else if (this->mark2 == 1) {
        ui->pwd2_edit->setEchoMode(QLineEdit::Password);
        this->mark2 = 0;
    }
}
// 注册按钮槽函数
void Register::on_register_btn_clicked() {
    // 如果内容为空就提示
    if(ui->name_edit->text().trimmed() == tr("") || ui->pwd_edit->text().trimmed() == tr("")) {
        QMessageBox::warning(this, tr("提示！"), tr("内容不能为空！"), QMessageBox::Yes);
    } else {
        // 如果长度大于5就继续判断
        if(ui->name_edit->text().length() > 5 && ui->pwd_edit->text().length() > 5) {
            // 如果密码和二次输入的密码一致就继续判断
            if(ui->pwd_edit->text() == ui->pwd2_edit->text()){
                PACK_HEAD head;
                PACK_REGIST_LOGIN login_body;
                char buffer[sizeof(PACK_HEAD) + sizeof(PACK_REGIST_LOGIN)] = { 0 };
                memset(&head, 0, sizeof(PACK_HEAD));
                memset(&login_body, 0, sizeof(PACK_REGIST_LOGIN));
                // 包头类型为注册
                head.bodyType = 1;
                // 将编辑框里面内容赋值给结构体
                strcpy(login_body.name, ui->name_edit->text().toStdString().c_str());
                strcpy(login_body.pwd, ui->pwd_edit->text().toStdString().c_str());
                // 拷贝包体+包体
                memcpy(buffer, &head, sizeof(PACK_HEAD));
                memcpy(buffer + sizeof(PACK_HEAD), &login_body, sizeof(PACK_REGIST_LOGIN));
                write(SingleDB::getInstance()->get_fd(), buffer, sizeof(PACK_HEAD) + sizeof(PACK_REGIST_LOGIN));
            }
        }
        // 清空输入框内容
        ui->name_edit->clear();
        ui->pwd_edit->clear();
        ui->pwd2_edit->clear();
    }
}
// 注册结果
void Register::doregister(bool success) {
    if (success == true) {
        QMessageBox::warning(this, tr("提示！"), tr("注册成功！"), QMessageBox::Yes);
        back();
    } else {
        QMessageBox::warning(this, tr("提示！"), tr("已经有该用户！"), QMessageBox::Yes);
    }
}
~~~

#### 5.7 测试注册登录界面

注册界面

![img](https://s2.loli.net/2022/05/23/49Ec5g8pGQqYxtn.png)

登录界面

![img](https://s2.loli.net/2022/05/23/49Ec5g8pGQqYxtn.png)

### 6、制作客户端管理系统UI界面

#### 6.1 系统主界面UI

##### ui界面

![img](https://s2.loli.net/2022/05/24/D6xhtGmVeapkobw.png)

##### main_interface.h 头文件

~~~c++
#ifndef MAIN_INTERFACE_H
#define MAIN_INTERFACE_H

#include <QWidget>
#include <QTimer>
#include <QDateTime>

#include "Widget/set.h"
#include "Widget/main_in.h"
#include "Widget/main_out.h"
#include "Widget/main_inmanage.h"
#include "Widget/main_equery.h"
#include "Widget/main_playback.h"

namespace Ui {
    class Main_Interface;
}

class Main_Interface : public QWidget {
    Q_OBJECT
public:
    explicit Main_Interface(QWidget *parent = 0);
    ~Main_Interface();

    void init_connect();

    Set* setwin;					// 用户设置窗口
    Main_In *inwin;					// 车辆入场监控窗口
    Main_Out *outwin;				// 车辆出场监控窗口
    Main_Inmanage *inmanagewin;		// 停车场内部监控窗口
    Main_Equery *equerywin;			// 车辆信息查询窗口
    Main_Playback *playbackwin;		// 监控回放窗口
private:
    Ui::Main_Interface *ui;

    int mark;
    QTimer *time;
private slots:
    void qtimeSlot();
    void on_in_out_btn_clicked();
    void on_inside_btn_clicked();
    void on_query_btn_clicked();
    void on_playback_btn_clicked();
    void on_set_btn_clicked();
    void update_car_amount(int num);
};

#endif // MAIN_INTERFACE_H

~~~

##### main_interface.cpp 源文件

~~~c++
#include "main_interface.h"
#include "ui_main_interface.h"

#include "Widget/login.h"
#include "Thread/thread.h"

Main_Interface::Main_Interface(QWidget *parent) : QWidget(parent), ui(new Ui::Main_Interface) {
    ui->setupUi(this);
    this->mark = 0;
    ui->car_amount_label->setText(QString("剩余车位：") + QString::number(200 - Thread::getInstance()->get_car_amount()));
    ui->user_label->setText(QString("当前用户：") + Login::user);
	// 栈窗口添加车辆入场监控窗口
    this->inwin = new Main_In();
    ui->stackedWidget->addWidget(inwin);
	// 栈窗口添加车辆出场监控窗口
    this->outwin = new Main_Out();
    ui->stackedWidget->addWidget(outwin);
	// 栈窗口添加停车场内部监控窗口
    this->inmanagewin = new Main_Inmanage();
    ui->stackedWidget->addWidget(inmanagewin);
	// 栈窗口添加车辆信息查询窗口
    this->equerywin = new Main_Equery();
    ui->stackedWidget->addWidget(equerywin);
	// 栈窗口添加监控回放窗口
    this->playbackwin = new Main_Playback();
    ui->stackedWidget->addWidget(playbackwin);
	// 将车辆入场监控窗口 设置为当前栈窗口
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
// 进场和出场窗口的切换按钮槽函数
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
// 切换停车场内部监控窗口按钮槽函数
void Main_Interface::on_inside_btn_clicked() {
    ui->stackedWidget->setCurrentWidget(inmanagewin);
}
// 切换车辆信息查询窗口按钮槽函数
void Main_Interface::on_query_btn_clicked() {
    ui->stackedWidget->setCurrentWidget(equerywin);
}
// 切换监控回放窗口按钮槽函数
void Main_Interface::on_playback_btn_clicked() {
    ui->stackedWidget->setCurrentWidget(playbackwin);
}
// 切换用户设置窗口按钮的槽函数
void Main_Interface::on_set_btn_clicked() {
    this->setwin = new Set();
    this->setwin->show();
}
// 更新停车场空位
void Main_Interface::update_car_amount(int num) {
    ui->car_amount_label->setText(QString("剩余车位：") + QString::number(num));
}

~~~



#### 6.2 车辆进入停车场UI界面

![img](https://s2.loli.net/2022/05/24/6u8Of2Dz3nxyqsE.png)

#### 6.3 车辆出去停车场UI界面

![img](https://s2.loli.net/2022/05/24/uqnZJ1U4S8QKk7p.png)

#### 6.4 停车场内部监控UI界面

![img](https://s2.loli.net/2022/05/24/us9RbTKNavJZCBV.png)

#### 6.5 车辆信息查询UI界面

![img](https://s2.loli.net/2022/05/24/5McOy7vzZTgDCLx.png)

#### 6.6 监控回放UI界面

![img](https://s2.loli.net/2022/05/24/OkcRoMVjwKyILCg.png)