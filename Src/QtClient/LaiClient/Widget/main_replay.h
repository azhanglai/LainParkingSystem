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
