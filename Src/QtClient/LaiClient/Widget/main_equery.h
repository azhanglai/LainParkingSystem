#ifndef MAIN_EQUERY_H
#define MAIN_EQUERY_H

#include <QWidget>
#include "Thread/thread.h"

namespace Ui {
    class Main_Equery;
}

class Main_Equery : public QWidget {
    Q_OBJECT

public:
    explicit Main_Equery(QWidget *parent = nullptr);
    ~Main_Equery();

    void init_connect();
    void send_msg();

private:
    Ui::Main_Equery *ui;

    int frequency, num;

private slots:
    void on_equery_btn_clicked();
    void on_more_btn_clicked();
    void show_msg(PACK_ALLCARMSG_BACK result);
    void on_export_btn_clicked();
};

#endif // MAIN_EQUERY_H
