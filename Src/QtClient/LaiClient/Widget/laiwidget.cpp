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
    // 场景的槽函数: tologin 作用: 跳转窗口
    connect(this->item2, &MyItem::end, this, &LaiWidget::toSetMainWidget);
}

void LaiWidget::watchStatus() {
    // 判断状态图元状态，变成1就停止
    if(this->item1->status == 1) {
        this->timer->stop();
    }
}

// 设置界面
void LaiWidget::toSetMainWidget() {
    // 1.隐藏开机动画
    this->hide();
    // 2.判断文件是否存在, 如果有就跳到登录
    QFile file("../LaiClient/data/set_data.txt");
    if (file.exists()) {
        SingleDB::getInstance()->socket_connect();
        Thread::getInstance()->start();
        this->loginwin = new Login();
        this->loginwin->show();
    } else {
        this->setwin = new Set();
        this->setwin->show();
    }
}
