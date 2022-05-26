#ifndef LAIWIDGET_H
#define LAIWIDGET_H

#include <QWidget>
#include <QBrush>
#include <QIcon>
#include <QPixmap>
#include <QTimer>
#include <QGraphicsView>
#include <QGraphicsScene>

#include "Tool/myitem.h"
#include "Thread/thread.h"
#include "Widget/set.h"
#include "Widget/login.h"

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
    Set* setwin;
    Login* loginwin;

public slots:
    void watchStatus();
    void toSetMainWidget();
};

#endif // LAIWIDGET_H
