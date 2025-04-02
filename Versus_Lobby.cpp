#include"Versus_Lobby.h"
#include"mainwindow.h"
#include"newWindow_end.h"
GameLobby::GameLobby(QString username,QWidget *parent) : QWidget(parent),username(username)
{
    // 设置WebSocket连接
    socket = new QWebSocket();
    connect(socket, &QWebSocket::connected, this, &GameLobby::onConnected);
    connect(socket, &QWebSocket::disconnected, this, &GameLobby::Disconnected);
    connect(socket, &QWebSocket::textMessageReceived, this, &GameLobby::getMessage);
    qDebug() << "Opening WebSocket..."+serverurl;
    socket->open(QUrl(serverurl));
    setWindowIcon(QIcon(":/images/image/icon.jpg"));
    onlinePlayerList = new QListWidget(this);
    roomList = new QTableWidget(this);
    roomList->setColumnCount(5);
    roomList->setHorizontalHeaderLabels({"Room","FirstPlayer","SecondPLayer","State","Join"});
    systemMessages = new QTextEdit(this);
    systemMessages->setReadOnly(true);
    nowstate = 0;
    opponent_name="";
    roomhostName = new QLabel(username,this);
    opponentName = new QLabel(opponent_name,this);
    currentStatusLabel = new QLabel("free", this);

    // 创建房间界面
    createRoomButton = new QPushButton("create room", this);
    roomNameInput = new QLineEdit(this);
    roomNameInput->setPlaceholderText("Please enter the room name");

    // 选择先手或后手
    handOptionCombo = new QComboBox(this);
    handOptionCombo->addItem("choose first");
    handOptionCombo->addItem("choose second");

    //显示当前所在房间
    startGameButton = new QPushButton("start game",this);
    exitRoomButton = new QPushButton("exit room",this);
    // 布局
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(new QLabel("Online Players"));
    mainLayout->addWidget(onlinePlayerList);
    mainLayout->addWidget(new QLabel("Room List"));
    mainLayout->addWidget(roomList);

    // 创建房间部分
    QHBoxLayout *createRoomLayout = new QHBoxLayout;
    createRoomLayout->addWidget(roomNameInput);
    createRoomLayout->addWidget(handOptionCombo);
    createRoomLayout->addWidget(createRoomButton);

    //操控房间部分
    QHBoxLayout *roomControlLayout = new QHBoxLayout;
    roomControlLayout->addWidget(exitRoomButton);
    roomControlLayout->addWidget(startGameButton);
    roomControlLayout->addWidget(roomhostName);
    roomControlLayout->addWidget(opponentName);

    mainLayout->addLayout(createRoomLayout);
    mainLayout->addLayout(roomControlLayout);
    mainLayout->addWidget(currentStatusLabel);
    mainLayout->addWidget(new QLabel("SystemMessage"));
    mainLayout->addWidget(systemMessages);

    setLayout(mainLayout);
    updateStartGameButton();
    updateExitRoomButton();
    myBoard = new board(true,1,username);
    connect(myBoard, &board::moveSent, this, &GameLobby::handleMoveSent);
    connect(myBoard, &board::sentfinal, this, &GameLobby::handleFinalSent);
    connect(myBoard, &board::sue_for_peace, this, &GameLobby::handlePeaceTalk);
    connect(createRoomButton, &QPushButton::clicked, this, &GameLobby::CreateRoom);
}
void GameLobby::exitRoom()
{
    QJsonObject request;
    request["action"] = "exitRoom";
    request["username"] = username;
    QJsonDocument doc(request);
    socket->sendTextMessage(doc.toJson(QJsonDocument::Compact));
    nowstate=0;
    opponent_name="";
    opponentName->setText(opponent_name);
    updateButton();
}
void GameLobby::startGame()
{
    QJsonObject request;
    request["action"] = "startGame";
    request["username"] = opponent_name;
    QJsonDocument doc(request);
    socket->sendTextMessage(doc.toJson(QJsonDocument::Compact));
    qDebug()<<"start game";
    if (myBoard) {
        disconnect(myBoard, &board::moveSent, this, &GameLobby::handleMoveSent);
        disconnect(myBoard, &board::sentfinal, this, &GameLobby::handleFinalSent);
        disconnect(myBoard, &board::sue_for_peace, this, &GameLobby::handlePeaceTalk);
        delete myBoard;
    }
    currentStatusLabel->setText("Playing");
    updateButton();
    myBoard = new board(true,1,username);
    connect(myBoard, &board::moveSent, this, &GameLobby::handleMoveSent);
    connect(myBoard, &board::sentfinal, this, &GameLobby::handleFinalSent);
    connect(myBoard, &board::sue_for_peace, this, &GameLobby::handlePeaceTalk);
    myBoard->isfirst=isFirst;
    myBoard->opponent_name=opponent_name;
    if(isFirst){
        myBoard->waitingForMove=false;
        myBoard->last_black_position = new QLabel("step 1 black: "+username,myBoard);
        myBoard->last_white_position = new QLabel("step 0 white: "+opponent_name,myBoard);
        myBoard->last_black_position->setGeometry(50, 450, 200, 30);
        myBoard->last_white_position->setGeometry(250, 450, 200, 30);
    }else {
        myBoard->waitingForMove=true;
        myBoard->last_black_position = new QLabel("step 1 black: "+opponent_name,myBoard);
        myBoard->last_white_position = new QLabel("step 0 white: "+username,myBoard);
        myBoard->last_black_position->setGeometry(50, 450, 200, 30);
        myBoard->last_white_position->setGeometry(250, 450, 200, 30);
    }
    myBoard->show();
}
void GameLobby::handleMoveSent(const QPair<int,int>& move)
{
    myBoard->update_last_position(2-isFirst,opponent_name,move.first,move.second);
    myBoard->update();
    QJsonObject request;
    request["action"] = "sentMove";
    request["recipient"] = opponent_name;
    request["x"] = move.first;
    request["y"] = move.second;
    QJsonDocument doc(request);
    socket->sendTextMessage(doc.toJson(QJsonDocument::Compact));
}
void GameLobby::handleFinalSent(const int final)
{
    QJsonObject request;
    request["action"] = "sentResult";
    request["recipient"] = opponent_name;
    if(final==1){
        request["result"] = "Lose";
    }else if(final==-1){
        request["result"] = "Win";
    }else{
        request["result"] = "Draw";
    }
    QJsonDocument doc(request);
    socket->sendTextMessage(doc.toJson(QJsonDocument::Compact));
    currentStatusLabel->setText("Waiting");
    updateButton();
}
void GameLobby::handlePeaceTalk()
{
    QJsonObject request;
    qDebug()<<"Sue For Peace";
    request["action"] = "SueForPeace";
    request["promoter"] = username;
    request["recipient"] = opponent_name;
    QJsonDocument doc(request);
    socket->sendTextMessage(doc.toJson(QJsonDocument::Compact));
}
void GameLobby::updateBoard(int x,int y)
{
    myBoard->countstep++;
    myBoard->addStack(x,y,0);
    if(isFirst){
        myBoard->chessboard[x][y]=2;
        myBoard->update_last_position(2,opponent_name,x,y);
    }else{
        myBoard->chessboard[x][y]=1;
        myBoard->update_last_position(1,opponent_name,x,y);
    }
    myBoard->update();
    myBoard->waitingForMove=false;
}
void GameLobby::CreateRoom()
{
    QString roomName = roomNameInput->text();
    QString handChoice = handOptionCombo->currentText();
    QJsonObject request;
    request["action"] = "createRoom";
    request["roomname"]=roomName;
    if (roomName.isEmpty())
    {
        QMessageBox::warning(this, "Error", "The room name can't be empty");
            return;
    }
    if (handChoice == "choose first")
    {
        QMessageBox::information(this, "Create Room", "You have chosen to play first. The room has been created successfully!");
        isFirst=true;
        request["firstusername"]=username;
        request["secondusername"]="";
    }
    else if (handChoice == "choose second")
    {
        isFirst=false;
        request["firstusername"]="";
        request["secondusername"]=username;
    }
    QJsonDocument doc(request);
    socket->sendTextMessage(doc.toJson(QJsonDocument::Compact));
}

