#include "login.h"
#include "ui_login.h"
#include <QDebug>

QString Login::user;

Login::Login(QWidget *parent) : QWidget(parent), ui(new Ui::Login) {
    ui->setupUi(this);
    this->mark = 0;
    this->code = this->getcode();

    ui->pwd_edit->setEchoMode(QLineEdit::Password);
    this->regWin = new Register();
    init_connect();
    this->update();
}

Login::~Login() {
    delete ui;
}

void Login::init_connect() {
    connect(this->regWin, &Register::back, this, &Login::backLogin);
    connect(Thread::getInstance(), SIGNAL(Login(bool)), this, SLOT(dologin(bool)));
    connect(this, &Login::tomain, this, &Login::tomainwin);

}

// 生成验证码
QString Login::getcode() {
    update();
    QString ncode;
    // 随机数字
    for (int i = 0; i < 4; ++i) {
        int num = qrand() % 3;
        if (num == 0) {
            // 数字
            ncode += QString::number(qrand() % 10);
        } else if(num == 1) {
            // 大写字母
            int temp = 'A';
            ncode += static_cast<QChar>(temp + qrand() % 26);
        } else if(num == 2) {
            // 小写字母
            int temp = 'a';
            ncode += static_cast<QChar>(temp + qrand() % 26);
        }
    }
    return ncode;
}

// 验证码干扰元素
void Login::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    QPen pen;
    painter.drawPixmap(this->rect(), QPixmap(":image/pt.jpg"));
    // 画点
    for(int i = 0;i < 100;++i) {
        pen = QPen(QColor(qrand()%256, qrand()%256, qrand()%256));
        painter.setPen(pen);
        painter.drawPoint(360 + qrand()%110, 250 + qrand()%45);
    }
    // 画线
    for(int i = 0;i < 10;++i){
        painter.drawLine(360 + qrand()%110, 250 + qrand()%45,
                         360 + qrand()%110, 250 + qrand()%45);
    }
    pen = QPen(QColor(255, 0, 0, 100));
    QFont font("楷体", 20, QFont::Bold, true);
    painter.setFont(font);
    painter.setPen(pen);
    // 绘画字
    for(int i = 0;i < 4;++i) {
        painter.drawText(360+25*i, 250, 30, 40, Qt::AlignCenter, QString(code[i]));
    }
}

void Login::mousePressEvent(QMouseEvent *event) {
    //验证码区域做重写鼠标点击事件
    int x = event->x();
    int y = event->y();
    if(x > 360 && x < 460 && y > 230 && y < 280) {
        this->code = this->getcode();
    }
}

// 登录验证函数
void Login::on_login_btn_clicked() {
    QString captcha = ui->code_edit->text();
    if(captcha.toLower() == this->code.toLower()) {
        PACK_HEAD head;
        PACK_REGIST_LOGIN login_body;
        char buffer[sizeof(PACK_HEAD) + sizeof(PACK_REGIST_LOGIN)] = { 0 };
        memset(&head, 0, sizeof(PACK_HEAD));
        memset(&login_body, 0, sizeof(PACK_REGIST_LOGIN));

        head.bodyType=2;
        strcpy(login_body.name, ui->name_edit->text().toStdString().c_str());
        strcpy(login_body.pwd, ui->pwd_edit->text().toStdString().c_str());

        memcpy(buffer, &head, sizeof(PACK_HEAD));
        memcpy(buffer + sizeof(PACK_HEAD), &login_body, sizeof(PACK_REGIST_LOGIN));
        write(SingleDB::getInstance()->get_fd(), buffer, sizeof(PACK_HEAD) + sizeof(PACK_REGIST_LOGIN));
        qDebug() << "fd:" << SingleDB::getInstance()->get_fd() << " client send: login\n";
        return;
    } else {
        QMessageBox::warning(this, tr("警告！"), tr("验证码错误！"), QMessageBox::Yes);
        this->code = this->getcode();
    }
}

// 跳转到注册
void Login::on_register_btn_clicked() {
    this->hide();

    this->regWin->show();
    // 清空输入框内容
    ui->name_edit->clear();
    ui->pwd_edit->clear();
    ui->code_edit->clear();
}

void Login::on_pwdeye_btn_clicked() {
    // 0是密文 1是明文
    if(this->mark == 0) {
        ui->pwd_edit->setEchoMode(QLineEdit::Normal);
        this->mark = 1;
    } else if(this->mark == 1) {
        ui->pwd_edit->setEchoMode(QLineEdit::Password);
        this->mark = 0;
    }
}

// 返回到登录
void Login::backLogin() {
    this->regWin->close();
    this->show();
}

// 根据服务器返回的布尔类型判断是否登陆成功
void Login::dologin(bool success) {
    if(success == true) {
        this->user = ui->name_edit->text();
        emit tomain();
    } else {
        QMessageBox::warning(this, tr("提示！"), tr("帐号或密码错误！"), QMessageBox::Yes);
        // 清空输入框内容
        ui->name_edit->clear();
        ui->pwd_edit->clear();
        ui->code_edit->clear();
        this->update();
    }
}

void Login::tomainwin() {
    this->hide();
    this->mainwin = new Main_Interface();
    this->mainwin->show();
}
