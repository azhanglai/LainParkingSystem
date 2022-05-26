[TOC]

### 1、实现车辆进入停车场管理

#### 1.1 UI界面

![img](https://s2.loli.net/2022/05/24/6u8Of2Dz3nxyqsE.png)

#### 1.2 Qt线程实现视频帧处理

##### thread_in.h 头文件

~~~c++
#ifndef THREAD_IN_H
#define THREAD_IN_H

#include <QThread>
#include <QObject>
#include <QPainter>
#include <opencv2/opencv.hpp>
#include <easypr.h>
using namespace cv;

class Thread_In :public QThread {
    Q_OBJECT
public:
    Thread_In();
	// 使用级联分类器处理图片
    void detectCarDraw(Mat& img, CascadeClassifier& cascade, double scale);
private:
    VideoCapture capture;		// 读取视频的对象
    Mat picture, img_RGB;		// 图像对象
    QImage img;					// Qt图像对象
    CascadeClassifier cascade;	// 级联分类器对象
protected:
    void run();					// 重载Qt线程处理函数
signals:
    void sendImg(QImage img);	// 发送Qt图像的信号
    void sendMat(Mat mat);		// 发送Opencv图像的信号
};

#endif // THREAD_IN_H
~~~

##### thread_in.cpp 源文件

~~~c++
#include "thread_in.h"
#include <unistd.h>

Thread_In::Thread_In() {
    // 打开摄像头视频或者本地视频
    this->capture.open("../LaiClient/video.mp4");
    // 加载xml级联分类器
    this->cascade.load("../LaiClient/data/cars.xml");
}

void Thread_In::detectCarDraw(Mat& img, CascadeClassifier& cascade, double scale) {
    //我们使用级联分类器前有一套标准处理流程，这个处理流程目的是加快计算机识别物体的速度。
    //对于原始视频来说，每张图片都非常大
    vector<Rect> cars;
    Mat grayImg, smallImg(cvRound(img.rows/scale), cvRound(img.cols/scale), CV_8UC1);
    // 1.先转灰度
    cvtColor(img, grayImg, CV_BGR2GRAY);
    // 2.再把图片缩小一半
    resize(grayImg, smallImg, smallImg.size(), 0, 0);
    // 3.直方图均值化
    equalizeHist(smallImg,smallImg);
    // 4.开始利用级联分类器进行识别物体
    // cars: 通过级联分类器识别完成之后会把数据存放到cars中
    // 1.1: 每次识别后图像矩形框扩大1.1
    // 3: opencv识别物体的时候最少检测3次才是算是目标
    // Size(30,30)：能识别的物体最小必须大于30*30像素。
    cascade.detectMultiScale(smallImg, cars, 1.1, 3, 0|CV_HAAR_SCALE_IMAGE, Size(30, 30));
    for (vector<Rect>::const_iterator r = cars.begin(); r != cars.end(); r++) {
        // 在原始图片中绘制，不是在smallimg中绘制
        rectangle(img,cvPoint(r->x*scale,r->y*scale),
                  cvPoint((r->x + r->width)*scale, (r->y + r->height)*scale),
                  Scalar(0, 255, 0), 3);
    }
}
// 线程处理函数
void Thread_In::run() {
    // 1.将视频帧图像读到picture
    // 2.使用级联分类器处理图像picture
    // 3.将处理后的picture转成彩色，再转成Qt图像
    // 4.发送Qt图像和OpenCV图像的信号，触发槽函数将其显示出来
    while(capture.read(picture)) {
        if(picture.data) {
            detectCarDraw(picture, cascade, 2);
            cvtColor(picture, img_RGB, CV_BGR2RGB);
            img = QImage(img_RGB.data, img_RGB.cols, img_RGB.rows, QImage::Format_RGB888);
            emit sendMat(picture);
            emit sendImg(img);
        }
    }
}
~~~

#### 1.3 实现车辆入场管理

##### main_in.h 头文件

~~~c++
#ifndef MAIN_IN_H
#define MAIN_IN_H

#include <QWidget>
#include <QPainter>
#include <QEvent>
#include <QDateTime>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include "Thread/thread.h"
#include "Thread/thread_in.h"
using namespace cv;
using namespace easypr;

namespace Ui {
    class Main_In;
}

class Main_In : public QWidget {
    Q_OBJECT
public:
    explicit Main_In(QWidget *parent = nullptr);
    ~Main_In();

    void init_connect();
    void insertTableItems();

    static int car_num;
    static int mark;
private:
    Ui::Main_In *ui;

    Thread_In *thread_in;
    Mat mat, mat_RGB, plateMat_car, plateMat;
    QImage Image, car_image;

    CPlateRecognize pr;
    vector<CPlate> plateVec;
    CPlate plate;

    QString location, old_car_date, date, time;

    QPixmap pixmap, fitpixmap;

    list<QString> queue_pathstr;

    FILE *fp;
    PACK_HEAD head_photo;
    PACK_PHOTO photo;
    int num;

    QString path, carInfo;
    QDateTime cur_time;
protected:
    void paintEvent(QPaintEvent*);
signals:
    void update_car_amount(int num);
private slots:
    void receiveimg(QImage img);
    void receivemat(Mat mat);

    void on_carNum_btn_clicked();
    void on_modifyNum_btn_clicked();
    void photo_send(int result);

};

#endif // MAIN_IN_H
~~~

##### main_in.cpp 源文件

~~~c++
#include "main_in.h"
#include "ui_main_in.h"

#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <unistd.h>

#include "Tool/package.h"
#include "Tool/singledb.h"
#include "Sqlite/car_model.h"
#include <QDebug>

int Main_In::mark = 0;		// 是否车牌识别标记
int Main_In::car_num = 0;	// 停车场空闲位置

Main_In::Main_In(QWidget *parent) : QWidget(parent), ui(new Ui::Main_In) {
    ui->setupUi(this);

    car_num = 500 - Thread::getInstance()->get_car_amount();
    this->location = "西大门";
    QDateTime date_time = QDateTime::currentDateTime();
    this->date = date_time.toString("yyyy-MM-dd");

    // tableWidget
    ui->tableWidget->setColumnCount(4);
    QStringList header;
    header << "序号" << "车牌号码" << "时间" << "位置";
    ui->tableWidget->setHorizontalHeaderLabels(header);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->tableWidget->setColumnWidth(0, 20);
    ui->tableWidget->setColumnWidth(1, 100);
    ui->tableWidget->setColumnWidth(2, 140);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    ui->tableWidget->verticalHeader()->setVisible(false);
    ui->tableWidget->setAlternatingRowColors(true);

    // 倒序插入数据
    ui->tableWidget->sortByColumn(1, Qt::DescendingOrder);
    insertTableItems();

    // 车牌识别参数初始化
    pr.setDetectType(PR_DETECT_CMSER | PR_DETECT_COLOR);
    pr.setResultShow(false);
    pr.setMaxPlates(4);

    // 开启线程
    this->thread_in = new Thread_In();
    init_connect();
    this->thread_in->start();

}

Main_In::~Main_In() {
    delete ui;
}

void Main_In::init_connect() {
    // Qt线程发送OpenCV图像信号，触发接受图像功能的槽函数
    connect(thread_in, SIGNAL(sendMat(Mat)), this, SLOT(receivemat(Mat)), Qt::BlockingQueuedConnection);
    // Qt线程发送Qt图像信号，触发接受图像功能的槽函数
    connect(thread_in, SIGNAL(sendImg(QImage)), this, SLOT(receiveimg(QImage)));
    // 客户与服务器交互线程发送图片发送信号，触发发送图片功能的槽函数
    connect(Thread::getInstance(), SIGNAL(photo_continue_in(int)), this, SLOT(photo_send(int)));
}

// 插入当日的入场车辆信息到表
void Main_In::insertTableItems() {
    int i, nCount, row, col;
    char **qres;
    // 查询车辆入场信息(车牌号，入场时间，入场地点)，返回到qres
    qres = Car_Model::getInstance()->get_in_car(date);
    row = Car_Model::getInstance()->get_row();  // 行数
    col = Car_Model::getInstance()->get_col();  // 列数

    // 创建表格item，加载到表格中
    QTableWidgetItem *item[4];
    this->num = row;
    for (i = 0; i < row; ++i) {
        nCount = ui->tableWidget->rowCount();
        ui->tableWidget->insertRow(nCount);

        // 序号
        item[0] = new QTableWidgetItem(QString("%1").arg(num--));
        ui->tableWidget->setItem(i, 0, item[0]);

        // 车牌号
        item[1] = new QTableWidgetItem(QString(qres[(i+1) * col]));
        ui->tableWidget->setItem(i, 1, item[1]);

        // 时间
        item[2] = new QTableWidgetItem(QString(qres[(i+1) * col + 1]));
        ui->tableWidget->setItem(i, 2, item[2]);

        // 位置
        item[3] = new QTableWidgetItem(QString(qres[(i+1) * col + 2]));
        ui->tableWidget->setItem(i, 3, item[3]);
    }
    this->num = row + 1;
}
// 接受Qt图像功能的槽函数
void Main_In::receiveimg(QImage img) {
    this->Image = img;
    this->update();		// 绘画事件更新图像
}
// 接受OpenCV图像功能的槽函数
void Main_In::receivemat(Mat mat) {
    this->mat = mat.clone();
}

// 画面显示的绘画事件
void Main_In::paintEvent(QPaintEvent* ) {
    if (!this->Image.isNull()) {
        // Video_label图片更新
        this->pixmap = QPixmap::fromImage(this->Image);
        this->fitpixmap = this->pixmap.scaled(ui->inVideo_label->width(),
                                              ui->inVideo_label->height(),
                                              Qt::IgnoreAspectRatio,
                                              Qt::SmoothTransformation);
        // 控件接收图片
        ui->inVideo_label->setPixmap(fitpixmap);
        ui->inVideo_label->setScaledContents(true);
    }
}

// 点击车牌识别的槽函数
void Main_In::on_carNum_btn_clicked() {
    mark = 1;
    this->plateVec.clear(); // 先清空，防止二次点击不识别新车牌

    pr.plateRecognize(this->mat, this->plateVec);
    if (this->plateVec.size() > 0) {
        // 1.车牌识别
        this->plate = plateVec.at(0);
        this->plateMat = this->plate.getPlateMat();
        cvtColor(this->plateMat, this->plateMat_car, CV_BGR2RGB);
        // mat 转 QImage
        this->car_image = QImage(this->plateMat_car.data,
                                 this->plateMat_car.cols,
                                 this->plateMat_car.rows,
                                 QImage::Format_RGB888);
        // 2.控件显示车牌图片
        QPixmap *image = new QPixmap(QPixmap::fromImage(this->car_image));
        image->scaled(ui->car_image_label->size(), Qt::KeepAspectRatio);
        ui->car_image_label->setPixmap(QPixmap::fromImage(this->car_image));

        // 3.控件显示车牌号
        string liences = this->plate.getPlateStr();
        // 字符串部分选择，舍弃颜色内容
        this->carInfo = QString::fromStdString(liences).mid(3, -1);
        this->old_car_date = this->carInfo;
        ui->carNum_edit->setText(this->carInfo);

        // 4.控件显示入场时间
        this->cur_time = QDateTime::currentDateTime();
        this->time = cur_time.toString("yyyy-MM-dd hh:mm:ss");
        ui->inTime_edit->setText(this->time);

    }
}

// 确定车牌识别的槽函数
void Main_In::on_modifyNum_btn_clicked() {
    if (ui->carNum_edit->text().isEmpty()) {
        QMessageBox::warning(this, tr("提示！"), tr("车牌识别为空!"), QMessageBox::Yes);
        return ;
    }
    // 车牌识别确定后，按钮暂时失效
    ui->modifyNum_btn->setEnabled(false);
    ui->carNum_btn->setEnabled(false);

    // 图片存本地
    this->path = SingleDB::getInstance()->get_image_path() + "/" + this->time;
    this->car_image.save(path, "JPG", -1);

    QString path_in = SingleDB::getInstance()->get_image_path() + "/" + this->time + "_big";
    this->Image.save(path_in, "JPG", -1);
    this->queue_pathstr.push_back(path_in);

    // 车辆入场信息插入到本地数据库
    Car_Model::getInstance()->add_car(ui->carNum_edit->text(),
                                      this->date,
                                      this->time,
                                      this->location);

    char **qres2 = Car_Model::getInstance()->get_last_car_num();
    // 在表的首部加入新的车辆信息
    QTableWidgetItem *item2[4];
    ui->tableWidget->insertRow(0);
    --car_num;
    emit update_car_amount(car_num);

    //序号
    item2[0] = new QTableWidgetItem(QString("%1") .arg(num++));
    ui->tableWidget->setItem(0, 0, item2[0]);

    //车牌号码
    item2[1] = new QTableWidgetItem(QString(qres2[3]));
    ui->tableWidget->setItem(0, 1, item2[1]);

    //时间
    item2[2] = new QTableWidgetItem(QString(qres2[4]));
    ui->tableWidget->setItem(0, 2, item2[2]);

    //位置
    item2[3] = new QTableWidgetItem(QString(qres2[5]));
    ui->tableWidget->setItem(0, 3, item2[3]);

    // 发到服务器
    PACK_HEAD head;
    PACK_ENTER car_in;
    char buffer[sizeof(PACK_HEAD) + sizeof(PACK_ENTER)] = { 0 };

    memset(&head, 0, sizeof(PACK_HEAD));
    memset(&car_in, 0, sizeof(PACK_ENTER));

    head.bodyType = CAR_GETIN; // 入场包
    // 标签控件里面内容赋值给结构体
    strcpy(car_in.car_num, ui->carNum_edit->text().toStdString().c_str());
    strcpy(car_in.now_time, ui->inTime_edit->text().toStdString().c_str());
    strcpy(car_in.location, location.toStdString().c_str());
    strcpy(car_in.photo_path, path.toStdString().c_str());

    // 拷贝包头+包体，发送到服务器
    memcpy(buffer, &head, sizeof(PACK_HEAD));
    memcpy(buffer + sizeof(PACK_HEAD), &car_in, sizeof(PACK_ENTER));

    write(SingleDB::getInstance()->get_fd(), buffer, sizeof(PACK_HEAD) + sizeof(PACK_ENTER));
    qDebug() << "fd:" << SingleDB::getInstance()->get_fd() << " client send: getin\n";

    // 传图片到服务器
    usleep(2000);
    this->head_photo.bodyType = PHOTO_TYPE; // 图片包
    char buff[sizeof(PACK_HEAD) + sizeof(PACK_PHOTO)] = {0};

    if(this->queue_pathstr.empty()) {
        return;
    }

    this->fp = fopen(this->queue_pathstr.front().toStdString().c_str(), "rb");
    // 获取文件的大小
    fseek(this->fp, 0, SEEK_END);
    double size = ftell(this->fp);
    size /= 1024;
    this->photo.sum = ceil(size);
    
    QString save_name = this->cur_time.toString("yyyy-MM-dd_hh:mm:ss")+ ".jpg";
    strcpy(this->photo.filename, save_name.toStdString().c_str());
    this->photo.num = 1;
    memcpy(buff, &head_photo, sizeof (PACK_HEAD));
    fseek(this->fp, 0, SEEK_SET);
    this->photo.realSize = fread(this->photo.context, 1, 1024, fp);
    memcpy(buff + sizeof(PACK_HEAD), &photo, sizeof(PACK_PHOTO));
    this->queue_pathstr.pop_front();
    write(SingleDB::getInstance()->get_fd(), buff, sizeof(PACK_HEAD) + sizeof(PACK_PHOTO));
    qDebug() << "fd:" << SingleDB::getInstance()->get_fd() << " client send: photo\n";
    // 本地数据库更新车牌号
    Car_Model::getInstance()->change_car(this->old_car_date.toStdString().c_str(),
                                         ui->carNum_edit->text().toStdString().c_str());
}
// 图片发送的槽函数
void Main_In::photo_send(int result) {
    if (mark == 1) {
        if (result == 1) {
            char buffer[sizeof(PACK_HEAD) + sizeof(PACK_PHOTO)] = { 0 };
            memcpy(buffer, &head_photo, sizeof (PACK_HEAD));
            photo.realSize = fread(photo.context, 1, 1024, fp);
            if (photo.realSize == 0) {
                fclose(fp);
                mark = 0;
                ui->modifyNum_btn->setEnabled(true);
                ui->carNum_btn->setEnabled(true);
                QMessageBox::warning(this, tr("提示！"), tr("图片发送成功！"), QMessageBox::Yes);
                ui->carNum_edit->setText("");
                ui->inTime_edit->setText("");
                return;
            }
            photo.num++;
            memcpy(buffer+sizeof(PACK_HEAD), &photo, sizeof(PACK_PHOTO));
            write(SingleDB::getInstance()->get_fd(), buffer, sizeof(PACK_HEAD) + sizeof(PACK_PHOTO));
        } else {
            fclose(fp);
        }
    }
}
~~~

### 2、实现车辆出去停车场管理

#### 2.1 UI界面

![img](https://s2.loli.net/2022/05/24/uqnZJ1U4S8QKk7p.png)

#### 2.2 Qt线程实现视频帧处理

##### thread_out.h 头文件 (与thread_in.h 一样)

~~~c++
#ifndef THREAD_OUT_H
#define THREAD_OUT_H

#include <QObject>
#include <QThread>
#include <QPainter>
#include <QObject>
#include <opencv2/opencv.hpp>
#include <easypr.h>
using namespace cv;
using namespace easypr;

class Thread_Out : public QThread {
    Q_OBJECT
public:
    Thread_Out();

    void detectCarDraw(Mat& img, CascadeClassifier& cascade, double scale);
private:
    VideoCapture capture;
    Mat picture, img_RGB;
    QImage img;
    CascadeClassifier cascade;
protected:
    void run();
signals:
    void sendImg(QImage img);
    void sendMat(Mat mat);
};

#endif // THREAD_OUT_H
~~~

##### thread_out.cpp 源文件 (与thread_in.cpp 一样)

~~~c++
#include "thread_out.h"

Thread_Out::Thread_Out(){
    this->capture.open("../LaiClient/video.mp4");
    this->cascade.load("../LaiClient/data/cars.xml");
}

void Thread_Out::detectCarDraw(Mat& img, CascadeClassifier& cascade, double scale) {
    vector<Rect> cars;
    Mat grayImg, smallImg(cvRound(img.rows/scale), cvRound(img.cols/scale), CV_8UC1);
    // 1.先转灰度
    cvtColor(img,grayImg,CV_BGR2GRAY);
    // 2.再把图片缩小一半
    resize(grayImg,smallImg,smallImg.size(),0,0);
    // 3.直方图均值化
    equalizeHist(smallImg,smallImg);
    // 4.开始利用级联分类器进行识别物体
    cascade.detectMultiScale(smallImg, cars, 1.1, 3, 0|CV_HAAR_SCALE_IMAGE, Size(30, 30));
    for (vector<Rect>::const_iterator r = cars.begin(); r != cars.end(); r++) {
        // 在原始图片中绘制，不是在smallimg中绘制
        rectangle(img,cvPoint(r->x*scale,r->y*scale),
                  cvPoint((r->x + r->width)*scale, (r->y + r->height)*scale),
                  Scalar(0, 255, 0), 3);
    }
}

void Thread_Out::run() {
    while(capture.read(picture)) {
        if(picture.data) {
            detectCarDraw(picture, cascade, 2);
            cvtColor(picture, img_RGB, CV_BGR2RGB);
            img = QImage(img_RGB.data, img_RGB.cols, img_RGB.rows, QImage::Format_RGB888);
            emit sendMat(picture);
            emit sendImg(img);
        }
    }
}
~~~

#### 2.3 实现车辆出场管理

##### main_out.h 头文件

~~~c++
#ifndef MAIN_OUT_H
#define MAIN_OUT_H

#include <QWidget>
#include <QPainter>
#include <QEvent>
#include <QDateTime>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>

#include "Thread/thread.h"
#include "Thread/thread_in.h"
#include "Thread/thread_out.h"
using namespace cv;

namespace Ui {
    class Main_Out;
}

class Main_Out : public QWidget {
    Q_OBJECT
public:
    explicit Main_Out(QWidget *parent = nullptr);
    ~Main_Out();

     void init_connect();
private:
    Ui::Main_Out *ui;

    Thread_Out *thread_out;

    QImage Image, car_image;

    Mat mat, plateMat_car, plateMat;

    CPlateRecognize pr;
    vector <CPlate> plateVec;
    CPlate plate;

    QPixmap pixmap, fitpixmap;

    QDateTime date_time;
protected:
    void paintEvent(QPaintEvent*);
signals:
    void update_car_amount(int num);
private slots:
    void on_carNum_btn_clicked();
    void receiveimg(QImage img);
    void receivemat(Mat mat);
    void out_msg(PACK_EXIT_BACK msg);
    void on_out_btn_clicked();
};

#endif // MAIN_OUT_H

~~~

##### main_out.cpp 源文件

~~~c++
#include "main_out.h"
#include "ui_main_out.h"

#include <QMetaType>
#include "Tool/singledb.h"
#include "Widget/main_in.h"
#include <QDebug>

Main_Out::Main_Out(QWidget *parent) : QWidget(parent), ui(new Ui::Main_Out) {
    qRegisterMetaType<PACK_EXIT_BACK>("PACK_EXIT_BACK");
    ui->setupUi(this);

    this->pr.setDetectType(PR_DETECT_CMSER | PR_DETECT_COLOR);
    this->pr.setResultShow(false);
    this->pr.setMaxPlates(4);

    // 开启线程
    this->thread_out=new Thread_Out();
    init_connect();
    this->thread_out->start();
}

Main_Out::~Main_Out() {
    delete ui;
}

void Main_Out::init_connect() {
    connect(thread_out, SIGNAL(sendMat(Mat)), this, SLOT(receivemat(Mat)), Qt::BlockingQueuedConnection);
    connect(thread_out, SIGNAL(sendImg(QImage)), this, SLOT(receiveimg(QImage)));
    // 客户与服务器交互线程发送车辆出场的信号，触发车辆出场信息功能的槽函数
    connect(Thread::getInstance(), SIGNAL(out(PACK_EXIT_BACK)), this, SLOT(out_msg(PACK_EXIT_BACK)));
}

void Main_Out::receiveimg(QImage img) {
    this->Image =img;
    this->update();
}

void Main_Out::receivemat(Mat mat) {
    this->mat = mat.clone();
}

void Main_Out::paintEvent(QPaintEvent*) {
    if (!this->Image.isNull()) {
        // Video_label图片更新
        this->pixmap = QPixmap::fromImage(this->Image);
        this->fitpixmap = this->pixmap.scaled(ui->outVideo_label->width(),
                                              ui->outVideo_label->height(),
                                              Qt::IgnoreAspectRatio,
                                              Qt::SmoothTransformation);
        // 控件接收图片
        ui->outVideo_label->setPixmap(fitpixmap);
        ui->outVideo_label->setScaledContents(true);
    }
}
// 车牌识别按钮槽函数
void Main_Out::on_carNum_btn_clicked() {
    this->plateVec.clear();
    this->pr.plateRecognize(this->mat, this->plateVec);
    if (this->plateVec.size() > 0) {
        // 1.车牌识别
        this->plate = plateVec.at(0);
        this->plateMat = this->plate.getPlateMat();

        cvtColor(this->plateMat, this->plateMat_car, CV_BGR2RGB);
        this->car_image = QImage(this->plateMat_car.data,
                                 this->plateMat_car.cols,
                                 this->plateMat_car.rows,
                                 QImage::Format_RGB888);
        // 2.控件显示车牌图片
        QPixmap *image = new QPixmap(QPixmap::fromImage(this->car_image));
        image->scaled(ui->car_Image_label->size(), Qt::KeepAspectRatio);
        ui->car_Image_label->setPixmap(QPixmap::fromImage(this->car_image));

        // 3.控件显示车牌号
        string liences = this->plate.getPlateStr();
        QString carInfo = QString::fromStdString(liences).mid(3, -1);
        ui->carNum_edit->setText(carInfo);

        // 4.控件显示出场时间
        this->date_time = QDateTime::currentDateTime();
        QString cur_date = date_time.toString("yyyy-MM-dd hh:mm:ss");
        ui->outTime_label_2->setText(cur_date);

        // 5.图片存本地
        QString path = SingleDB::getInstance()->get_image_path() + "/" + cur_date;
        this->car_image.save(path, "JPG", -1);

        // 6.发到服务器
        PACK_HEAD head;
        PACK_EXIT car_out;
        char buffer[sizeof(PACK_HEAD) + sizeof(PACK_EXIT)] = { 0 };
        memset(&head, 0, sizeof(PACK_HEAD));
        memset(&car_out, 0, sizeof(PACK_EXIT));

        head.bodyType = CAR_GETOUT; // 出场包
        // 将标签控件里面内容赋值给结构体
        strcpy(car_out.car_number, ui->carNum_edit->text().toStdString().c_str());
        strcpy(car_out.out_time, ui->outTime_label_2->text().toStdString().c_str());

        // 拷贝包头+包体，发送到服务器
        memcpy(buffer, &head, sizeof(PACK_HEAD));
        memcpy(buffer + sizeof(PACK_HEAD), &car_out, sizeof(PACK_EXIT));
        write(SingleDB::getInstance()->get_fd(), buffer, sizeof(PACK_HEAD) + sizeof(PACK_EXIT));
        qDebug() << "fd:" << SingleDB::getInstance()->get_fd() << " client send: getout\n";
    }
}
// 显示车辆出场信息的槽函数
void Main_Out::out_msg(PACK_EXIT_BACK msg) {
    ui->inTime_edit->setText(msg.in_time);
    ui->stayTime_edit->setText(msg.total_time);
    ui->price_label->setText("收费：" + QString::number(msg.money) + "元");
    if (msg.vip) {
        ui->radioButton->setCheckable(1);
    } else {
        ui->radioButton->setCheckable(0);
    }
}
// 放行按钮的槽函数
void Main_Out::on_out_btn_clicked() {
    Main_In::car_num++;
    emit update_car_amount(Main_In::car_num);
}
~~~

### 3、基于单例模式实现客户端数据库连接

#### 3.1 创建client.db数据库，以及car、video数据表

![img](https://s2.loli.net/2022/05/25/CWbcMne61lLivHB.png)

#### 3.2 singledb_sqlite.h 

~~~c++
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
~~~

#### 3.3 singledb_sqlite.cpp 

~~~c++
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
~~~

#### 3.4 car_model.h

~~~c++
#ifndef CAR_MODEL_H
#define CAR_MODEL_H

#include <QString>
#include <QDebug>
#include "Sqlite/singledb_sqlite.h"

class Car_Model {
public:
    static Car_Model *getInstance();

    // 获取车辆入场信息
    char **get_in_car(QString date);

    int add_car(QString car_num, QString in_date, QString in_time, QString in_location);
    int change_car(QString old_car_num, QString new_car_num);
    char **get_date();
    char **get_day();
    char **get_video_by_date(QString date, int num);
    char **get_video_by_day(QString day, int num);
    char **get_last_car_num();
    int add_video(QString video_name, QString add_date, QString add_day);
    char **get_video(int num);

    int get_row() { return row; }
    int get_col() { return col; }
private:
    Car_Model();                // 构造私有化
    static Car_Model *moder;    // 静态私成员变量
    int row, col;
};

#endif // CAR_MODEL_H

~~~

#### 3.5 car_model.cpp

~~~c++
#include "car_model.h"

Car_Model *Car_Model::moder = nullptr;

Car_Model::Car_Model() { }

Car_Model *Car_Model::getInstance() {
    if(Car_Model::moder == nullptr) {
        Car_Model::moder = new Car_Model;
    }
    return Car_Model::moder;
}
// 查询获取车辆入场信息
char **Car_Model::get_in_car(QString date) {
    char sql[256];
    sprintf(sql, "select car_num,in_time,in_location from car where in_date='%s';",
            date.toStdString().c_str());
    char **qres;
    int res = Singledb_Sqlite::getInstance()->dosql(sql, qres, row, col);
    if (res == 0) {
        return qres;
    }
    return nullptr;
}
// 插入车辆入场信息
int Car_Model::add_car(QString car_num, QString in_date, QString in_time, QString in_location) {
    char sql[256];
    sprintf(sql, "insert into car(car_num,in_date,in_time,in_location)values('%s','%s','%s','%s');",
            car_num.toStdString().c_str(),
            in_date.toStdString().c_str(),
            in_time.toStdString().c_str(),
            in_location.toStdString().c_str());
    char **qres;
    return Singledb_Sqlite::getInstance()->dosql(sql, qres, row, col);
}
// 更新车牌号
int Car_Model::change_car(QString old_car_num, QString new_car_num) {
    char sql[256];
    sprintf(sql, "update car set car_num='%s' where car_num='%s';",
            new_car_num.toStdString().c_str(),
            old_car_num.toStdString().c_str());
    char **qres;
    return Singledb_Sqlite::getInstance()->dosql(sql, qres, row, col);
}
// 查询获取视频数据库的时间
char **Car_Model::get_date() {
    char sql[256];
    sprintf(sql, "select distinct add_date from video;");
    char **qres;
    int res = Singledb_Sqlite::getInstance()->dosql(sql, qres, row, col);
    if(res == 0) {
        return qres;
    }
    return nullptr;
}
// 查询获取视频数据库的日期
char **Car_Model::get_day() {
    char sql[256];
    sprintf(sql, "select distinct add_day from video;");
    char **qres;
    int res = Singledb_Sqlite::getInstance()->dosql(sql, qres, row, col);
    if(res == 0) {
        return qres;
    }
    return nullptr;
}
// 通过时间查询视频名称
char **Car_Model::get_video_by_date(QString date, int num) {
    char sql[256];
    sprintf(sql, "select video_name from video where add_date = '%s' limit 9 offset (%d-1)*9;",
            date.toStdString().c_str(),
            num);
    char **qres;
    int res = Singledb_Sqlite::getInstance()->dosql(sql, qres, row, col);
    if(res == 0) {
        return qres;
    }
    return nullptr;
}
// 通过日期查询视频名称
char **Car_Model::get_video_by_day(QString day, int num) {
    char sql[256];
    sprintf(sql, "select video_name from video where add_day = '%s' limit 9 offset (%d-1)*9;",
            day.toStdString().c_str(),
            num);
    char **qres;
    int res = Singledb_Sqlite::getInstance()->dosql(sql, qres, row, col);
    if(res == 0) {
        return qres;
    }
    return nullptr;
}
// 查询获取最后一辆车的进场信息
char **Car_Model::get_last_car_num() {
    char sql[256];
    sprintf(sql, "select car_num,in_time,in_location from car order by id desc limit 1;");
    char **qres;
    int res = Singledb_Sqlite::getInstance()->dosql(sql, qres, row, col);
    if(res == 0) {
        return qres;
    }
    return nullptr;
}
// 插入视频信息
int Car_Model::add_video(QString video_name, QString add_date, QString add_day) {
    char sql[256];
    sprintf(sql, "insert into video(video_name,add_date,add_day)values('%s','%s','%s');",
            video_name.toStdString().c_str(),
            add_date.toStdString().c_str(),
            add_day.toStdString().c_str());
    char **qres;
    return Singledb_Sqlite::getInstance()->dosql(sql,qres,row,col);
}
~~~

### 4、实现停车场内部管理

#### 4.1 UI界面

![img](https://s2.loli.net/2022/05/24/us9RbTKNavJZCBV.png)

#### 4.2 Qt线程实现视频帧处理

##### thread_inmanage.h 头文件

~~~c++
#ifndef THREAD_INMANAGE_H
#define THREAD_INMANAGE_H

#include <QObject>
#include <QThread>
#include <QPainter>
#include <opencv2/opencv.hpp>
using namespace cv;
#define MANUAL 1 // 手动录制
#define AUTO   2 // 自动录制
#define STOP   0 // 无录制

class Thread_Inmanage : public QThread {
    Q_OBJECT
public:
    Thread_Inmanage();

    void video_save();
    static int state;
private:
    VideoCapture capture;
    Mat picture, img_RGB;
    QImage img;
    VideoWriter write;					// 视频录制的对象

    int counter, first;
    QString time, date, day, path;

    CascadeClassifier cascade, mancascade;

    void detectCarDraw(Mat &img, CascadeClassifier &cascade, double scale);
protected:
    void run();
signals:
    void btn_enable();
    void sendImg(QImage img);
    void sendMat(Mat mat);
};

#endif // THREAD_INMANAGE_H
~~~

##### thread_inmanage.cpp 源文件

~~~c++
#include "thread_inmanage.h"
#include <QDateTime>

#include "Tool/singledb.h"
#include "Sqlite/car_model.h"
#include "Widget/main_inmanage.h"

int Thread_Inmanage::state = 0;		// 视频录制状态

Thread_Inmanage::Thread_Inmanage() {
    this->capture.open("../LaiClient/video.mp4");
    this->cascade.load("../LaiClient/data/cars.xml");
    this->counter = 0;
}
// 打开视频录制存储路径并初始化
void Thread_Inmanage::video_save() {
    QDateTime date_time = QDateTime::currentDateTime();
    time = date_time.toString("yyyy-MM-dd hh:mm:ss");
    date = date_time.toString("yyyy-MM");
    day = date_time.toString("yyyy-MM-dd");
    // get_video_path:获取保存视频的地址
    path = SingleDB::getInstance()->get_video_path() + "/" + time + ".avi";
    // 打开视频录制存储路径
    // 第1个参数：读取的视频帧所存放的新的文件
    // 第2个参数：视频存放的编码格式
    // 第3个参数：每秒的帧数
    // 第4个参数：图像的长宽大小
    this->write.open(path.toStdString().c_str(), CV_FOURCC('M', 'J', 'P', 'G'), 25.0, Size(img.width(), img.height()), true);
}

void Thread_Inmanage::detectCarDraw(Mat &img, CascadeClassifier &cascade, double scale) {
    vector<Rect> cars;
    Mat grayImg, smallImg(cvRound(img.rows/scale), cvRound(img.cols/scale), CV_8UC1);
    // 1.先转灰度
    cvtColor(img,grayImg,CV_BGR2GRAY);
    // 2.再把图片缩小一半
    resize(grayImg,smallImg,smallImg.size(),0,0);
    // 3.直方图均值化
    equalizeHist(smallImg,smallImg);
    // 4.开始利用级联分类器进行识别物体
    cascade.detectMultiScale(smallImg, cars, 1.1, 3, 0|CV_HAAR_SCALE_IMAGE, Size(30, 30));
    for (vector<Rect>::const_iterator r = cars.begin(); r != cars.end(); r++) {
        // 在原始图片中绘制，不是在smallimg中绘制
        rectangle(img,cvPoint(r->x*scale,r->y*scale),
                  cvPoint((r->x + r->width)*scale, (r->y + r->height)*scale),
                  Scalar(0, 255, 0), 3);
    }
}

void Thread_Inmanage::run() {
    while (capture.read(picture)) {
        if (picture.data) {
            detectCarDraw(picture, cascade, 2);
            cvtColor(picture,img_RGB, CV_BGR2RGB);
            img = QImage(img_RGB.data, img_RGB.cols, img_RGB.rows, QImage::Format_RGB888);
            emit sendImg(img);
            // 如果为自动录制并且视频帧数为0时，打开视频录制存储路径并初始化
            if(state == AUTO && counter == 0) {
                video_save();
            }
            // 自动录制或手动录制
            if(state == MANUAL || state == AUTO) {
                if(first == 0) {
                    img.save(SingleDB::getInstance()->get_video_path() + "/" + time + ".jpg");
                    first = 1;
                }
                // 录制200帧
                if (counter < 200){
                    write.write(picture);
                    counter++;
                }
            }
            // 录制完200帧，将录制视频的信息插入到数据库video表中
            if(counter == 200) {
                Car_Model::getInstance()->add_video(time, date, day);
                emit btn_enable();		// 激活按键
                // 手动录制停止
                if(state == MANUAL) {
                    state = STOP;
                }
                counter = 0;	// 初始化
                first = 0;		// 初始化
            }
            emit sendMat(picture);
        }
    }
}
~~~

#### 4.3 实现停车场内部管理

##### main_inmanage.h 头文件

~~~c++
#ifndef MAIN_INMANAGE_H
#define MAIN_INMANAGE_H

#include <QWidget>
#include <unistd.h>
#include <QTimer>

#include "Tool/package.h"
#include "Thread/thread.h"
#include "Thread/thread_in.h"
#include "Thread/thread_inmanage.h"
using namespace cv;
using namespace easypr;

#define MANUAL 1 // 手动录制
#define AUTO   2 // 自动录制
#define STOP   0 // 无录制

namespace Ui {
    class Main_Inmanage;
}

class Main_Inmanage : public QWidget {
    Q_OBJECT
public:
    explicit Main_Inmanage(QWidget *parent = nullptr);
    ~Main_Inmanage();

    void init_connect();
    void update_car();

    Thread_Inmanage *thread_inmanage;
private:
    Ui::Main_Inmanage *ui;

    CPlateRecognize pr;
    vector <CPlate> plateVec;
    CPlate plate;

    Mat plateMat, plateMat_car;

    CascadeClassifier cascade;
    list<QString> queue_pathstr;
    QImage car_image;

    VideoCapture cap;
    VideoWriter write;
    QString time;
    static int mark;
    int counter;  	// 记录帧数
    int state;

    Mat frame, mat;
    QImage Image;

    QPixmap pixmap, fitpixmap;

    FILE*fp;
    PACK_HEAD head_photo;
    PACK_PHOTO photo;
protected:
    void paintEvent(QPaintEvent*);
private slots:
    void receiveimg(QImage img);
    void receivemat(Mat mat);

    void on_record_btn_clicked();
    void on_auto_btn_toggled(bool checked);
    void btn_enabel();

    void on_snap_btn_clicked();
    void photo_send(int result);
};

#endif // MAIN_INMANAGE_H
~~~

##### main_inmanage.cpp 源文件

~~~c++
#include "main_inmanage.h"
#include "ui_main_inmanage.h"

#include <QMessageBox>
#include <QDateTime>
#include <sys/stat.h>
#include "Tool/singledb.h"
#include "Widget/main_in.h"

int Main_Inmanage::mark = 0;	// 抓拍标记

Main_Inmanage::Main_Inmanage(QWidget *parent) : QWidget(parent), ui(new Ui::Main_Inmanage) {
    ui->setupUi(this);
    ui->carTotal_label->setText(QString("目前车场总车辆：") + QString::number(Main_In::car_num));
    int freeParking = 500 - Main_In::car_num;
    ui->freePark_label->setText(QString("目前车场空闲车位：") + QString::number(freeParking));
    this->counter = 0;
    
    this->pr.setDetectType(PR_DETECT_CMSER | PR_DETECT_COLOR);
    this->pr.setResultShow(false);
    this->pr.setMaxPlates(4);

    this->cascade.load("../LaiClient/data/cars.xml");

    this->thread_inmanage = new Thread_Inmanage();
    init_connect();
    this->thread_inmanage->start();
}

Main_Inmanage::~Main_Inmanage() {
    delete ui;
}

void Main_Inmanage::init_connect() {
    connect(thread_inmanage, SIGNAL(sendImg(QImage)), this, SLOT(receiveimg(QImage)));
    connect(thread_inmanage, SIGNAL(sendMat(Mat)), this, SLOT(receivemat(Mat)), Qt::BlockingQueuedConnection);
    // 连接激活按键的信号与槽函数
    connect(thread_inmanage, SIGNAL(btn_enable()), this, SLOT(btn_enabel()));
    connect(Thread::getInstance(), SIGNAL(photo_continue_in(int)),this, SLOT(photo_send(int)));
}
// 更新停车场内部的车辆和车位数
void Main_Inmanage::update_car() {
    ui->carTotal_label->setText(QString("目前车场总车辆：") + QString::number(Main_In::car_num));
    int freeParking = 500 - Main_In::car_num;
    ui->freePark_label->setText(QString("目前车场空闲车位：") + QString::number(freeParking));
}

void Main_Inmanage::receiveimg(QImage img) {
    this->Image = img;
    this->update();
}

void Main_Inmanage::receivemat(Mat mat) {
    this->mat = mat.clone();
}

void Main_Inmanage::paintEvent(QPaintEvent*) {
    if (!this->Image.isNull()) {
        pixmap = QPixmap::fromImage(Image);
        fitpixmap = pixmap.scaled(ui->video_label->width(),
                                  ui->video_label->height(),
                                  Qt::IgnoreAspectRatio,
                                  Qt::SmoothTransformation);
        //控件接收图片
        ui->video_label->setPixmap(fitpixmap);
        ui->video_label->setScaledContents(true);
    }
}
// 手动录制按钮槽函数
void Main_Inmanage::on_record_btn_clicked() {
    ui->record_btn->setEnabled(false);
    ui->auto_btn->setEnabled(false);
    thread_inmanage->video_save();		// 打开视频录制存储路径并初始化
    thread_inmanage->state = MANUAL;	// 手动录制状态
}
// 自动录制按钮槽函数
void Main_Inmanage::on_auto_btn_toggled(bool checked) {
    if (checked == true) {
        thread_inmanage->state = AUTO;
    } else {
        thread_inmanage->state = STOP;
    }
}
// 激活按钮
void Main_Inmanage::btn_enabel() {
    ui->record_btn->setEnabled(true);
    ui->auto_btn->setEnabled(true);
}
// 车辆特征抓拍按钮槽函数
void Main_Inmanage::on_snap_btn_clicked() {
    this->mark = 1;
    this->plateVec.clear();
    this->pr.plateRecognize(this->mat, this->plateVec);
    if (plateVec.size() > 0) {
        // 1.车牌识别
        this->plate = this->plateVec.at(0);
        this->plateMat = this->plate.getPlateMat();

        cvtColor(this->plateMat, this->plateMat_car, CV_BGR2RGB);
        this->car_image = QImage(this->plateMat_car.data,
                                 this->plateMat_car.cols,
                                 this->plateMat_car.rows,
                                 QImage::Format_RGB888);

        // 2.图片存本地
        QString path_in = SingleDB::getInstance()->get_image_path() + "/" + time + "_big2";
        Image.save(path_in, "JPG", -1);
        queue_pathstr.push_back(path_in);

        // 3.传图片到服务器
        usleep(20000);
        head_photo.bodyType = PHOTO_TYPE;
        char buff[sizeof(PACK_HEAD) + sizeof(PACK_PHOTO)] = { 0 };
        if(this->queue_pathstr.empty()) {
            return;
        }
        fp = fopen(this->queue_pathstr.front().toStdString().c_str(), "rb");
        // 获取文件的大小
        fseek(fp, 0, SEEK_END);
        double size = ftell(fp);
        size /= 1024;
        photo.sum = ceil(size);

        QDateTime date_time = QDateTime::currentDateTime();
        QString save_name = date_time.toString("yyyy_MM_dd_hh_mm_ss") + ".jpg";
        strcpy(photo.filename, save_name.toStdString().c_str());

        photo.num = 1;
        memcpy(buff, &head_photo, sizeof (PACK_HEAD));
        fseek(fp, 0, SEEK_SET);

        photo.realSize = fread(photo.context, 1, 1024, fp);
        memcpy(buff + sizeof(PACK_HEAD), &photo, sizeof(PACK_PHOTO));
        this->queue_pathstr.pop_front();
        ::write(SingleDB::getInstance()->get_fd(), buff, sizeof(PACK_HEAD) + sizeof(PACK_PHOTO));
        qDebug() << "fd:" << SingleDB::getInstance()->get_fd() << " client send: photo\n";
    }
}

void Main_Inmanage::photo_send(int result) {
    if (mark == 1) {
        if (result == 1) {
            char buffer[sizeof(PACK_HEAD) + sizeof(PACK_PHOTO)] = { 0 };
            memcpy(buffer,&head_photo, sizeof (PACK_HEAD));
            photo.realSize = fread(photo.context, 1, 1024, fp);
            if (photo.realSize == 0) {
                fclose(fp);
                mark = 0;
                QMessageBox::warning(this, tr("提示！"), tr("图片发送成功！"), QMessageBox::Yes);
                return;
            }
            photo.num++;
            memcpy(buffer+sizeof(PACK_HEAD), &photo, sizeof(PACK_PHOTO));
            ::write(SingleDB::getInstance()->get_fd(), buffer, sizeof(PACK_HEAD) + sizeof(PACK_PHOTO));
        } else {
            fclose(fp);
        }
    }
}
~~~

### 5、实现停车场车辆信息查询

#### 5.1 UI界面

![img](https://s2.loli.net/2022/05/24/5McOy7vzZTgDCLx.png)

#### 5.2 main_equery.h 头文件

~~~c++
#ifndef MAIN_EQUERY_H
#define MAIN_EQUERY_H

#include <QWidget>
#include "Thread/thread.h"

namespace Ui {
    class Main_Equery;
}

class Main_Equery : public QWidget {
    Q_OBJECT
public:
    explicit Main_Equery(QWidget *parent = nullptr);
    ~Main_Equery();

    void init_connect();
    void send_msg();
private:
    Ui::Main_Equery *ui;

    int frequency, num;
private slots:
    void on_equery_btn_clicked();
    void on_more_btn_clicked();
    void show_msg(PACK_ALLCARMSG_BACK result);
    void on_export_btn_clicked();
};

#endif // MAIN_EQUERY_H
~~~

#### 5.3 main_equery.cpp 源文件

~~~c++
#include "main_equery.h"
#include "ui_main_equery.h"

#include <QMessageBox>
#include <QMetaType>
#include <QFile>
#include <QTextStream>
#include <QDateTime>

#include "Tool/singledb.h"

Main_Equery::Main_Equery(QWidget *parent) : QWidget(parent), ui(new Ui::Main_Equery) {
    qRegisterMetaType<PACK_ALLCARMSG_BACK>("PACK_ALLCARMSG_BACK");
    ui->setupUi(this);

    this->num = 1;
    ui->tableWidget->setColumnCount(7);
    QStringList header;
    header << "序号" << "车牌号码" << "入场时间" << "出场时间" <<
              "入场图片" << "出场图片" << "停车金额";
    ui->tableWidget->setHorizontalHeaderLabels(header);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->tableWidget->setColumnWidth(0, 20);
    ui->tableWidget->setColumnWidth(1, 170);
    ui->tableWidget->setColumnWidth(2, 170);
    ui->tableWidget->setColumnWidth(3, 170);
    ui->tableWidget->setColumnWidth(4, 170);
    ui->tableWidget->setColumnWidth(5, 170);

    // 倒序插入数据
    ui->tableWidget->sortByColumn(1, Qt::DescendingOrder);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(6, QHeaderView::Stretch);
    ui->tableWidget->verticalHeader()->setVisible(false);
    ui-> tableWidget->setAlternatingRowColors(true);

    init_connect();
}

Main_Equery::~Main_Equery() {
    delete ui;
}

void Main_Equery::init_connect() {
    // 客户与服务器交互线程发送车辆信息的信号，触发显示车辆信息的槽函数
    connect(Thread::getInstance(), SIGNAL(car_msg(PACK_ALLCARMSG_BACK)),this, SLOT(show_msg(PACK_ALLCARMSG_BACK)));
}
// 发送车辆信息给服务器
void Main_Equery::send_msg() {
    // 发到服务器
    PACK_HEAD head;
    PACK_CARMSG car_equery;
    char buffer[sizeof(PACK_HEAD) + sizeof(PACK_CARMSG)] = { 0 };

    memset(&head, 0, sizeof(PACK_HEAD));
    memset(&car_equery, 0, sizeof(PACK_CARMSG));

    head.bodyType = CAR_MSG_TYPE; // 车辆信息包
	// 车辆信息（车牌号，入场时间，出场时间，页数）
    strcpy(car_equery.car_number, ui->carNUM_edit->text().toStdString().c_str());
    strcpy(car_equery.in_time, ui->inTime_edit->text().toStdString().c_str());
    strcpy(car_equery.out_time, ui->outTime_edit->text().toStdString().c_str());
    car_equery.page = this->frequency;

    memcpy(buffer, &head, sizeof(PACK_HEAD));
    memcpy(buffer + sizeof(PACK_HEAD), &car_equery, sizeof(PACK_CARMSG));

    write(SingleDB::getInstance()->get_fd(), buffer, sizeof(PACK_HEAD) + sizeof(PACK_CARMSG));
    qDebug() << "fd:" << SingleDB::getInstance()->get_fd() << " client send: carmsg\n";
}
// 在tablewidget上显示车辆信息
void Main_Equery::show_msg(PACK_ALLCARMSG_BACK result) {
    if (result.realCount == 0) {
        QMessageBox::warning(this, tr("提示！"), tr("没有内容！"), QMessageBox::Yes);
        return;
    }
    QTableWidgetItem *item[5];
    for (int i = 0; i < result.realCount; ++i) {
        ui->tableWidget->insertRow(0);
        ui->tableWidget->setRowHeight(0, 30);

        // 序号
        item[0] = new QTableWidgetItem(QString("%1").arg(num++));
        ui->tableWidget->setItem(0, 0, item[0]);

        // 车牌号码
        item[1] = new QTableWidgetItem(QString(result.arr[i].car_number));
        ui->tableWidget->setItem(0, 1, item[1]);

        // 入场时间
        item[2] = new QTableWidgetItem(QString(result.arr[i].in_time));
        ui->tableWidget->setItem(0, 2, item[2]);

        // 出场时间
        item[3] = new QTableWidgetItem(QString(result.arr[i].out_time));
        ui->tableWidget->setItem(0, 3, item[3]);

        // 入场图片
        QLabel *in_label = new QLabel();
        in_label->setPixmap(QPixmap(SingleDB::getInstance()->get_image_path()+ "/" + result.arr->in_time));
        ui->tableWidget->setCellWidget(0, 4, in_label);

        // 出场图片
        QLabel *out_label = new QLabel();
        out_label->setPixmap(QPixmap(SingleDB::getInstance()->get_image_path()+ "/" + result.arr->out_time));
        ui->tableWidget->setCellWidget(0, 5, out_label);

        // 停车金额
        item[4] = new QTableWidgetItem(QString::number(result.arr[i].money));
        ui->tableWidget->setItem(0, 6, item[4]);
    }
}
// 查询按钮槽函数
void Main_Equery::on_equery_btn_clicked() {
    ui->tableWidget->setRowCount(0);
    this->num = 1;
    this->frequency = 0;
    send_msg();
}
// 加载更多按钮槽函数
void Main_Equery::on_more_btn_clicked() {
    this->frequency++;
    send_msg();
}
// 数据导出按钮槽函数
void Main_Equery::on_export_btn_clicked() {
    QDateTime date_time = QDateTime::currentDateTime();
    QString date = date_time.toString("yyyy-MM-dd hh:mm:ss");

    QFile file(QString("../LaiClient/data/") + date + QString("_date.txt"));
    // 已写方式打开文件， 如果文件不存在会自动创建文件
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, tr("提示！"), tr("导出失败！"), QMessageBox::Yes);
        return;
    } else {
        QTextStream in(&file);
        for (int i = 0; i < ui->tableWidget->rowCount(); ++i) {
            for (int j = 0; j < 7; ++j) {
                if (j == 4 || j == 5) { continue; }
                in << ui->tableWidget->item(i, j)->text() << " ";
            }
            in << "\n";
        }
        file.close();
    }
    QMessageBox::warning(this, tr("提示！"), tr("导出成功！"), QMessageBox::Yes);
}
~~~

