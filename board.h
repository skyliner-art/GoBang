#ifndef BOARD_H
#define BOARD_H
#include <QApplication>
#include <QWidget>
#include <QPainter>
#include <QMouseEvent>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTimer>
#include <QTextStream>
#include <QThread>
#include <QStack>
#include <vector>
#include "ai.h"
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QInputDialog>
using namespace std;
class board : public QWidget
{
    Q_OBJECT
    friend class GameLobby;
public:
    board(bool newstart,int type,QString username,QWidget *parent = nullptr);
    struct GameData
    {
        QPair<int,int> recordstep;
        int recordscore;
        QJsonObject toJson()
        {
            QJsonObject json;
            QJsonArray jsonstep;
            jsonstep.append(recordstep.first);
            jsonstep.append(recordstep.second);
            json["recordstep"]=jsonstep;
            json["recordscore"]=recordscore;
            return json;
        }
        static GameData fromJson(const QJsonObject &json)
        {
            GameData gameData;
            QJsonArray jsonstep = json["recordstep"].toArray();
            gameData.recordstep = QPair<int, int>(jsonstep[0].toInt(), jsonstep[1].toInt());
            gameData.recordscore = json["recordscore"].toInt();
            return gameData;
        }
    };
    QStack<GameData> record;
    vector<vector<int>> chessboard; //记录棋盘落子
    QString username;
    QString opponent_name;
    int countstep;//奇数黑子下，偶数白子下
    int type;//记录对局类型，0为线下人人，1为线上人人，2为人机,3为ai自我对弈
    int current_player;//当前下棋方
    bool isfirst;
    bool waitingForMove;
    bool isForcedClose=false;
    bool newstart;//是否为新的开始，还是读取存档
    void openChessBoard();
    void sentfirst(int difficulty);//发送先后手信息给ai
    void mousePressEvent(QMouseEvent* event) override;//处理鼠标点击事件
    void update_last_position(int player,QString opponent,int x,int y);//更新当前最后一次位置便于标记
    void addStack(int x,int y,long long score);//记录每次操作并压入栈中
    void takeback();//悔棋(finished人机模式）联机传输TODO
    void callaiScript();//人机模式ai
    void callmcts();//唤起MCTS算法(TODO)
    long long onAiMoveCalculated(int human_player);//ai计算当前局面评分
    bool detect_prohibited_hands(int x,int y,int c);//TODO禁手检测
    bool win(int x,int y,int c);//判断是否胜利
    void giveup();//认负
    void Draw();//求和
    void openNewWindow_end();//打开结束界面
    void openNewWindow_start();//打开开始界面
    void saveGame();
    void sentdata(QStack<GameData>,int state);//发送数据到服务器，state=1为先手胜，2为后手胜，3为未结束,4为平局
    ~board();
signals:
    void aiMoveCalculated(int x, int y);
    void moveSent(const QPair<int,int>& move);//pvpSocket信号
    void sentfinal(int);//pvp信号,-1为输，0为平局，1为胜利
    void sue_for_peace();//pvp信号,求和
private:

    QPushButton *take_back;//悔棋
    QPushButton *savegame;//保存游戏
    QPushButton *give_up;//认输TODO
    QPushButton *draw;//求和TODO
    QPushButton *returnbutton;//返回
    QPushButton *pause;//ai对ai暂停键
    QLabel *last_black_position;//上一次黑方位置
    QLabel *last_white_position;//上一次白方位置
    QMediaPlayer* music1;//落子音效
    QAudioOutput* music1_volume; //落子音量
    pair<int,int> last_pos;//记录上次落子位置便于标示
    void paintEvent(QPaintEvent*) override;//绘图
    bool gamepause;//ai对ai暂停棋局
    ai* ai_player1;
    ai* ai_player2;
    QTimer *timer;
    void closeEvent(QCloseEvent *event) override;
    void forceClose() {
        isForcedClose = true;
        close();
    }
};

//禁手检测子函数
bool check_long(int player,int cx,int cy,vector<vector<int>> &chessboard);
bool check_four(int player,int cx ,int cy,vector<vector<int>> &chessboard);
bool check_three(int player,int cx ,int cy,vector<vector<int>> &chessboard);
#endif // BOARD_H
