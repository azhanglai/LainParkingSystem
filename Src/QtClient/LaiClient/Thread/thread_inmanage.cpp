#include "thread_inmanage.h"
#include <QDateTime>

#include "Tool/singledb.h"
#include "Sqlite/car_model.h"
#include "Widget/main_inmanage.h"

int Thread_Inmanage::state = 0;

Thread_Inmanage::Thread_Inmanage() {
    this->capture.open("../LaiClient/video.mp4");
    this->cascade.load("../LaiClient/data/cars.xml");
    this->counter = 0;
}

void Thread_Inmanage::video_save() {
    QDateTime date_time = QDateTime::currentDateTime();
    time = date_time.toString("yyyy-MM-dd hh:mm:ss");
    date = date_time.toString("yyyy-MM");
    day = date_time.toString("yyyy-MM-dd");
    path = SingleDB::getInstance()->get_video_path() + "/" + time + ".avi";
    this->write.open(path.toStdString().c_str(), CV_FOURCC('M', 'J', 'P', 'G'), 25.0, Size(img.width(), img.height()), true);
}

void Thread_Inmanage::detectCarDraw(Mat &img, CascadeClassifier &cascade, double scale) {
    //我们使用级联分类器前有一套标准处理流程，这个处理流程目的是加快计算机识别物体的速度。
    //对于原始视频来说，每张图片都非常大
    vector<Rect> cars;
    Mat grayImg, smallImg(cvRound(img.rows/scale), cvRound(img.cols/scale), CV_8UC1);
    // 1.先转灰度
    cvtColor(img,grayImg,CV_BGR2GRAY);
    // 2.再把图片缩小一半
    resize(grayImg,smallImg,smallImg.size(),0,0);
    // 3.直方图均值化
    equalizeHist(smallImg,smallImg);
    // 4.开始利用级联分类器进行识别物体
    // cars:表示通过级联分类器识别完成之后会把数据存放到cars中
    // 1.1表示:每次识别后图像矩形框扩大1.1
    // 3.表示:opencv识别物体的时候最少检测3次才是算是目标
    // 4.Size(30,30)能识别的物体最小必须大于30*30像素。
    cascade.detectMultiScale(smallImg, cars, 1.1, 3, 0|CV_HAAR_SCALE_IMAGE, Size(30, 30));
    for (vector<Rect>::const_iterator r = cars.begin(); r != cars.end(); r++) {
        // 在原始图片中绘制，不是在smallimg中绘制
        rectangle(img,cvPoint(r->x*scale,r->y*scale),
                  cvPoint((r->x + r->width)*scale, (r->y + r->height)*scale),
                  Scalar(0, 255, 0), 3);
    }
}

void Thread_Inmanage::run() {
    while (capture.read(picture)) {
        if (picture.data) {
            detectCarDraw(picture, cascade, 2);
            cvtColor(picture,img_RGB, CV_BGR2RGB);
            img = QImage(img_RGB.data, img_RGB.cols, img_RGB.rows, QImage::Format_RGB888);
            emit sendImg(img);
            if(state == AUTO && counter == 0) {
                video_save();
            }
            if(state == MANUAL || state == AUTO) {
                if(first == 0) {
                    img.save(SingleDB::getInstance()->get_video_path() + "/" + time + ".jpg");
                    first = 1;
                }
                //只保存750帧
                if (counter < 200){
                    write.write(picture);
                    counter++;
                }
            }
            if(counter == 200) {
                Car_Model::getInstance()->add_video(time, date, day);
                emit btn_enable();
                if(state == MANUAL) {
                    state = STOP;
                }
                counter = 0;
                first = 0;
            }
            emit sendMat(picture);
        }
    }
}
