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
