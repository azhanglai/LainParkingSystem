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
    // 找编解码器上下文结构体
    this->codec = pformatContext->streams[videoIndex]->codec;
    // 4.找解码器
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
    // 一帧图像数据大小
    int imgaeBytesRGB = avpicture_get_size(AV_PIX_FMT_RGB32,this->codec->width,this->codec->height);
    bufferRGB =(uint8_t *) av_malloc(imgaeBytesRGB *sizeof (uint8_t));
    // rgbpicture的data指针指向了bufferRGB
    avpicture_fill((AVPicture *)rgbpicture,bufferRGB,AV_PIX_FMT_RGB32,this->codec->width,this->codec->height);

}

void Ffmpeg_Decode::decode_frame() {
    // 7. 读取一帧数据的数据包
    while(av_read_frame(pformatContext, pkt) == 0) {
        while (status == false) {
            msleep(1);
        }
        // 获取当前的时间
        this->CurTimeMsec = pkt->pts * av_q2d(pformatContext->streams[pkt->stream_index]->time_base);

        // 读取码流数据 pkt
        if(videoIndex == pkt->stream_index) {
            int got_picture_ptr = -1;
            // 8. 对读取的数据包进行解码，picture：有损像素数据
            avcodec_decode_video2(codec,picture,&got_picture_ptr,pkt);
            if(got_picture_ptr != 0) {
                // 剔除 得到纯净RGBpicture
                sws_scale(swscontentRGB,picture->data,picture->linesize,0,
                          picture->height,rgbpicture->data,rgbpicture->linesize);

                img = QImage((uchar *)bufferRGB,codec->width,codec->height,QImage::Format_RGB32);
                msleep(this->play_speed);
                // 发送信号到播放界面
                emit sendImg(img);
            }
        }
        // 重置pkt
        av_packet_unref(pkt);
    }
}

void Ffmpeg_Decode::run() {
    while(1) {
        decode_frame();
        while (!cacheData) {
            msleep(1);
        }
    }
}

void Ffmpeg_Decode::start(Priority pro) {
    status = true;
    cacheData = true;
    QThread::start(pro);
    qDebug("ZFFmpeg start");
}

int  Ffmpeg_Decode::GetCurTimeMsec() {
    return (int)(this->CurTimeMsec * 1000);
}

int Ffmpeg_Decode::GetTotalTimeMsec() {
    return (int)(_totalTimeSec * 1000);
}

int Ffmpeg_Decode::play() {
    this->start();
    return 0;
}

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






