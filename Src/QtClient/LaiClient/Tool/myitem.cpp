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
