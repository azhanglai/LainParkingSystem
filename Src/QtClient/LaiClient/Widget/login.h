#ifndef LOGIN_H
#define LOGIN_H

#include <QWidget>
#include <QPaintEvent>
#include <QPainter>
#include <QTime>
#include <QMessageBox>
#include <QDialog>

#include "Tool/singledb.h"
#include "Tool/package.h"
#include "Thread/thread.h"
#include "Widget/register.h"
#include "Widget/main_interface.h"

namespace Ui {
    class Login;
}

class Login : public QWidget {
    Q_OBJECT

public:
    explicit Login(QWidget *parent = 0);
    ~Login();

    void init_connect();
    QString getcode();
    void paintEvent(QPaintEvent* );
    void mousePressEvent(QMouseEvent *event); // 鼠标点击事件

    static QString user;

private:
    Ui::Login *ui;

    QString code;
    Register* regWin;
    int mark;
    Main_Interface* mainwin;

signals:
    void tomain();

private slots:
    void on_login_btn_clicked();
    void on_register_btn_clicked();
    void on_pwdeye_btn_clicked();
    void backLogin();
    void dologin(bool success);
    void tomainwin();

};

#endif // LOGIN_H
