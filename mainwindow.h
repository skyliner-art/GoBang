#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QPushButton>
#include <QMainWindow>
#include <QString>
#include<QLabel>
#include<QSettings>
#include"board.h"
extern QString appconfigFile;
extern QSettings appsettings;
extern QString appip;
extern QString appport;
extern QString appurl;
extern QUrl appurlObj;

extern QString serverconfigFile;
extern QSettings serversettings;
extern QString serverip;
extern QString serverport;
extern QString serverurl;
extern QUrl serverurlObj;
class MainWindow : public QMainWindow
{
    Q_OBJECT
private:
    QPushButton *NewStart;//新一局
    QPushButton *Records;//查看记录
    QPushButton *Register;//注册
    QPushButton *Login;//登录
    QLabel *usernameLabel;
    void closeWindow();
    bool login;
public:
    struct GameInfo//user的历史对局数据
    {
        QString time;
        QString firstPlayer;
        QString secondPlayer;
        int gameid;
        int state;
        int step;
        vector<vector<int>> nowboard;
        QStack<board::GameData> gamedata;
    };
    QString username;
    void getdata(QString username);//从服务器获取记录
    MainWindow(bool login,QString usename,QWidget *parent = nullptr);
    void openRegisterWindow();//打开注册页面
    void openLoginWindow();//打开登录界面
    void openNewWindow();//打开新游戏界面
    void openRecordsWindow();//打开记录界面
    void openRulesWindow();//打开规则界面
    ~MainWindow();
};
#endif // MAINWINDOW_H
