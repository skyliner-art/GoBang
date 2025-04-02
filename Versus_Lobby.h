#ifndef VERSUS_LOBBY_H
#define VERSUS_LOBBY_H
#include <QWidget>
#include <QListWidget>
#include <QCloseEvent>
#include <QPushButton>
#include <QLabel>
#include <QTextEdit>
#include <QComboBox>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QtWebSockets/QWebSocket>
#include <QJsonDocument>
#include<QTableWidget>
#include <QJsonObject>
#include <QJsonArray>
#include "newWindow_start.h"
#include"board.h"
class GameLobby : public QWidget {
    Q_OBJECT
    friend class board;
public:
    QString opponent_name;
    GameLobby(QString username,QWidget *parent = nullptr);
    void onConnected();
    void Disconnected();
    void getMessage(const QString &message);
public slots:
    void handleMoveSent(const QPair<int,int>& move);//处理棋盘发过来的信号
    void handleFinalSent(const int final);//处理棋盘发过来的信号
    void handlePeaceTalk();//处理棋盘发过来的信号
private:
    QWebSocket *socket;
    QListWidget *onlinePlayerList;//玩家队列
    QTableWidget *roomList;//房间队列
    QTextEdit *systemMessages;//系统信息
    QLabel *currentStatusLabel;//当前状态
    QLabel *roomhostName;
    QLabel *opponentName;
    QPushButton *createRoomButton;//创建房间
    QPushButton *startGameButton;//开始游戏
    QPushButton *exitRoomButton;//退出房间
    QLineEdit *roomNameInput;//输入房间名
    QComboBox *handOptionCombo;//选择先后手
    QString username;
    board *myBoard;
    bool forceClose = false;
    bool isFirst;
    int nowstate;//0为空闲，1为房间等待中，2为对局中
    void CreateRoom();
    void updateOnlinePlayers(const QJsonArray &);
    void updateRooms(const QJsonArray &);
    void addSystemMessage(const QString &);
    void joinRoom(const QString &,const QString &,bool);
    void startGame();
    void exitRoom();
    void updateBoard(int x,int y);
    void updateButton();
    void updateCreateRoomButton();
    void updateStartGameButton();
    void updateExitRoomButton();
    void forceCloseWindow() {
        forceClose = true;
        close();
    }
    void closeEvent(QCloseEvent *event) override{
        if (forceClose) {
            event->accept();
            return;
        }
        if(nowstate==0){
            QMessageBox::StandardButton reply = QMessageBox::question(this, "Confirm", "Are you sure you want to exit?",
                QMessageBox::Yes | QMessageBox::No);
            if(reply==QMessageBox::Yes){
                socket->close();
                newWindow_start *newWindow = new newWindow_start(username);
                newWindow->show();
                event->accept();
            }
            else{
                event->ignore();
            }
        }else{
            QMessageBox::warning(this,"warning","please leave the room and exit");
            event->ignore();
        }
    }
    ~GameLobby();
};
#endif // VERSUS_LOBBY_H
