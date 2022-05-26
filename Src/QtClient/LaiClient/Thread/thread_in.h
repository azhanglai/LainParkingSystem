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

#endif // THREAD_IN_H