void GameLobby::updateOnlinePlayers(const QJsonArray &players)
{
    QStringList playerList;
    for(auto player:players){
        QString playerName = player.toString();
        playerList.append(playerName);
    }
    onlinePlayerList->clear();
    onlinePlayerList->addItems(playerList);
}
void GameLobby::updateRooms(const QJsonArray &rooms)
{
    roomList->clear();
    roomList->setRowCount(0);
    roomList->setHorizontalHeaderLabels({"Room","FirstPlayer","SecondPLayer","State","Join"});
    for (const QJsonValue &roomValue : rooms) {
        if (roomValue.isObject()) {
            QJsonObject room = roomValue.toObject();
            QString roomName = room["name"].toString();
            QString firstPlayer=room["FirstPlayerName"].toString();
            QString secondPlayer=room["SecondPlayerName"].toString();
            if((secondPlayer=="")&&(firstPlayer=="")){
                continue;
            }
            if((firstPlayer==username&&secondPlayer!="")||(firstPlayer!=""&&secondPlayer==username)){
                nowstate=2;
                opponent_name=(firstPlayer==username)? secondPlayer:firstPlayer;
                opponentName->setText(opponent_name);
                currentStatusLabel->setText("Waiting");
                updateButton();
            }else if(firstPlayer==username||secondPlayer==username){
                nowstate=1;
                currentStatusLabel->setText("Waiting");
                opponent_name="";
                opponentName->setText(opponent_name);
                updateButton();
            }
            int row = roomList->rowCount();
            roomList->insertRow(row);
            roomList->setItem(row, 0, new QTableWidgetItem(roomName));
            roomList->setItem(row, 1, new QTableWidgetItem(firstPlayer));
            roomList->setItem(row, 2, new QTableWidgetItem(secondPlayer));
            if(firstPlayer==""&&secondPlayer!=username||secondPlayer==""&&firstPlayer!=username)//房间未满
            {
                QString roomHost="";
                bool ismyfirst;
                roomList->setItem(row,3,new QTableWidgetItem("Waiting"));
                QPushButton *joinButton = new QPushButton("Join");
                if(firstPlayer==""){
                    ismyfirst= true;
                    roomHost = secondPlayer;
                }else {
                    ismyfirst = false;
                    roomHost = firstPlayer;
                }
                connect(joinButton, &QPushButton::clicked, this, [this, roomName,roomHost,ismyfirst](){
                    joinRoom(roomName,roomHost,ismyfirst);
                });
                roomList->setCellWidget(row, 4, joinButton);
            }
            else
            {
                if(firstPlayer!=""&&secondPlayer!=""){
                    roomList->setItem(row,3,new QTableWidgetItem("Full"));
                }else{
                    roomList->setItem(row,3,new QTableWidgetItem("Waiting"));
                }
            }
        }
    }
    roomList->update();
}

