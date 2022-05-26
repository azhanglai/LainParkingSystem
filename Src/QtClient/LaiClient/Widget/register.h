#ifndef REGISTER_H
#define REGISTER_H

#include <QWidget>

#include "Tool/singledb.h"
#include "Tool/package.h"
#include "Thread/thread.h"

namespace Ui {
    class Register;
}

class Register : public QWidget {
    Q_OBJECT

public:
    explicit Register(QWidget *parent = 0);
    ~Register();

    void init_connect();

private:
    Ui::Register *ui;
    int mark, mark2;

signals:
    void back();

private slots:
    void on_eye_btn_clicked();
    void on_eye2_btn_clicked();
    void on_register_btn_clicked();
    void doregister(bool success);

};

#endif // REGISTER_H
