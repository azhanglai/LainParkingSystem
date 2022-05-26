#include "Widget/laiwidget.h"
#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    LaiWidget w;
    w.show();

    return a.exec();
}
