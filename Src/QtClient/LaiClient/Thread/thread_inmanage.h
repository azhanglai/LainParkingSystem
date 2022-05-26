#ifndef THREAD_INMANAGE_H
#define THREAD_INMANAGE_H

#include <QObject>
#include <QThread>
#include <QPainter>
#include <opencv2/opencv.hpp>

using namespace cv;
#define MANUAL 1 // 手动录制
#define AUTO   2 // 自动录制
#define STOP   0 // 无录制

class Thread_Inmanage : public QThread {
    Q_OBJECT
public:
    Thread_Inmanage();

    void video_save();
    static int state;

private:
    VideoCapture capture;
    Mat picture, img_RGB;
    QImage img;
    VideoWriter write;

    int counter, first;
    QString time, date, day, path;

    CascadeClassifier cascade, mancascade;

    void detectCarDraw(Mat &img, CascadeClassifier &cascade, double scale);

protected:
    void run();

signals:
    void btn_enable();
    void sendImg(QImage img);
    void sendMat(Mat mat);

};

#endif // THREAD_INMANAGE_H
