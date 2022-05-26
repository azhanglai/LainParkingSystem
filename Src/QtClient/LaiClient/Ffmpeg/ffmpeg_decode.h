#ifndef FFMPEG_DECODE_H
#define FFMPEG_DECODE_H

#include <QImage>
#include <QThread>
#include <QString>
#include <QCameraInfo>
#include <QQueue>
#include <QMutex>
#include <QMutexLocker>
#include <QList>
#include <iostream>
using namespace std;

extern "C" {
    #include "libavcodec/avcodec.h"       //编解码
    #include "libavdevice/avdevice.h"     //转码
    #include "libavfilter/avfilter.h"
    #include "libavformat/avformat.h"     //格式
    #include "libavutil/avutil.h"         //工具
    #include "libswresample/swresample.h"
    #include "libswscale/swscale.h"       //转置
}

class Ffmpeg_Decode:public QThread {
    Q_OBJECT
public:
    static Ffmpeg_Decode* getInstance();

    int open_video(QString path);
    void decode_frame();

    int Seek(float pos);
    int play();
    int pause();

    int GetTotalTimeMsec();
    int GetCurTimeMsec();

    void start(Priority = InheritPriority);
    void change_play_speed(int speed) { this->play_speed = speed; }

    QString replay_path;


private:
    Ffmpeg_Decode();
    AVFormatContext *pformatContext;
    AVCodecContext *codec;
    AVCodec *decoder;
    AVPacket *pkt;
    AVFrame *picture, *rgbpicture;
    SwsContext *swscontentRGB;

    uint8_t *bufferRGB;
    QImage img;
    int play_speed = 25, videoIndex = -1;
    float _totalTimeSec = 0;
    bool status = true;
    float CurTimeMsec;

    static QAtomicPointer<Ffmpeg_Decode> _instance;
    static QMutex _mutex;

    bool cacheData = false;

protected:
    void run();

signals:
    void sendImg(QImage img);
};

#endif // FFMPEG_DECODE_H
