#ifndef MAIN_INMANAGE_H
#define MAIN_INMANAGE_H

#include <QWidget>
#include <unistd.h>
#include <QTimer>

#include "Tool/package.h"
#include "Thread/thread.h"
#include "Thread/thread_in.h"
#include "Thread/thread_inmanage.h"

using namespace cv;
using namespace easypr;

#define MANUAL 1 // 手动录制
#define AUTO   2 // 自动录制
#define STOP   0 // 无录制

namespace Ui {
    class Main_Inmanage;
}

class Main_Inmanage : public QWidget {
    Q_OBJECT

public:
    explicit Main_Inmanage(QWidget *parent = nullptr);
    ~Main_Inmanage();

    void init_connect();
    void update_car();

    Thread_Inmanage *thread_inmanage;

private:
    Ui::Main_Inmanage *ui;

    CPlateRecognize pr;
    vector <CPlate> plateVec;
    CPlate plate;

    Mat plateMat, plateMat_car;

    CascadeClassifier cascade;
    list<QString> queue_pathstr;
    QImage car_image;

    VideoCapture cap;
    VideoWriter write;
    QString time;
    static int mark;
    int counter;  //记录帧数
    int state;

    Mat frame, mat;
    QImage Image;

    QPixmap pixmap, fitpixmap;

    FILE*fp;
    PACK_HEAD head_photo;
    PACK_PHOTO photo;

protected:
    void paintEvent(QPaintEvent*);

private slots:
    void receiveimg(QImage img);
    void receivemat(Mat mat);

    void on_record_btn_clicked();
    void on_auto_btn_toggled(bool checked);
    void btn_enabel();

    void on_snap_btn_clicked();
    void photo_send(int result);

};

#endif // MAIN_INMANAGE_H
