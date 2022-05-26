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
