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

int Main_In::mark = 0;
int Main_In::car_num = 0;

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
    connect(thread_in, SIGNAL(sendMat(Mat)), this, SLOT(receivemat(Mat)), Qt::BlockingQueuedConnection);
    connect(thread_in, SIGNAL(sendImg(QImage)), this, SLOT(receiveimg(QImage)));
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

void Main_In::receiveimg(QImage img) {
    this->Image = img;
    this->update();
}

void Main_In::receivemat(Mat mat) {
    this->mat = mat.clone();
}

// 画面显示绘制事件
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
