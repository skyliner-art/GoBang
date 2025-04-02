#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    bool login =false;QString username="";//登录信息和用户名
    MainWindow *NewWindow=new MainWindow(login,username);//开启主菜单
    NewWindow->show();
    return a.exec();
}
