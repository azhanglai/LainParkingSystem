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
