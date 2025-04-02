#ifndef NEWWINDOW_START_H
#define NEWWINDOW_START_H
#include<QWidget>
#include<qpushbutton>
#include<QTimer>
class newWindow_start :public QWidget
{
    Q_OBJECT
private:
    QPushButton *human_machine;//人机
    QPushButton *machine_machine;//机器自对弈
    QPushButton *human_human;//人人（线下）
    QPushButton *human_human_online;//人人（线上）//TODO
    QPushButton *Rules;//规则阐述
    QPushButton *back;//返回主菜单
    QString username;
public:
    newWindow_start(QString username,QWidget *parent = nullptr);
    void openMainWindow();//打开主菜单
    void openNewHuman_humanWindow();
    void openNewOnlineWindow();
    void openRulesWindow();
    void openNewman_machineWindow();
    void openNewmachine_machineWindow();
    ~newWindow_start();
};

#endif // NEWWINDOW_START_H
