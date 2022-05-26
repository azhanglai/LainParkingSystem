#ifndef MAIN_INTERFACE_H
#define MAIN_INTERFACE_H

#include <QWidget>
#include <QTimer>
#include <QDateTime>

#include "Widget/set.h"
#include "Widget/main_in.h"
#include "Widget/main_out.h"
#include "Widget/main_inmanage.h"
#include "Widget/main_equery.h"
#include "Widget/main_playback.h"

namespace Ui {
    class Main_Interface;
}

class Main_Interface : public QWidget {
    Q_OBJECT

public:
    explicit Main_Interface(QWidget *parent = 0);
    ~Main_Interface();

    void init_connect();

    Set* setwin;
    Main_In *inwin;
    Main_Out *outwin;
    Main_Inmanage *inmanagewin;
    Main_Equery *equerywin;
    Main_Playback *playbackwin;

private:
    Ui::Main_Interface *ui;

    int mark;
    QTimer *time;

private slots:
    void qtimeSlot();
    void on_in_out_btn_clicked();
    void on_inside_btn_clicked();
    void on_query_btn_clicked();
    void on_playback_btn_clicked();
    void on_set_btn_clicked();
    void update_car_amount(int num);
};

#endif // MAIN_INTERFACE_H
