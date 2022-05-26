#include "main_replay.h"
#include "ui_main_replay.h"

#include "Ffmpeg/ffmpeg_decode.h"

Main_Replay::Main_Replay(QString path, QWidget *parent) : QWidget(parent), ui(new Ui::Main_Replay) {
    ui->setupUi(this);

    ui->Video_slider->setEnabled(true);
    ui->Video_slider->setMaximum(Ffmpeg_Decode::getInstance()->GetTotalTimeMsec());

    this->UpdatePlayTime.start(100);

    init_connect();
    ReadPlay();
    LoadVideo(path);
    Playing();
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

void Main_Replay::UpdatePlayTimeOut() {
    int curMsec = Ffmpeg_Decode::getInstance()->GetCurTimeMsec();
    if (videoTimeSilderPressed == false) {
        ui->Video_slider->setValue(curMsec);
    }
    int curSec = curMsec / 1000;
    ui->curTime_label->setText(QString("%1:%2").arg(curSec / 60, 2, 10, QLatin1Char('0')).arg(curSec% 60, 2, 10, QLatin1Char('0')));

}

void Main_Replay::ReadPlay() {
    playing = true;
    ui->Video_slider->setEnabled(true);
    ui->Play_btn->setEnabled(true);
    ui->Play_btn->setText(tr("播放"));
    ui->Video_slider->setValue(0);
    ui->curTime_label->setText(QString("00:00"));
    ui->totalTime_label->setText(QString("00:00"));
}

int Main_Replay::LoadVideo(QString path) {
    Ffmpeg_Decode::getInstance()->open_video(path);
    Ffmpeg_Decode::getInstance()->play();
    return 0;
}

void Main_Replay::Playing() {
    ui->Video_slider->setEnabled(true);
    ui->Play_btn->setText(tr("暂停"));
    ui->Video_slider->setMaximum(Ffmpeg_Decode::getInstance()->GetTotalTimeMsec());

    int totalMsec = Ffmpeg_Decode::getInstance()->GetTotalTimeMsec();
    int totalSec = totalMsec / 1000;
    ui->totalTime_label->setText(QString("%1:%2").arg(totalSec / 60, 2, 10,QLatin1Char('0')).arg(totalSec% 60, 2,10,QLatin1Char('0')));
}

void Main_Replay::PausePlay() {
    playing = false;
    ui->Play_btn->setText(tr("播放"));
}

// 暂停和播放槽函数
void Main_Replay::on_Play_btn_clicked() {
    if (playing) {
        PausePlay();
        Ffmpeg_Decode::getInstance()->pause();
        UpdatePlayTime.stop();
    } else {
        playing = true;
        Playing();
        Ffmpeg_Decode::getInstance()->play();
        UpdatePlayTime.start(100);
    }
}

// 滑动进度条鼠标按键按下
void Main_Replay::on_Video_slider_sliderPressed() {
    videoTimeSilderPressed = true;
}

// 滑动进度条鼠标按键释放
void Main_Replay::on_Video_slider_sliderReleased() {
    UpdatePlayTime.stop();
    videoTimeSilderPressed = false;

    Ffmpeg_Decode::getInstance()->Seek(ui->Video_slider->value());
    PausePlay();
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
