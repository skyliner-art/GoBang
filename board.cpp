#include "board.h"
#include "newWindow_end.h"
#include "newWindow_start.h"
#include <QThread>
#include <chrono>
#include <QUrl>
#include "mainwindow.h"
const int tsize = 15;//棋盘大小
const int offset = 30;//设置棋盘与原点的偏移量
ai* ai_object = nullptr;//人机对战ai类
void onDataSentResponse(QNetworkReply *reply);//调试网络数据传递
board::board(bool newstart,int type,QString username,QWidget *parent)
    : QWidget(parent),type(type),username(username),current_player(1),newstart(newstart),timer(nullptr),ai_player1(nullptr),ai_player2(nullptr)
{
    setFixedSize(700, 500);
    setWindowIcon(QIcon(":/images/image/icon.jpg"));
    take_back =new QPushButton("Take back",this);
    savegame =new QPushButton("SaveGame",this);
    give_up =new QPushButton("Give up",this);
    draw=new QPushButton("Draw",this);
    returnbutton = new QPushButton("Return",this);
    take_back->setGeometry(500, 50, 200, 50);
    savegame->setGeometry(500, 150, 200, 50);
    give_up->setGeometry(500, 250, 200, 50);
    draw->setGeometry(500, 350, 200, 50);
    returnbutton->setGeometry(500, 450, 200, 50);
    connect(take_back,&QPushButton::clicked,this,&board::takeback);
    connect(savegame,&QPushButton::clicked,this,&board::saveGame);
    connect(returnbutton,&QPushButton::clicked,this,&board::openNewWindow_start);
    connect(draw,&QPushButton::clicked,this,&board::Draw);
    connect(give_up,&QPushButton::clicked,this,&board::giveup);
    music1 = new QMediaPlayer(this);
    music1_volume = new QAudioOutput(this);
    music1->setAudioOutput(music1_volume);
    music1->setSource(QUrl("qrc:/audio/audio/place_stone.mp3"));  // 使用资源路径
    music1_volume->setVolume(1.0f);
    //以上为窗口过程
    last_pos.first=1;last_pos.second=1;
    chessboard.resize(16, vector<int>(16, 0));// 初始化15x15的棋盘，0表示空，1表示黑子，2表示白子
    countstep=0;//计数
    waitingForMove=false;
    if(type==2&&newstart)//人机模式进行先后手选择
    {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Choose First Move", "Do you want to go first?",QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::Yes)
        {
            // 用户选择了先手
            isfirst = true;
        }
        else
        {
            // 用户选择了后手
            isfirst = false;
            chessboard[7][7]=1;
            update();
            countstep++;
        }
        bool ok;
        int difficulty = QInputDialog::getInt(this, "Select Difficulty", "Enter AI's thinking depth:", 9, 5, 20, 1, &ok);

        if (ok) {
            qDebug() << "AI Thinking Depth set to:" << difficulty;
        } else {
            difficulty = 9;  // 你可以选择一个默认的值
            qDebug() << "AI Thinking Depth set to default:";
        }
        sentfirst(difficulty);
    }
    if (type == 3) {  // AI vs AI mode
        pause=new QPushButton("Pause",this);
        pause->setGeometry(250, 450, 100, 30);
        gamepause = false;
        ai_player1 = new ai(false, chessboard);
        ai_player2 = new ai(true, chessboard);
        chessboard[7][7]=1;
        countstep++;
        qDebug() << countstep<<" ai" << current_player << " move:"<<7<<' '<<7;
        qDebug() <<"ai"<<current_player<<':'<< ai_player1->new_evaluate_board(2-countstep%2,chessboard);
        board::addStack(7,7,ai_player1->new_evaluate_board(2-countstep%2,chessboard));
        current_player = 2;
        update();

        timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, [=]() {
            if(gamepause)
            {
                return;
            }
            pair<int, int> move;
            countstep++;
            if (current_player == 1)
            {
                move = ai_player1->ioput(countstep);
            }
            else
            {
                move = ai_player2->ioput(countstep);
            }
            chessboard[move.first][move.second] = current_player;
            qDebug() << countstep<<" ai" << current_player << " move:"<<move.first+1<<' '<<move.second+1;
            qDebug() <<"ai"<<current_player<<':'<< ai_player1->new_evaluate_board(2-countstep%2,chessboard);
            addStack(move.first,move.second,ai_player1->new_evaluate_board(2-countstep%2,chessboard));
            update();

            if (win(move.first, move.second, current_player))
            {
                qDebug() << "ai" << current_player << "获胜";
                QTimer::singleShot(1000, this, [this]()
                {
                    sentdata(record,2-(countstep%2));
                });
                openNewWindow_end();
                delete ai_player1;
                delete ai_player2;
                timer->stop();
            }

            current_player = 3 - current_player;
            timer->setInterval(1000);
        });
        timer->start(1000);
        connect(pause, &QPushButton::clicked, this, [&]()
        {
            if (pause->text() == "Pause")
            {
                gamepause=true;
                pause->setText("Continue");
            }
            else
            {
                gamepause=false;
                pause->setText("Pause");
            }
        });
    }
}
void board::update_last_position(int player,QString opponent,int x,int y)
{
    if(player==1&&isfirst)
    {
        last_black_position->setText("step "+QString::number(countstep)+" black: "+username+" x: "+QString::number(x+1)+" y: "+QString::number(y+1));
    }
    else if(player==1&&!isfirst)
    {
        last_black_position->setText("step "+QString::number(countstep)+" black: "+opponent+  "x: "+QString::number(x+1)+" y: "+QString::number(y+1));
    }
    else if(player==2&&isfirst)
    {
        last_white_position->setText("step "+QString::number(countstep)+" white: "+opponent+  " x: "+QString::number(x+1)+" y: "+QString::number(y+1));
    }
    else
    {
        last_white_position->setText("step "+QString::number(countstep)+" white: "+username+" x: "+QString::number(x+1)+" y: "+QString::number(y+1));
    }
}
void board::sentfirst(int difficulty)//发送先手信息给ai
{
    if(isfirst)
    {
        last_black_position = new QLabel("step 0 black: "+username,this);
        last_white_position = new QLabel("step 0 white: ai",this);
        last_black_position->setGeometry(50, 450, 200, 30);
        last_white_position->setGeometry(250, 450, 200, 30);
    }
    else
    {
        last_black_position = new QLabel("step 1 black: ai x:8 y:8",this);
        last_white_position = new QLabel("step 0 white: "+username,this);
        last_black_position->setGeometry(50, 450, 200, 30);
        last_white_position->setGeometry(250, 450, 200, 30);
    }
    ai_object = new ai(isfirst,chessboard);
    ai_object->startdepth=difficulty;
}
void board::saveGame()//保存为未完成的游戏
{
    if(type==1){
        QMessageBox::warning(this,"Warning","PVP cannot save games");
        return;
    }
    sentdata(record,3);
}
void board::giveup()
{
    sentdata(record,isfirst+1);//是先手则后手赢，不是先手则是先手赢，1对应2，0对应1
    qDebug() <<"you give up";
    board::openNewWindow_end();
}
void board::Draw()
{
    if(type==1){
        qDebug() <<"Draw";
        emit sue_for_peace();
    }else if(type==2){
        QMessageBox::warning(this,"Warning","Fight AI to the End!");
        return;
    }else if(type==3||type==0){
        QMessageBox::warning(this,"Warning","Ridiculous!");
        return;
    }
}
void board::sentdata(QStack<GameData> gameDataStack,int state)
{
    QJsonArray jsonData;
    while (!gameDataStack.isEmpty()) {
        GameData gameData = gameDataStack.pop();
        jsonData.append(gameData.toJson());
    }
    qDebug()<<jsonData;
    QJsonObject json;
    json["gameData"] = jsonData;
    QJsonArray jsonboard;
    for (const auto& row : chessboard) {
        QJsonArray rowArray;
        for (int cell : row) {
            rowArray.append(cell);
        }
        jsonboard.append(rowArray);
    }
    json["nowBoard"]=jsonboard;
    json["state"] = state;
    json["step"] = countstep;
    if(isfirst&&type==2)
    {
        json["firstPlayer"] = username;
        json["secondPlayer"] = "ai";
    }
    else if(!isfirst&&type==2)
    {
        json["firstPlayer"] = "ai";
        json["secondPlayer"] = username;
    }
    else if(type==0)
    {
        json["firstPlayer"] = username;
        json["secondPlayer"] = username;
    }
    else if(isfirst&&type==1)
    {
        json["firstPlayer"] = username;
        json["secondPlayer"] = opponent_name;
    }
    else if(!isfirst&&type==1)
    {
        json["firstPlayer"] = opponent_name;
        json["secondPlayer"] = username;
    }
    else if(type==3)
    {
        json["firstPlayer"] = "ai";
        json["secondPlayer"] = "ai";
    }
    else
    {
        return;
    }
    QJsonDocument doc(json);
    QByteArray requestData = doc.toJson();

    // 创建请求并发送
    QNetworkRequest request(QUrl(appurl+"gobang/gamedata/storage"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    manager->post(request, requestData);
    connect(manager, &QNetworkAccessManager::finished, this, &onDataSentResponse);
}
void board::paintEvent(QPaintEvent* event)
{
    if(!record.empty()){
        last_pos=record.top().recordstep;
    }
    QPainter painter(this);
    QPixmap background(":/images/image/chessboard.jpg");  // 从资源中加载背景图
    painter.drawPixmap(0, 0, 450 + offset, 450 + offset, background);  // 绘制背景图
    painter.setPen(Qt::black);

    // 绘制网格
    for (int i = 0; i < 15; ++i) {
        // 绘制竖线
        painter.drawLine(offset + 30 * i, offset, offset + 30 * i, 420 + offset);
        // 绘制横线
        painter.drawLine(offset, offset + 30 * i, 420 + offset, offset + 30 * i);
    }

    // 绘制行和列索引
    QFont font = painter.font();
    font.setPointSize(10);
    painter.setFont(font);
    painter.setPen(Qt::black);
    // 绘制列索引（从上到下，顶部显示）
    for (int i = 0; i < 15; ++i)
    {
        //下述操作为了对齐数字与格线
        QString text = QString::number(i + 1);  // 获取列索引数字
        QFontMetrics fm(painter.font());  // 获取字体度量
        int textWidth = fm.horizontalAdvance(text);  // 获取文本宽度
        int xPosition = i * 30 + (30 - textWidth) / 2+15;  // 计算居中的x位置
        painter.drawText(xPosition, 15, text);  // 绘制文本
    }
    // 绘制行索引（从左到右，左侧显示）
    for (int i = 0; i < 15; ++i)
    {
        //下述操作为了对齐数字与格线
        QString text = QString::number(i + 1);  // 获取列索引数字
        QFontMetrics fm(painter.font());  // 获取字体度量
        int textWidth = fm.horizontalAdvance(text);  // 获取文本宽度
        int xPosition = (30 - textWidth) / 2;  // 计算居中的x位置
        painter.drawText(xPosition-5, i * 30 + 35,text); // 绘制行号
    }
    // 绘制棋子
    for (int x = 0; x < 15; ++x) {
        for (int y = 0; y < 15; ++y) {
            if (chessboard[x][y] == 1)
            {  // 黑子
                if (x == last_pos.first && y == last_pos.second)
                {
                    QPen pen;
                    pen.setColor(Qt::red);  // 设置边框颜色为红色
                    pen.setWidth(3);  // 设置边框宽度
                    painter.setPen(pen);  // 应用边框设置
                    painter.setBrush(Qt::black);
                    painter.drawEllipse(x * 30 - 10 + offset, y * 30 - 10 + offset, 20, 20);
                     painter.setPen(Qt::NoPen);
                }
                else
                {
                    painter.setBrush(Qt::black);
                    painter.drawEllipse(x * 30-10+offset, y * 30 -10+offset, 20, 20);
                }
            }
            else if (chessboard[x][y] == 2)
            {  // 白子
                if (x == last_pos.first && y == last_pos.second)
                {
                    QPen pen;
                    pen.setColor(Qt::red);  // 设置边框颜色为红色
                    pen.setWidth(3);  // 设置边框宽度
                    painter.setPen(pen);  // 应用边框设置
                    painter.setBrush(Qt::white);
                    painter.drawEllipse(x * 30 - 10 + offset, y * 30 - 10 + offset, 20, 20);
                     painter.setPen(Qt::NoPen);
                }
                painter.setBrush(Qt::white);
                painter.drawEllipse(x * 30 -10+offset, y * 30 -10+offset, 20, 20);
            }
        }
    }
}
void board::mousePressEvent(QMouseEvent* event)
{
    if(waitingForMove) return;
    int x = qFloor((event->position().x() - offset+15) / 30);
    int y = qFloor((event->position().y() - offset+15) / 30);

    if (x < 0 || x >= 15 || y < 0 || y >= 15||chessboard[x][y] != 0) {
        qDebug() << "Out of bounds click ignored.";
        return;
    }
    countstep++;
    if(countstep%2==1)
    {
        chessboard[x][y] = 1;
        if(isOverline(x,y,chessboard))
        {
            chessboard[x][y] = 0;
            countstep--;
            QMessageBox::warning(this, "Invalid Move", "You cannot place a stone on this position. It's a banned position!");
            return;
        }
    }
    music1->play();
    if (countstep%2==1)
    {
        chessboard[x][y] = 1;  // 黑子
        update();
    }
    else if(countstep%2==0)
    {
        chessboard[x][y] = 2;//白子
        update();
    }
    long long humman_score = 0;
    // if(type==2)
    // {
    //     humman_score = ai_object->new_evaluate_board(2-countstep%2,chessboard);
    // }
    addStack(x,y,-humman_score);
    if (win(x, y, 2 - (countstep % 2)))
    {
        sentdata(record,2-(countstep%2));
        qDebug() <<"you win";
        openNewWindow_end();
        return;
    }
    if(countstep%2==1&&checkBan(x,y,chessboard))
    {
        QMessageBox::warning(this, "Invalid Move", "You cannot place a stone on this position. It's a banned position!");
        record.pop();
        countstep--;
        chessboard[x][y] = 0;
        update();
        return;
    }
    qDebug() <<countstep<<"your move"<< " x:" << x+1 << " y:" << y+1;
    if(type==2){
        update_last_position(2-countstep%2,"ai",x,y);
    }
    if(countstep==255||countstep==254)
    {
        sentdata(record,4);
        qDebug() <<"draw";
        openNewWindow_end();
        return;
    }
    if(type==1)//如果为PVP
    {
        waitingForMove = true;
        emit moveSent(QPair<int, int>(x, y));
    }
    if(type==2)//如果为人机模式
    {
        waitingForMove = true;
        QTimer::singleShot(500, this, [this]() {
            countstep++;
            // board::callmcts();
            callaiScript();
        });
    }
}
void board::callaiScript()
{
    if(ai_object)//保护，确保ai对象已经存在
    {
        auto start = chrono::high_resolution_clock::now();
        pair<int,int> move = ai_object->ioput(countstep);
        auto end = chrono::high_resolution_clock::now();

        // 计算时间差
        chrono::duration<double> duration = end - start;

        // 打印当前一步的计算时间
        qDebug() << "Time taken for this move: " << duration.count() << " seconds";
        qDebug() <<countstep<<"ai move"<< " x:" << move.first+1 << " y:" << move.second+1;
        chessboard[move.first][move.second]=2-(countstep%2);
        music1->play();
        board::addStack(move.first,move.second,onAiMoveCalculated(2-countstep%2));
        update();
        update_last_position(2-countstep%2,"ai",move.first,move.second);
        if(win(move.first,move.second,2-(countstep%2)))
        {
            QTimer::singleShot(1000, this, [this]() {
                sentdata(record,2-(countstep%2));
            });
            qDebug() <<"AI win";
            board::openNewWindow_end();
        }
        waitingForMove=false;
    }
}
long long board::onAiMoveCalculated(int human_player)
{
    int dx[4]={1, 0, 1, 1};int dy[4]={0, 1, 1, -1};
    long long score=0;
    int center[2]={7,7};
    for(int i=0;i<15;i++)
    {
        for(int j=0;j<15;j++)
        {
            if (chessboard[i][j]!=0)
            {
                int now_player=chessboard[i][j];
                int distance_to_center = abs(i - center[0]) + abs(j - center[1]);//距离中心的曼哈顿距离计算
                int center_bonus = max(0, 5 - distance_to_center);  // 给中心位置加分
                if (now_player == human_player) score += center_bonus;
                else score-=center_bonus; //增加/减少到评分中
                long long movepoint = 0; //该点分数
                for(int k=0;k<4;k++)
                {
                    int dxx=dx[k],dyy=dy[k];
                    pair<int,int>set = count_continuous_chess(i, j, now_player, dxx, dyy,chessboard);
                    if (set.first == 2)
                    {
                        if (set.second == 2)
                        {
                            movepoint += (now_player == human_player) ? 10 : -10;
                        }
                        else if (set.second == 1)
                        {
                            movepoint += (now_player == human_player) ? 1 : -1;
                        }
                        else
                        {
                            movepoint += (now_player == human_player) ? 0.5 : -0.5;
                        }
                    }
                    else if (set.first == 3)
                    {
                        if (set.second == 2)
                        {
                            movepoint += (now_player == human_player) ? 500 : -700;
                        }
                        else if (set.second == 1)
                        {
                            movepoint += (now_player == human_player) ? 100 : -100;
                        }
                        else
                        {
                            movepoint += (now_player == human_player) ? 30 : -30;
                        }
                    }
                    else if (set.first == 4)
                    {
                        if (set.second == 2)
                        {
                            movepoint += (now_player == human_player) ? 20000 : -22000;
                        }
                        else if (set.second == 1)
                        {
                            movepoint += (now_player ==human_player) ? 4000 : -4000;
                        }
                        else
                        {
                            movepoint += (now_player == human_player) ? 300 : -300;
                        }
                    }
                    else if (set.first >= 5)
                    {
                        return (now_player == human_player) ? 1000000000 : -1000000010;
                    }
                }
                if (abs(movepoint) >= 1000)
                {
                    movepoint *= 10;
                }
                score += movepoint;
            }
        }
    }
    return score;
}
void board::takeback()//悔棋
{
    if(!record.empty()&&type==2)//人机模式悔棋，每次悔棋连着上次机器下的一起撤回
    {
        chessboard[record.top().recordstep.first][record.top().recordstep.second]=0;
        record.pop();
        countstep--;
        chessboard[record.top().recordstep.first][record.top().recordstep.second]=0;
        record.pop();
        countstep--;
        update();
    }
    else if(!record.empty()&&type==0)//线下人人模式悔棋
    {
        chessboard[record.top().recordstep.first][record.top().recordstep.second]=0;
        record.pop();
        countstep--;
        update();
    }
    else if(type==1){
        QMessageBox::warning(this,"Warning","It's no use crying over split milk.");
        return;
    }
}
void board::addStack(int x,int y,long long score)//数据入栈
{
    GameData cur;
    QPair<int,int> temp;//记录每一步于栈中,为后续悔棋和传输数据做准备
    temp.first=x;temp.second=y;
    cur.recordstep=temp;
    cur.recordscore=score;
    record.push(cur);
}
bool board::win(int x,int y,int c)
{

    int count1=0;
    int count2=0;
    for(int j=1;j<5;j++)
    {
        if(x+j<15&&chessboard[x+j][y]==c) count1++;
        else break;
    }
    for(int j=1;j<5;j++)
    {

        if(x-j>=0&&chessboard[x-j][y]==c) count2++;
        else break;
    }
    if(count1+count2>=4)return true;
    count1=0;
    count2=0;
    for(int j=1;j<5;j++)
    {
        if(y+j<15&&chessboard[x][y+j]==c) count1++;
        else break;
    }
    for(int j=1;j<5;j++)
    {

        if(y-j>=0&&chessboard[x][y-j]==c) count2++;
        else break;
    }
    if(count1+count2>=4)return true;
    count1=0;
    count2=0;
    for(int j=1;j<5;j++)
    {
        if(y+j<15&&x+j<15&&chessboard[x+j][y+j]==c) count1++;
        else break;
    }
    for(int j=1;j<5;j++)
    {

        if(y-j>=0&&x-j>=0&&chessboard[x-j][y-j]==c) count2++;
        else break;
    }
    if(count1+count2>=4)return true;
    count1=0;
    count2=0;
    for(int j=1;j<5;j++)
    {
        if(y+j<15&&x-j>=0&&chessboard[x-j][y+j]==c) count1++;
        else break;
    }
    for(int j=1;j<5;j++)
    {

        if(y-j>=0&&x+j<15&&chessboard[x+j][y-j]==c) count2++;
        else break;
    }
    if(count1+count2>=4)return true;
    return false;
}
void board::openNewWindow_end()
{
    newWindow_end *NewWindow;
    if(countstep==255||countstep==254)
    {
        NewWindow = new newWindow_end(type,4,username);
        emit sentfinal(0);
    }
    else
    {
        if(isfirst&&countstep%2==1||!isfirst&&countstep%2==0){
            emit sentfinal(1);
        }else{
            emit sentfinal(-1);
        }
        NewWindow = new newWindow_end(type,countstep%2,username);
    }
    NewWindow->show();
    disconnect(this, nullptr, nullptr, nullptr);
    this->forceClose();
}
void board::openNewWindow_start()
{
    if(type==1){
        QMessageBox::warning(this,"Warning","It's shameful to be a deserter.");
        return;
    }
    newWindow_start *NewWindow = new newWindow_start(username);
    NewWindow->show();
    this->forceClose();
}
void onDataSentResponse(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError)
    {
        QByteArray response = reply->readAll();
        qDebug() << "Data sent response: " << response;
    }
    else
    {
        qWarning() << "Error: " << reply->errorString();
    }
    reply->deleteLater();
}
board::~board()
{
}
void board::closeEvent(QCloseEvent *event){
    if(type==1&&!isForcedClose){
        QMessageBox::warning(this,"Warning","NO STEP BACK");
         event->ignore();
    }else{
        if (timer) {
            timer->stop();
            delete timer;
            timer = nullptr;
        }
        event->accept();
    }
}
bool check_long(int player,int cx,int cy,vector<vector<int>> &chessboard)
{
    int dx[4]={1,0,1,-1};int dy[4]={0,1,1,1};
    for(int i=0;i<4;i++)
    {
        int dxx=dx[i],dyy=dy[i];
        int count = -1;  //减去重复算的原点棋子
        int original_x=cx, original_y = cy;
        // 正方向
        while(is_in_board(cx, cy)&&chessboard[cx][cy] == player)
        {
            count++;
            cx+=dxx;cy+=dyy;
        }
        cx=original_x;cy =original_y; //恢复到原始位置
        while(is_in_board(cx, cy)&&chessboard[cx][cy] == player)
        {
            count++;
            cx-=dxx;cy-=dyy;
        }
        if(count>5) return true;
    }
    return false;
}
