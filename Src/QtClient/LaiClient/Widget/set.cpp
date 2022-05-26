#include "set.h"
#include "ui_set.h"
#include <QTextStream>
#include <QMessageBox>
#include <QRegExpValidator>
#include <QFileDialog>

#include "Tool/singledb.h"
#include "Thread/thread.h"
#include "Widget/login.h"

Set::Set(QWidget *parent) : QWidget(parent), ui(new Ui::Set) {
    ui->setupUi(this);

    init_connect();
}

Set::~Set() {
    delete ui;
}

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
         //已读写方式打开文件， 如果文件不存在会自动创建文件
         if(!file.open(QIODevice::WriteOnly)) {
             return;
         } else {
            QTextStream in(&file);
            in << ui->IP_edit->text() << "\n";
            in << ui->Port_edit->text() << "\n";
            in << ui->ImagePath_edit->text() << "\n";
            in << ui->VideoPath_edit->text() << "\n";
            file.close();
         }
         int res = SingleDB::getInstance()->socket_connect();
         Thread::getInstance()->start();
         if(res == 0) {
             qDebug() << "ok\n";
             QMessageBox::warning(this, tr("提示！"), tr("连接服务器成功！"), QMessageBox::Yes);
             Login *loginwin = new Login();
             this->hide();
             loginwin->show();
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
