#ifndef SET_H
#define SET_H

#include <QWidget>

namespace Ui {
    class Set;
}

class Set : public QWidget {
    Q_OBJECT

public:
    explicit Set(QWidget *parent = 0);
    ~Set();

    void init_connect();

private:
    Ui::Set *ui;

private slots:
    void on_OK_btn_clicked();
    void Select_VideoPath();
    void Select_ImagePath();
};

#endif // SET_H
