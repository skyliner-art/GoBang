#include "newWindow_start.h"
#include"mainwindow.h"
#include"Rules.h"
#include "board.h"
#include "Versus_Lobby.h"
newWindow_start::newWindow_start(QString username,QWidget *parent)
    : QWidget(parent),username(username)
{
    setWindowIcon(QIcon(":/images/image/icon.jpg"));
    human_human =new QPushButton("human_human",this);
    human_human_online=new QPushButton("PVPonline",this);
    human_machine =new QPushButton("human_machine",this);
    machine_machine=new QPushButton("machine_machine",this);
    Rules =new QPushButton("Rules&Settings",this);
    back = new QPushButton("Back",this);

    human_human->setGeometry(100, 50, 200, 50);//人人(线下）
    human_human_online->setGeometry(100, 150, 200, 50);//人人(线上)
    human_machine->setGeometry(100, 250, 200, 50);//人机
    machine_machine->setGeometry(500, 50, 200, 50);//人机
    Rules->setGeometry(500, 150, 200, 50);//规则
    back->setGeometry(500, 250, 200, 50);//回退
    // 设置窗口大小
    setFixedSize(800, 400);
    connect(human_human,&QPushButton::clicked,this,&newWindow_start::openNewHuman_humanWindow);
    connect(human_human_online,&QPushButton::clicked,this,&newWindow_start::openNewOnlineWindow);
    connect(human_machine,&QPushButton::clicked,this,&newWindow_start::openNewman_machineWindow);
    connect(machine_machine,&QPushButton::clicked,this,&newWindow_start::openNewmachine_machineWindow);
    connect(Rules,&QPushButton::clicked,this,&newWindow_start::openRulesWindow);
    connect(back,&QPushButton::clicked,this,&newWindow_start::openMainWindow);
}
void newWindow_start::openNewHuman_humanWindow()
{
    board *newWindow = new board(true,0,username);
    newWindow->show();
    this->close();
    QTimer::singleShot(0, this, &newWindow_start::close);
}
void newWindow_start::openNewOnlineWindow()
{
    GameLobby *newWindow = new GameLobby(username);
    newWindow->show();
    this->close();
    QTimer::singleShot(0, this, &newWindow_start::close);
}
void newWindow_start::openNewman_machineWindow()
{
    board *newWindow = new board(true,2,username);
    newWindow->show();
    this->close();
    QTimer::singleShot(0, this, &newWindow_start::close);
}
void newWindow_start::openNewmachine_machineWindow()
{
    QMessageBox::warning(this,"Warning","How about we explore the area ahead of us later?");
    return;
}
void newWindow_start::openMainWindow()
{
    MainWindow *newWindow = new MainWindow(true,username);
    newWindow->show();
    this->close();
}
void newWindow_start::openRulesWindow()
{
    RulesWindow *newWindow = new RulesWindow(username);
    newWindow->show();
    this->close();
}
newWindow_start::~newWindow_start()
{
}