void GameLobby::addSystemMessage(const QString &message)
{
    systemMessages->append(message);
}
void GameLobby::onConnected()
{
    qDebug()<<"WebSocket connected successfully!";
    QJsonObject request;
    request["action"]="addNewUser";
    request["username"]=username;
    QJsonDocument doc(request);
    socket->sendTextMessage(doc.toJson(QJsonDocument::Compact));
}
void GameLobby::Disconnected()
{
    qDebug() << "WebSocket disconnected!";
}
void GameLobby::getMessage(const QString &message)
{
    qDebug() << "Message received: " << message;

    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
    if (doc.isObject()) {
        QJsonObject json = doc.object();
        QString action = json["action"].toString();

        if (action == "updateRooms") {
            QJsonArray RoomsArray = json["rooms"].toArray();
            updateRooms(RoomsArray);
        }else if(action =="updatePlayers"){
            QJsonArray PlayersArray = json["players"].toArray();
            updateOnlinePlayers(PlayersArray);
        }else if(action =="message"){
            const QString message = json["notice"].toString();
            addSystemMessage(message);
        }else if(action =="CreateRoom Error"){
            QMessageBox::warning(this,"CreateRoom Error","Your room name has been occupied");
        }else if(action =="addUser Error"){
            QMessageBox::warning(this,"addUser Error","The username is online");
            newWindow_start *newWindow = new newWindow_start(username);
            newWindow->show();
            this->forceCloseWindow();
        }else if(action =="startGame"){
            if (myBoard) {
                disconnect(myBoard, &board::moveSent, this, &GameLobby::handleMoveSent);
                disconnect(myBoard, &board::sentfinal, this, &GameLobby::handleFinalSent);
                disconnect(myBoard, &board::sue_for_peace, this, &GameLobby::handlePeaceTalk);
                delete myBoard;
            }
            currentStatusLabel->setText("Playing");
            updateButton();
            myBoard = new board(true,1,username);
            connect(myBoard, &board::moveSent, this, &GameLobby::handleMoveSent);
            connect(myBoard, &board::sentfinal, this, &GameLobby::handleFinalSent);
            connect(myBoard, &board::sue_for_peace, this, &GameLobby::handlePeaceTalk);
            myBoard->isfirst = isFirst;
            myBoard->opponent_name=opponent_name;
            if(isFirst){
                myBoard->waitingForMove=false;
                myBoard->last_black_position = new QLabel("step 1 black: "+username,myBoard);
                myBoard->last_white_position = new QLabel("step 0 white: "+opponent_name,myBoard);
                myBoard->last_black_position->setGeometry(50, 450, 200, 30);
                myBoard->last_white_position->setGeometry(250, 450, 200, 30);
            }else {
                myBoard->waitingForMove=true;
                myBoard->last_black_position = new QLabel("step 1 black: "+opponent_name,myBoard);
                myBoard->last_white_position = new QLabel("step 0 white: "+username,myBoard);
                myBoard->last_black_position->setGeometry(50, 450, 200, 30);
                myBoard->last_white_position->setGeometry(250, 450, 200, 30);
            }
            myBoard->show();
        }else if(action =="updateMove"){
            int x = json["x"].toInt();
            int y = json["y"].toInt();
            updateBoard(x,y);
        }else if(action =="GameOver"){
            const QString message = json["result"].toString();

            newWindow_end *NewWindow_end;
            if(message=="Lose"){
                NewWindow_end = new newWindow_end(1,!isFirst,username);
                myBoard->sentdata(myBoard->record,isFirst+1);//是先手则后手赢，不是先手则是先手赢，1对应2，0对应1
            }else if(message=="Win"){
                NewWindow_end = new newWindow_end(1,isFirst,username);
                myBoard->sentdata(myBoard->record,2-isFirst);//是先手则先手赢，不是先手则是后手赢，1对应1，0对应2
            }else{
                NewWindow_end = new newWindow_end(1,4,username);
                myBoard->sentdata(myBoard->record,4);
            }
            disconnect(myBoard, &board::moveSent, this, &GameLobby::handleMoveSent);
            disconnect(myBoard, &board::sentfinal, this, &GameLobby::handleFinalSent);
            disconnect(myBoard, &board::sue_for_peace, this, &GameLobby::handlePeaceTalk);
            NewWindow_end->show();
            myBoard->forceClose();
            currentStatusLabel->setText("Waiting");
            updateButton();
        }else if(action == "RequestDraw"){
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(this, "Choose First Move", "Do you agree to the tie request?",QMessageBox::Yes | QMessageBox::No);
            if(reply == QMessageBox::Yes){
                QJsonObject request;
                request["action"]="AgreeToDraw";
                request["recipient"]=opponent_name;
                request["promoter"]=username;
                QJsonDocument doc(request);
                socket->sendTextMessage(doc.toJson(QJsonDocument::Compact));
                myBoard->sentdata(myBoard->record,4);
                newWindow_end *NewWindow_end;
                NewWindow_end = new newWindow_end(1,4,username);
                disconnect(myBoard, &board::moveSent, this, &GameLobby::handleMoveSent);
                disconnect(myBoard, &board::sentfinal, this, &GameLobby::handleFinalSent);
                disconnect(myBoard, &board::sue_for_peace, this, &GameLobby::handlePeaceTalk);
                NewWindow_end->show();
                myBoard->forceClose();
                currentStatusLabel->setText("Waiting");
                updateButton();
            }else{
                QJsonObject request;
                request["action"]="DisagreeToDraw";
                request["recipient"]=opponent_name;
                request["promoter"]=username;
                QJsonDocument doc(request);
                socket->sendTextMessage(doc.toJson(QJsonDocument::Compact));
            }
        }else if(action == "OpponentAgreeToDraw"){
            QMessageBox::information(this, "Draw", "Opponent Agree To Draw");
            myBoard->sentdata(myBoard->record,4);
            newWindow_end *NewWindow_end;
            NewWindow_end = new newWindow_end(1,4,username);
            disconnect(myBoard, &board::moveSent, this, &GameLobby::handleMoveSent);
            disconnect(myBoard, &board::sentfinal, this, &GameLobby::handleFinalSent);
            disconnect(myBoard, &board::sue_for_peace, this, &GameLobby::handlePeaceTalk);
            NewWindow_end->show();
            myBoard->forceClose();
            currentStatusLabel->setText("Waiting");
            updateButton();
        }else if(action == "OpponentDisagreeToDraw"){
            QMessageBox::information(this, "Draw", "Opponent Disagree To Draw");
        }
    }
}
void GameLobby::joinRoom(const QString &roomName,const QString &roomHost,bool myfirst){
    QJsonObject request;
    isFirst=myfirst;
    request["action"] = "joinRoom";
    request["objectRoom"] = roomName;
    request["username"] = username;
    request["roomHost"] = roomHost;
    QJsonDocument doc(request);
    socket->sendTextMessage(doc.toJson(QJsonDocument::Compact));
}
void GameLobby::updateButton(){
    updateCreateRoomButton();
    updateExitRoomButton();
    updateStartGameButton();
}
void GameLobby::updateCreateRoomButton(){
    disconnect(createRoomButton, &QPushButton::clicked, this, nullptr);
    if(nowstate==0)
    {
        connect(createRoomButton, &QPushButton::clicked, this, &GameLobby::CreateRoom);
    }
    else if(currentStatusLabel->text()=="Playing"){
        connect(exitRoomButton, &QPushButton::clicked,this,[this]{
            QMessageBox::warning(this,"Invalid Operation","You are playing game");
        });
    }
    else
    {
        connect(createRoomButton, &QPushButton::clicked,this,[this]{
            QMessageBox::warning(this,"Invalid Operation","You are already in a room");
        });
    }
}
void GameLobby::updateStartGameButton(){
    disconnect(startGameButton, &QPushButton::clicked, this, nullptr);
    if(nowstate==2&&currentStatusLabel->text()!="Playing")
    {
        connect(startGameButton, &QPushButton::clicked, this, &GameLobby::startGame);
    }
    else if(nowstate==0)
    {
        connect(startGameButton, &QPushButton::clicked,this,[this]{
            QMessageBox::warning(this,"Invalid Operation","You are not in a room");
        });
    }
    else if(currentStatusLabel->text()=="Playing"){
        connect(exitRoomButton, &QPushButton::clicked,this,[this]{
            QMessageBox::warning(this,"Invalid Operation","You are playing game");
        });
    }
    else
    {
        connect(startGameButton, &QPushButton::clicked,this,[this]{
            QMessageBox::warning(this,"Invalid Operation","Insufficient number of people in room");
        });
    }
}
void GameLobby::updateExitRoomButton(){
    disconnect(exitRoomButton, &QPushButton::clicked, this, nullptr);
    if((nowstate==1||nowstate==2)&&currentStatusLabel->text()!="Playing")
    {
        connect(exitRoomButton, &QPushButton::clicked, this, &GameLobby::exitRoom);
    }
    else if(currentStatusLabel->text()=="Playing"){
        connect(exitRoomButton, &QPushButton::clicked,this,[this]{
            QMessageBox::warning(this,"Invalid Operation","You are playing game");
        });
    }
    else
    {
        connect(exitRoomButton, &QPushButton::clicked,this,[this]{
            QMessageBox::warning(this,"Invalid Operation","You are not in a room");
        });
    }
}
GameLobby::~GameLobby() {
    delete socket;
    delete myBoard;
}
