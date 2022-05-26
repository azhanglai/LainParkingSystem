#include "main_playback.h"
#include "ui_main_playback.h"

#include <QMessageBox>
#include "Widget/main_replay.h"
#include "Tool/singledb.h"
#include "Sqlite/car_model.h"

Main_Playback::Main_Playback(QWidget *parent) : QWidget(parent), ui(new Ui::Main_Playback) {
    ui->setupUi(this);

    this->num = 1;
    QString page = QString::number(num);
    ui->page_label->setText("第" + page + "页");

    init_connect();

    // 设置QListWidget的显示模式
    ui->listWidget_2->setViewMode(QListView::IconMode);
    // 设置QListWidget中单元项的间距
    ui->listWidget_2->setSpacing(4);
    // 设置自动适应布局调整
    ui->listWidget_2->setResizeMode(QListWidget::Adjust);
    // 设置不能移动
    ui->listWidget_2->setMovement(QListWidget::Static);
    ui->listWidget_2->setStyleSheet("border:none;font-size:15px");
    ui->listWidget_2->setIconSize(QSize(230, 150));

}

Main_Playback::~Main_Playback() {
    delete ui;
}

void Main_Playback::init_connect() {
    connect(ui->listWidget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(click_date(QListWidgetItem*)));
    connect(ui->listWidget_2, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(click_video(QListWidgetItem*)));
}

void Main_Playback::dateOrday() {
    this->num = 1;
    ui->listWidget_2->clear();
    ui->listWidget->clear();

    char **qres;
    if (state == DATE) {
        qres = Car_Model::getInstance()->get_date();
    } else {
        qres = Car_Model::getInstance()->get_day();
    }
    this->row = Car_Model::getInstance()->get_row();
    this->col = Car_Model::getInstance()->get_col();
    for (int i = 0; i < row; ++i) {
        //定义QListWidgetItem对象
        QListWidgetItem *playItem = new QListWidgetItem;
        playItem->setText(qres[(i+1) * col]);
        playItem->setTextAlignment(0x0004);
        playItem->setSizeHint(QSize(150,40));   // 重新设置单元项宽度和高度
        ui->listWidget->addItem(playItem);      // 将单元项添加到QListWidget中
    }
}

int Main_Playback::show_video() {
    char **qres2;
    if(state == DATE) {
        qres2=Car_Model::getInstance()->get_video_by_date(video_date, num);
        
    } else {
        qres2=Car_Model::getInstance()->get_video_by_day(video_date, num);
    }
    this->row = Car_Model::getInstance()->get_row();
    this->col = Car_Model::getInstance()->get_col();
    if (this->row == 0 && this->col == 0) {
        return -1;
    }
    ui->listWidget_2->clear();
   
    QList<QString> nameList;
    for(int i = col; i < (row+1)*col; i++) {
        nameList << qres2[i];
    }
    for (int i = 0; i < nameList.size(); i++){
        //定义QListWidgetItem对象
        QListWidgetItem *imageItem = new QListWidgetItem;
        //为单元项设置属性
        imageItem->setIcon(QIcon(SingleDB::getInstance()->get_video_path()+ "/" + nameList[i] + ".jpg"));
        imageItem->setText(nameList.at(i));
        
        imageItem->setSizeHint(QSize(220,150)); // 重新设置单元项图片的宽度和高度
        ui->listWidget_2->addItem(imageItem);   // 将单元项添加到QListWidget中
    }
    return 0;
}

void Main_Playback::click_date(QListWidgetItem *item) {
    video_date = item->text();
    show_video();
}

void Main_Playback::click_video(QListWidgetItem *item) {
    Main_Replay *replay = new Main_Replay(SingleDB::getInstance()->get_video_path()+"/"+item->text()+".avi");
    replay->show();
}

void Main_Playback::on_month_btn_toggled(bool checked) {
    if(checked == true) {
        this->state = DATE;
        dateOrday();
    }
}

void Main_Playback::on_day_btn_toggled(bool checked) {
    if(checked == true) {
        this->state = DAY;
        dateOrday();
    }
}

void Main_Playback::on_pre_btn_clicked() {
    if(num == 1) {
        QMessageBox::warning(this, tr("提示！"),tr("已经是第一页！"),QMessageBox::Yes);
    }
    else {
        num--;
        ui->page_label->setText("第"+QString::number(num)+"页");
        show_video();

    }
}

void Main_Playback::on_next_btn_clicked() {
    num++;
    if(show_video() == -1) {
        num--;
        QMessageBox::warning(this, tr("提示！"),tr("已经是最后一页！"),QMessageBox::Yes);
    }
    else {
        ui->page_label->setText("第"+QString::number(num)+"页");
    }
}
