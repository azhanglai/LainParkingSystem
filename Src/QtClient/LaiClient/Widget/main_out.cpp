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

void Main_Out::on_out_btn_clicked() {
    Main_In::car_num++;
    emit update_car_amount(Main_In::car_num);
}
