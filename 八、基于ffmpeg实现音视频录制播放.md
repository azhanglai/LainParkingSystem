[TOC]

### 1、ffmpeg解码简介

#### 1.1 链接

~~~http
https://blog.csdn.net/qq21497936/article/details/108639103
~~~

#### 1.2 ffmpeg解码流程图

![img](https://s2.loli.net/2022/05/26/G6BQb1lNnfqLmt4.png)

#### 1.3 ffmpeg解码相关结构体

~~~c
// AVFormatContext：描述了一个媒体文件或媒体流的构成和基本信息。

// AVInputFormat：类似COM 接口的数据结构，表示输入文件容器格式，着重于功能函数，一种文件容器格式对应一个AVInputFormat 结构，在程序运行时有多个实例。

// AVDictionary：是一个字典集合，键值对，用于配置相关信息。

// AVCodecContext：是一个描述编解码器上下文的数据结构，包含了众多编解码器需要的参数信息。

// AVPacket：保存解复用（demuxer)之后，解码（decode）之前的数据和关于这些数据的一些附加的信息，如显示时间戳（pts），解码时间戳（dts）,数据时长（duration），所在流媒体的索引（stream_index）等等。使用前，使用av_packet_alloc()分配。

// AVCodec：存储编解码器信息的结构体。

// AVFrame：存储的是经过解码后的原始数据。在解码中，AVFrame是解码器的输出；在编码中，AVFrame是编码器的输入。使用前，使用av_frame_alloc()进行分配。

// struct SwsContext：使用前，使用sws_getContext()进行获取，主要用于视频图像的转换。
~~~

#### 1.4 ffmpeg解码相关函数

~~~c++
// 函数原型：
void av_register_all(void);
// 函数功能：初始化libavformat并注册所有muxer、demuxer和协议。如果不调用此函数，则可以选择想要指定注册支持的哪种格式，通过av_register_input_format()、av_register_output_format()。

// 函数原型： 
int avformat_open_input(AVFormatContext **ps,
                        const char *url,
                        AVInputFormat *fmt, 
                        AVDictionary **options);
// 函数功能：打开输入流并读取标头。编解码器未打开。流必须使用avformat_close_input()关闭，返回0成功，<0失败错误码。
// 函数参数ps：指向用户提供的AVFormatContext（由avformat_alloc_context分配）的指针。
// 函数参数url：要打开的流的url。
// 函数参数fmt：fmt如果非空，则此参数强制使用特定的输入格式。否则将自动检测格式。
// 函数参数options：包含AVFormatContext和demuxer私有选项的字典。返回时，此参数将被销毁并替换为包含找不到的选项。都有效则返回为空。

// 函数原型：
int avformat_find_stream_info(AVFormatContext *ic, 
                              AVDictionary **options);
// 函数功能：读取检查媒体文件的数据包以获取具体的流信息，如媒体存入的编码格式。
// 函数参数ic：媒体文件上下文。
// 函数参数options：字典，一些配置选项。

// 函数原型：
AVCodec *avcodec_find_decoder(enum AVCodecID id);
// 函数功能：查找具有匹配编解码器ID的已注册解码器，解码时，已经获取到了，注册的解码器可以通过枚举查看

// 函数原型：
int avcodec_open2(AVCodecContext *avctx, 
                  const AVCodec *codec, 
                  AVDictionary **options);
// 函数功能：初始化AVCodeContext以使用给定的AVCodec。
// 函数参数avctx：描述编解码器上下文的数据结构。
// 函数参数codec：存储编解码器信息的结构体。
// 函数参数options：字典，一些配置选项。

// 函数原型：
struct SwsContext *sws_getContext(int srcW, 
                                  int srcH, 
                                  enum AVPixelFormat srcFormat,
                                  int dstW,
                                  int dstH, 
                                  enum AVPixelFormat dstFormat,
                                  int flags, SwsFilter *srcFilter,
                                  SwsFilter *dstFilter,
                                  const double *param);
// 函数功能：分配并返回一个SwsContext。需要它来执行sws_scale()进行缩放/转换操作。

// 函数原型：
int avpicture_get_size(enum AVPixelFormat pix_fmt, 
                       int width, 
                       int height);
// 函数功能：返回存储具有给定参数的图像的缓存区域大小。
// 函数参数pix_fmt：图像的像素格式。
// 函数参数width：图像的像素宽度。
// 函数参数height：图像的像素高度。

// 函数原型：
int avpicture_fill(AVPicture *picture,
              const uint8_t *ptr,
              enum AVPixelFormat pix_fmt,
              int width,
              int height);

// 函数功能：根据指定的图像、提供的数组设置数据指针和线条大小参数。
// 函数参数picture：输入AVFrame指针，强制转换为AVPciture即可。
// 函数参数ptr：映射到的缓存区，开发者自己申请的存放图像数据的缓存区。

// 函数原型：
int av_read_frame(AVFormatContext *s, 
                  AVPacket *pkt);
// 函数功能：返回流的下一帧。此函数返回存储在文件中的内容，不对有效的帧进行验证。获取存储在文件中的帧中，并为每个调用返回一个。不会的省略有效帧之间的无效数据，以便给解码器最大可用于解码的信息。
// 返回0是成功，小于0则是错误，大于0则是文件末尾，所以大于等于0是返回成功。

// 函数原型：
int avcodec_send_packet(AVCodecContext *avctx, 
                        const AVPacket *avpkt);
// 函数功能：将原始分组数据发送给解码器。在内部，此调用将复制相关的AVCodeContext字段，这些字段可以影响每个数据包的解码，并在实际解码数据包时应用这些字段。（例如AVCodeContext.skip_frame，这可能会指示解码器丢弃使用此函数发送的数据包所包含的帧。）这个函数可以理解为ffmpeg为多线程准备的，将解码数据帧包送入编码器理解为一个线程，将从编码器获取解码后的数据理解为一个线程。
// 函数参数avpkt：通常，这将是一个单一的视频帧，或几个完整的音频帧

// 函数原型：
int avcodec_receive_frame(AVCodecContext *avctx, 
                          AVFrame *frame);
// 函数功能：从解码器返回解码输出数据。这个函数可以理解为ffmpeg为多线程准备的，将解码数据帧包送入编码器理解为一个线程，将从编码器获取解码后的数据理解为一个线程。
// 函数参数frame：这将被设置为参考计数的视频或音频解码器分配的帧（取决于解码器类型）。请注意，函数在执行任何其他操作之前总是调用av_frame_unref（frame）。

// 函数原型：
int avcodec_decode_video2(AVCodecContext *avctx,
                          AVFrame *picture,
                          int *got_picture_ptr,
                          const AVPacket *avpkt);
// 函数功能：将大小为avpkt->size from avpkt->data的视频帧解码为图片。一些解码器可以支持单个avpkg包中的多个帧，解码器将只解码第一帧。出错时返回负值，否则返回字节数，如果没有帧可以解压缩，则为0。
// 函数参数avctx：编解码器上下文
// 函数参数picture：将解码视频帧存储在AVFrame中
// 函数参数got_picture_ptr：输入缓冲区的AVPacket
// 函数参数avpkt：如果没有帧可以解压，那么得到的图片是0，否则，它是非零的。

// 函数原型：
int sws_scale(struct SwsContext *c,
              const uint8_t *const srcSlice[],
              const int srcStride[],
              int srcSliceY,
              int srcSliceH,
              uint8_t *const dst[],
              const int dstStride[]);
// 函数功能：在srcSlice中缩放图像切片并将结果缩放在dst中切片图像。切片是连续的序列图像中的行。
// 函数参数c：以前用创建的缩放上下文*sws_getContext()。
// 函数参数srcSlice：包含指向源片段，就是AVFrame的data。
// 函数参数srcStride：包含每个平面的跨步的数组，其实就是AVFrame的linesize。
// 函数参数srcSliceY：切片在源图像中的位置，从开始计数0对应切片第一行的图像，所以直接填0即可。
// 函数参数srcSliceH：源切片的像素高度。
// 函数参数dst：目标数据地址映像，是目标AVFrame的data。
// 函数参数dstStride：目标每个平面的跨步的数组，就是linesize。

// 函数原型：
void av_free_packet(AVPacket *pkt);
// 函数功能：释放一个包。

// 函数原型：
int avcodec_close(AVCodecContext *avctx);
// 函数功能：关闭给定的avcodeContext并释放与之关联的所有数据（但不是AVCodecContext本身）。

// 函数原型：
void avformat_close_input(AVFormatContext **s);
// 函数功能：关闭打开的输入AVFormatContext。释放它和它的所有内容并将*s设置为空。
~~~

#### 1.5 ffmpeg解码步骤解析

~~~c++
QString fileName = "test/1.mp4";
// ffmpeg相关变量预先定义与分配
AVFormatContext *pAVFormatContext = 0;          // ffmpeg的全局上下文
AVDictionary *pAVDictionary = 0;                // ffmpeg的字典option
AVCodecContext *pAVCodecContext = 0;            // ffmpeg编码上下文
AVCodec *pAVCodec = 0;                          // ffmpeg编码器
AVPacket *pAVPacket = 0;                        // ffmpag单帧数据包
AVFrame *pAVFrame = 0;                          // ffmpeg单帧缓存
AVFrame *pAVFrameRGB32 = 0;                     // ffmpeg单帧缓存转换颜色空间后的缓存
struct SwsContext *pSwsContext = 0;             // ffmpag编码数据格式转换

int ret = 0;                                    // 函数执行结果
int videoIndex = -1;                            // 音频流所在的序号
int gotPicture = 0;                             // 解码时数据是否解码成功
int numBytes = 0;                               // 解码后的数据长度
uchar *outBuffer = 0;                           // 解码后的数据存放缓存区

pAVFormatContext = avformat_alloc_context();    // 分配
pAVPacket = av_packet_alloc();                  // 分配
pAVFrame = av_frame_alloc();                    // 分配
pAVFrameRGB32 = av_frame_alloc();               // 分配

// 步骤一：注册所有容器和编解码器（也可以只注册一类，如注册容器、注册编码器等）
av_register_all();

// 步骤二：打开文件(ffmpeg成功则返回0)
ret = avformat_open_input(&pAVFormatContext, fileName.toUtf8().data(), 0, 0);

// 步骤三：探测流媒体信息
ret = avformat_find_stream_info(pAVFormatContext, 0);

// 步骤四：提取流信息,提取视频信息
for(int index = 0; index < pAVFormatContext->nb_streams; index++) {
	pAVCodecContext = pAVFormatContext->streams[index]->codec;
    switch (pAVCodecContext->codec_type) {
	......
	}
	// 已经找打视频流
    if(videoIndex != -1) {
    	break;
    }
}

// 步骤五：对找到的视频流寻解码器
pAVCodec = avcodec_find_decoder(pAVCodecContext->codec_id);

// 步骤六：打开解码器
ret = avcodec_open2(pAVCodecContext, pAVCodec, NULL);

// 步骤七：对拿到的原始数据格式进行缩放转换为指定的格式高宽大小
pSwsContext = sws_getContext(pAVCodecContext->width,
                             pAVCodecContext->height,
                             pAVCodecContext->pix_fmt,
                             pAVCodecContext->width,
                             pAVCodecContext->height,
                             AV_PIX_FMT_RGBA,
                             SWS_FAST_BILINEAR,
                             0,
                             0,
                             0);

numBytes = avpicture_get_size(AV_PIX_FMT_RGBA,
                              pAVCodecContext->width,
                              pAVCodecContext->height);
outBuffer = (uchar *)av_malloc(numBytes);
// pAVFrame32的data指针指向了outBuffer
avpicture_fill((AVPicture *)pAVFrameRGB32,
                   outBuffer,
                   AV_PIX_FMT_RGBA,
                   pAVCodecContext->width,
                   pAVCodecContext->height);

// 步骤八：读取一帧数据的数据包
while(av_read_frame(pAVFormatContext, pAVPacket) >= 0) {
	if(pAVPacket->stream_index == videoIndex) {
#if 0
	// 步骤九：对读取的数据包进行解码
	ret = avcodec_decode_video2(pAVCodecContext, pAVFrame, &gotPicture, pAVPacket);

	sws_scale(pSwsContext,
          	(const uint8_t * const *)pAVFrame->data,
          	pAVFrame->linesize,
          	0,
          	pAVCodecContext->height,
          	pAVFrameRGB32->data,
          	pAVFrameRGB32->linesize);
          	QImage imageTemp((uchar *)outBuffer,
          	pAVCodecContext->width,
          	pAVCodecContext->height,
          	QImage::Format_RGBA8888);
        
	QImage image = imageTemp.copy();     
	av_free_packet(pAVPacket);
#else
	// 步骤九：发送数据给编码器
	ret = avcodec_send_packet(pAVCodecContext, pAVPacket);

	// 步骤十：循环冲编码器获取解码后的数据
	while(!avcodec_receive_frame(pAVCodecContext, pAVFrame)) {
		sws_scale(pSwsContext,
              	(const uint8_t * const *)pAVFrame->data,
              	pAVFrame->linesize,
              	0,
              	pAVCodecContext->height,
              	pAVFrameRGB32->data,
              	pAVFrameRGB32->linesize);
              	QImage imageTemp((uchar *)outBuffer,
              	pAVCodecContext->width,
              	pAVCodecContext->height,
              	QImage::Format_RGBA8888);
    
	QImage image = imageTemp.copy();
	av_free_packet(pAVPacket);
#endif
	}
	QThread::msleep(1);
}
~~~

### 2、编写ffmpeg解码类

#### 2.1 ffmpeg_decode.h 头文件

~~~c++
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
    #include "libavcodec/avcodec.h"       // 编解码
    #include "libavdevice/avdevice.h"     // 转码
    #include "libavfilter/avfilter.h"
    #include "libavformat/avformat.h"     // 格式
    #include "libavutil/avutil.h"         // 工具
    #include "libswresample/swresample.h"
    #include "libswscale/swscale.h"       // 转置
}

class Ffmpeg_Decode:public QThread {
    Q_OBJECT
public:
    static Ffmpeg_Decode* getInstance();

    int open_video(QString path); 		// 打开视频文件流
    void decode_frame();          		// 解码视频流
    int Seek(float pos);				// 指定播放位置
    int play();							// 播放视频
    int pause();						// 暂停视频

    int GetTotalTimeMsec();
    int GetCurTimeMsec();

    void start(Priority = InheritPriority);
    void change_play_speed(int speed) { this->play_speed = speed; }

    QString replay_path;
private:
    Ffmpeg_Decode();
    
    AVFormatContext *pformatContext; 	// 全局上下文结构体
    AVCodecContext *codec;				// 编码上下文
    AVCodec *decoder;					// 编码器
    AVPacket *pkt;						// 单帧数据包
    AVFrame *picture; 					// 单帧缓存
    AVFrame *rgbpicture; 				// 单帧缓冲转换颜色空间后的缓存 
    SwsContext *swscontentRGB;			// 编码数据格式转换
    
    uint8_t *bufferRGB;					// 解码后的数据存放缓存区
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

~~~

#### 2.2 ffmpeg_decode.cpp 源文件

~~~c++
#include "ffmpeg_decode.h"
#include <QDebug>

QAtomicPointer<Ffmpeg_Decode> Ffmpeg_Decode::_instance = 0;
QMutex Ffmpeg_Decode::_mutex;

Ffmpeg_Decode::Ffmpeg_Decode() {
    // 注册所有容器和编解码器
    av_register_all();
    // 注册输入/输出设备
    avdevice_register_all();
}

Ffmpeg_Decode * Ffmpeg_Decode::getInstance() {
    // 第一次检测
    if (_instance.testAndSetOrdered(0, 0)) {
        QMutexLocker locker(&_mutex);                       // 加互斥锁。
        _instance.testAndSetOrdered(0, new Ffmpeg_Decode);  // 第二次检测。
    }
    return _instance;
}
// 打开视频文件
int Ffmpeg_Decode::open_video(QString path) {
    this->pformatContext = avformat_alloc_context();
    this->replay_path = path;
    // 1.打开文件获取文件全局上下 
    if (avformat_open_input(&pformatContext, replay_path.toStdString().c_str(), nullptr, nullptr) != 0) {
        qDebug("Couldn't open input stream.");
        return -1;
    }
    // 2.探测视频文件信息、流数据、视频流
    if (avformat_find_stream_info(pformatContext, nullptr) < 0) {
        qDebug("Couldn't find stream information.");
        return -1;
    }
    // 3.查找是否存在视频流
    for(int i = 0; i < this->pformatContext->nb_streams; i++) {
        if(pformatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            this->videoIndex = i; // 找到视频流下标
            break;
        }
    }
	// 4.对找到的编解码器上下文结构体寻找解码器
    this->codec = pformatContext->streams[videoIndex]->codec;
    this->decoder = avcodec_find_decoder(codec->codec_id);
    if(decoder == nullptr) {
        printf("video code not find");
        return -1;
    }
    // 5.打开解码器
    if(avcodec_open2(codec , decoder, nullptr) != 0) {
        printf("video code not open");
        return -1;
    }
    // 获取视频的总时间
    _totalTimeSec = (float)pformatContext->duration / (AV_TIME_BASE);
    // 分配空间
    this->pkt = (AVPacket *)malloc(sizeof (AVPacket));  
    picture = av_frame_alloc();
    rgbpicture = av_frame_alloc();
    // 6.对拿到的原始数据格式进行缩放转换为指定的格式高宽大小
    swscontentRGB = nullptr;
    swscontentRGB = sws_getContext(this->codec->width,this->codec->height,this->codec->pix_fmt,
                                this->codec->width,this->codec->height,AV_PIX_FMT_RGB32,
                                SWS_BICUBIC,nullptr,nullptr,nullptr);

    //----------------------RGB32像素数据准备---------------------------//
    // 计算一帧图像数据大小
    int imgaeBytesRGB = avpicture_get_size(AV_PIX_FMT_RGB32,this->codec->width,this->codec->height);
    // 动态开辟空间
    bufferRGB =(uint8_t *) av_malloc(imgaeBytesRGB *sizeof (uint8_t));
    // rgbpicture的data指针指向了bufferRGB
    avpicture_fill((AVPicture *)rgbpicture, bufferRGB, AV_PIX_FMT_RGB32, this->codec->width, this->codec->height);
}
// 循环解码视频数据包
void Ffmpeg_Decode::decode_frame() {
    // 7. 读取一帧数据的数据包
    while(av_read_frame(pformatContext, pkt) >= 0) {
        while (status == false) {
            msleep(1);
        }
        // 获取当前的时间
        this->CurTimeMsec = pkt->pts * av_q2d(pformatContext->streams[pkt->stream_index]->time_base);

        // 读取码流数据 pkt
        if(videoIndex == pkt->stream_index) {
            int got_picture_ptr = -1;
            // 8. 对读取的数据包进行解码，picture：有损像素数据
            avcodec_decode_video2(codec, picture, &got_picture_ptr, pkt);
            if(got_picture_ptr != 0) {
                // 剔除 得到纯净RGBpicture
                sws_scale(swscontentRGB,picture->data,picture->linesize,0,
                          picture->height,rgbpicture->data,rgbpicture->linesize);
                // 转成Qt图像
                img = QImage((uchar *)bufferRGB, codec->width, codec->height,QImage::Format_RGB32);

                msleep(this->play_speed);
                // 发送信号到播放界面
                emit sendImg(img);
            }
        }
        // 重置pkt
        av_packet_unref(pkt);
    }
}
// 线程处理函数
void Ffmpeg_Decode::run() {
    while(1) {
        decode_frame();
        while (!cacheData) {
            msleep(1);
        }
    }
}
// 线程启动函数
void Ffmpeg_Decode::start(Priority pro) {
    status = true;
    cacheData = true;
    QThread::start(pro);
    qDebug("ZFFmpeg start");
}
// 获取当前播放时间(ms)
int  Ffmpeg_Decode::GetCurTimeMsec() {
    return (int)(this->CurTimeMsec * 1000);
}
// 获取视频总时间（ms）
int Ffmpeg_Decode::GetTotalTimeMsec() {
    return (int)(_totalTimeSec * 1000);
}
// 视频播放，启动线程
int Ffmpeg_Decode::play() {
    this->start();
    return 0;
}
// 暂停视频
int Ffmpeg_Decode::pause() {
    status = false;
    cacheData = false;
    return 0;
}

int Ffmpeg_Decode::Seek(float pos) {
    // 加互斥锁
    QMutexLocker locker(&_mutex);
    // 未打开视频
    if (!pformatContext) {
        return -1;
    }
    cacheData = false;
    msleep(20);
    // 将视频移至到当前点击滑动条位置
    if (av_seek_frame(pformatContext,-1,(int64_t)(pos / 1000.0) * AV_TIME_BASE,AVSEEK_FLAG_ANY) > 0) {
        qDebug("seek error");
        return -1;
    }
    return 0;
}
~~~

### 3、基于ffmpeg解码器实现录制视频播放

#### 3.1 UI界面

![img](https://s2.loli.net/2022/05/26/aOpqos7xfDBguXw.png)

#### 3.2 main_replay.h 头文件

~~~c++
#ifndef MAIN_REPLAY_H
#define MAIN_REPLAY_H

#include <QWidget>
#include <QFileDialog>
#include <QFile>
#include <QMessageBox>
#include <QTimer>

namespace Ui {
    class Main_Replay;
}

class Main_Replay : public QWidget {
    Q_OBJECT
public:
    explicit Main_Replay(QString path, QWidget *parent = nullptr);
    ~Main_Replay();

    void init_connect();
private:
    Ui::Main_Replay *ui;

    QImage image;
    QTimer UpdatePlayTime;

    int LoadVideo(QString path);
    void ReadPlay();
    void Playing();
    void PausePlay();

    bool videoTimeSilderPressed = false;
    bool playing = false;
protected:
    void paintEvent(QPaintEvent*);
private slots:
    void receiveImg(QImage img);

    void UpdatePlayTimeOut();
    void on_Play_btn_clicked();
    void on_Video_slider_sliderPressed();
    void on_Video_slider_sliderReleased();
    void on_comboBox_currentIndexChanged(int index);
};

#endif // MAIN_REPLAY_H
~~~

#### 3.3 main_replay.cpp 源文件

~~~c++
#include "main_replay.h"
#include "ui_main_replay.h"

#include "Ffmpeg/ffmpeg_decode.h"

Main_Replay::Main_Replay(QString path, QWidget *parent) : QWidget(parent), ui(new Ui::Main_Replay) {
    ui->setupUi(this);

    ui->Video_slider->setEnabled(true);
    // 录制视频总时长
    ui->Video_slider->setMaximum(Ffmpeg_Decode::getInstance()->GetTotalTimeMsec());
    this->UpdatePlayTime.start(100);

    init_connect();
    ReadPlay();			// 1.播放前初始化
    LoadVideo(path);	// 2.使用解码器播放视频
    Playing();			// 3.播放状态画面
}

Main_Replay::~Main_Replay() {
    delete ui;
}

void Main_Replay::init_connect() {
    connect(Ffmpeg_Decode::getInstance(), SIGNAL(sendImg(QImage)), this, SLOT(receiveImg(QImage)));
    connect(&UpdatePlayTime, SIGNAL(timeout()), this, SLOT(UpdatePlayTimeOut()));
}

void Main_Replay::receiveImg(QImage img) {
    this->image = img;
    this->update();
}

void Main_Replay::paintEvent(QPaintEvent*) {
    if(!this->image.isNull()) {
        QPixmap *image = new QPixmap(QPixmap::fromImage(this->image));
        image->scaled(ui->Video_label->size(), Qt::KeepAspectRatio);
        ui->Video_label->setPixmap(QPixmap::fromImage(this->image));
        ui->Video_label->setScaledContents(true);
    }
}
// 更新新当前播放时长
void Main_Replay::UpdatePlayTimeOut() {
    int curMsec = Ffmpeg_Decode::getInstance()->GetCurTimeMsec();
    if (videoTimeSilderPressed == false) {
        ui->Video_slider->setValue(curMsec);
    }
    int curSec = curMsec / 1000;
    ui->curTime_label->setText(QString("%1:%2").arg(curSec / 60, 2, 10, QLatin1Char('0')).arg(curSec% 60, 2, 10, QLatin1Char('0')));

}
// 视频播放前初始化
void Main_Replay::ReadPlay() {
    playing = true;						// 设置成播放状态（但还未真正的播放）
    ui->Video_slider->setEnabled(true);
    ui->Play_btn->setEnabled(true);
    ui->Play_btn->setText(tr("播放"));  // 还未播放，点击播放可以播放
    ui->Video_slider->setValue(0);
    ui->curTime_label->setText(QString("00:00"));
    ui->totalTime_label->setText(QString("00:00"));
}
// 根据视频文件路径，使用解码器加载并播放视频
int Main_Replay::LoadVideo(QString path) {
    Ffmpeg_Decode::getInstance()->open_video(path);
    Ffmpeg_Decode::getInstance()->play();
    return 0;
}
// 播放状态
void Main_Replay::Playing() {
    ui->Video_slider->setEnabled(true);
    ui->Play_btn->setText(tr("暂停")); // 已经播放，点击暂停可以暂停
    ui->Video_slider->setMaximum(Ffmpeg_Decode::getInstance()->GetTotalTimeMsec());

    int totalMsec = Ffmpeg_Decode::getInstance()->GetTotalTimeMsec();
    int totalSec = totalMsec / 1000;
    ui->totalTime_label->setText(QString("%1:%2").arg(totalSec / 60, 2, 10,QLatin1Char('0')).arg(totalSec% 60, 2,10,QLatin1Char('0')));
}
// 暂停播放
void Main_Replay::PausePlay() {
    playing = false;					// 设置成暂停状态
    ui->Play_btn->setText(tr("播放"));  // 已经暂停，点击播放可以播放	
}

// 暂停和播放按钮槽函数
void Main_Replay::on_Play_btn_clicked() {
    // 如果目前是播放状态，点击的是暂停按钮，所以会暂停
    if (playing) {
        PausePlay();							// 暂停播放
        Ffmpeg_Decode::getInstance()->pause();  // 视频暂停解析
        UpdatePlayTime.stop();					// 不在更新播放时长
    }
    // 目前是暂停状态，点击的是播放按钮，所以会播放
    else {
        playing = true;
        Playing();								// 播放状态				
        Ffmpeg_Decode::getInstance()->play();	// 解析视频
        UpdatePlayTime.start(100);				// 更新播放时长
    }
}

// 滑动进度条鼠标按键按下槽函数
void Main_Replay::on_Video_slider_sliderPressed() {
    videoTimeSilderPressed = true;
}

// 滑动进度条鼠标按键释放槽函数
void Main_Replay::on_Video_slider_sliderReleased() {
    UpdatePlayTime.stop(); 			// 不在更新播放时长
    videoTimeSilderPressed = false;
	// 根据进度条的位置，调整播放时长
    Ffmpeg_Decode::getInstance()->Seek(ui->Video_slider->value());
    PausePlay();					// 暂停播放
}

// 视频播放速率控制槽函数
void Main_Replay::on_comboBox_currentIndexChanged(int index) {
    if(index == 0) {
        Ffmpeg_Decode::getInstance()->change_play_speed(25);
    }
    else if(index == 1) {
        Ffmpeg_Decode::getInstance()->change_play_speed(50);
    }
    else if(index == 2) {
        Ffmpeg_Decode::getInstance()->change_play_speed(12);
    }
}
~~~

### 4、实现停车场监控视频回放功能

#### 4.1 UI界面

![img](https://s2.loli.net/2022/05/24/OkcRoMVjwKyILCg.png)

##### 4.2 main_playback.h 头文件

~~~c++
#ifndef MAIN_PLAYBACK_H
#define MAIN_PLAYBACK_H

#include <QWidget>
#include <QListWidgetItem>

#define DATE 0
#define DAY  1

namespace Ui {
    class Main_Playback;
}

class Main_Playback : public QWidget {
    Q_OBJECT
public:
    explicit Main_Playback(QWidget *parent = nullptr);
    ~Main_Playback();

    void init_connect();
    void dateOrday();
    int show_video();
private:
    Ui::Main_Playback *ui;

    int row, col;
    int state;
    int num;
    QString video_date;
private slots:
    void on_month_btn_toggled(bool checked);
    void on_day_btn_toggled(bool checked);

    void click_date(QListWidgetItem *);
    void click_video(QListWidgetItem *);

    void on_pre_btn_clicked();
    void on_next_btn_clicked();
};

#endif // MAIN_PLAYBACK_H

~~~

##### 4.3 main_playback.h 源文件

~~~c++
#include "main_playback.h"
#include "ui_main_playback.h"

#include <QMessageBox>
#include "Widget/main_replay.h"
#include "Tool/singledb.h"
#include "Sqlite/car_model.h"

Main_Playback::Main_Playback(QWidget *parent) : QWidget(parent), ui(new Ui::Main_Playback) {
    ui->setupUi(this);

    this->num = 1;
    QString page = QString::number(num);
    ui->page_label->setText("第" + page + "页"); 

    init_connect();

    // 设置QListWidget的显示模式
    ui->listWidget_2->setViewMode(QListView::IconMode);
    // 设置QListWidget中单元项的间距
    ui->listWidget_2->setSpacing(4);
    // 设置自动适应布局调整
    ui->listWidget_2->setResizeMode(QListWidget::Adjust);
    // 设置不能移动
    ui->listWidget_2->setMovement(QListWidget::Static);
    ui->listWidget_2->setStyleSheet("border:none;font-size:15px");
    ui->listWidget_2->setIconSize(QSize(230, 150));

}

Main_Playback::~Main_Playback() {
    delete ui;
}

void Main_Playback::init_connect() {
    // listWidget点击信号
    connect(ui->listWidget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(click_date(QListWidgetItem*)));
    connect(ui->listWidget_2, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(click_video(QListWidgetItem*)));
}
// 根据日期，查询本地数据库的监控视频，显示到listWidget中
void Main_Playback::dateOrday() {
    this->num = 1;
    ui->listWidget_2->clear();
    ui->listWidget->clear();

    char **qres;
    if (state == DATE) {
        qres = Car_Model::getInstance()->get_date();
    } else {
        qres = Car_Model::getInstance()->get_day();
    }
    this->row = Car_Model::getInstance()->get_row();
    this->col = Car_Model::getInstance()->get_col();
    for (int i = 0; i < row; ++i) {
        // 定义QListWidgetItem对象
        QListWidgetItem *playItem = new QListWidgetItem;
        playItem->setText(qres[(i+1) * col]);
        playItem->setTextAlignment(0x0004);
        playItem->setSizeHint(QSize(150,40));   // 重新设置单元项宽度和高度
        ui->listWidget->addItem(playItem);      // 将单元项添加到QListWidget中
    }
}
// 查询本地数据库的监控视频，并找到本地视频文件，将其显示在listWidget_2中
int Main_Playback::show_video() {
    char **qres2;
    if(state == DATE) {
        qres2=Car_Model::getInstance()->get_video_by_date(video_date, num);
        
    } else {
        qres2=Car_Model::getInstance()->get_video_by_day(video_date, num);
    }
    this->row = Car_Model::getInstance()->get_row();
    this->col = Car_Model::getInstance()->get_col();
    if (this->row == 0 && this->col == 0) {
        return -1;
    }
    ui->listWidget_2->clear();
   
    QList<QString> nameList;
    for(int i = col; i < (row+1)*col; i++) {
        nameList << qres2[i];
    }
    for (int i = 0; i < nameList.size(); i++){
        //定义QListWidgetItem对象
        QListWidgetItem *imageItem = new QListWidgetItem;
        //为单元项设置属性
        imageItem->setIcon(QIcon(SingleDB::getInstance()->get_video_path()+ "/" + nameList[i] + ".jpg"));
        imageItem->setText(nameList.at(i));
        
        imageItem->setSizeHint(QSize(220,150)); // 重新设置单元项图片的宽度和高度
        ui->listWidget_2->addItem(imageItem);   // 将单元项添加到QListWidget中
    }
    return 0;
}
// 点击listWidget数据的槽函数, 功能是将本地视频显示到listWidget_2中
void Main_Playback::click_date(QListWidgetItem *item) {
    video_date = item->text();
    show_video();
}
// 点击listWidget_2数据的槽函数，功能是播放点击的本地视频
void Main_Playback::click_video(QListWidgetItem *item) {
    Main_Replay *replay = new Main_Replay(SingleDB::getInstance()->get_video_path()+"/"+item->text()+".avi");
    replay->show();
}
// 按月显示按钮槽函数
void Main_Playback::on_month_btn_toggled(bool checked) {
    if(checked == true) {
        this->state = DATE;
        dateOrday();
    }
}
// 按天显示按钮槽函数
void Main_Playback::on_day_btn_toggled(bool checked) {
    if(checked == true) {
        this->state = DAY;
        dateOrday();
    }
}
// 上一页按钮槽函数
void Main_Playback::on_pre_btn_clicked() {
    if(num == 1) {
        QMessageBox::warning(this, tr("提示！"),tr("已经是第一页！"),QMessageBox::Yes);
    }
    else {
        num--;
        ui->page_label->setText("第"+QString::number(num)+"页");
        show_video();

    }
}
// 下一页按钮槽函数
void Main_Playback::on_next_btn_clicked() {
    num++;
    if(show_video() == -1) {
        num--;
        QMessageBox::warning(this, tr("提示！"),tr("已经是最后一页！"),QMessageBox::Yes);
    }
    else {
        ui->page_label->setText("第"+QString::number(num)+"页");
    }
}
~~~

