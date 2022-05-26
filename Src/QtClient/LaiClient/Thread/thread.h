#ifndef THREAD_H
#define THREAD_H

#include <QThread>
#include <QTimer>
#include "Tool/package.h"

class Thread : public QThread {
    Q_OBJECT

public:
    static Thread* getInstance();

    int get_car_amount() { return car_amount; }

private:
    Thread();
    static Thread* myThread;
    int car_amount = 0;
    QTimer* heartbeat;

protected:
    void run();

signals:
    void Login(bool success);
    void Register(bool success);
    void out(PACK_EXIT_BACK );
    void car_msg(PACK_ALLCARMSG_BACK);
    void photo_continue_in(int result);

public slots:
    void send_heartbeat();

};

#endif // THREAD_H
