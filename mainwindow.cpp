#include "mainwindow.h"
#include "newWindow_start.h"
#include"login.h"
#include"register.h"
#include"Records.h"

#include <QTimer>
MainWindow::MainWindow(bool login,QString username,QWidget *parent)
    : QMainWindow(parent),login(login),username(username)
{
    QFont font;
    font.setFamily("Arial");  // 设置字体
    font.setPointSize(16);    // 设置字体大小
    font.setItalic(true);    //设置斜体
    setWindowIcon(QIcon(":/images/image/icon.jpg"));
    usernameLabel = new QLabel(this);
    usernameLabel->setFont(font);
    usernameLabel->setText("Welcome " + username);
    usernameLabel->setGeometry(100, 50, 250, 30);
    NewStart =new QPushButton("New Start",this);
    // ReadAchived =new QPushButton("Read Achived",this);
    Records =new QPushButton("Records",this);
    Register =new QPushButton("Register",this);
    Login =new QPushButton("Login",this);

    NewStart->setGeometry(165, 200, 200, 50);
    // ReadAchived->setGeometry(100, 200, 200, 50);
    Records->setGeometry(165, 300, 200, 50);
    Register->setGeometry(400, 50, 100, 30);
    Login->setGeometry(400, 100, 100, 30);

    setFixedSize(550, 500);
    connect(NewStart,&QPushButton::clicked,this,&MainWindow::openNewWindow);
    connect(Register,&QPushButton::clicked,this,&MainWindow::openRegisterWindow);
    connect(Login,&QPushButton::clicked,this,&MainWindow::openLoginWindow);
    connect(Records,&QPushButton::clicked,this,&MainWindow::openRecordsWindow);
}
void MainWindow::openNewWindow()
{
    if(login)
    {
        newWindow_start *newWindow = new newWindow_start(username);
        newWindow->show();
        this->close();
    }
}
void MainWindow::openRegisterWindow()
{
    RegisterWindow *newWindow = new RegisterWindow();
    newWindow->show();
    this->close();
}
void MainWindow::openLoginWindow()
{
    LoginWindow *newWindow = new LoginWindow();
    newWindow->show();
    this->close();
    QTimer::singleShot(0, this, &MainWindow::close);
}
void MainWindow::openRecordsWindow()
{
    if(login)
    {
        getdata(username);
    }
}
void MainWindow::getdata(QString username)
{
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    QUrl url(appurl+"gobang/gamedata/history?username=" + QUrl::toPercentEncoding(username));
    QNetworkRequest request(url);
    qDebug() << "Sending GET request to: " << request.url();
    qDebug() << "Request Headers: " << request.rawHeaderList();
    QNetworkReply *reply = manager->get(request);
    reply->setProperty("timeout", 2000);

    connect(reply, &QNetworkReply::finished, this, [this, reply,username]()
    {
        if (reply->error() != QNetworkReply::NoError)
        {
            qWarning() << "Request failed:" << reply->errorString();
        }
        else
        {
            qDebug() << "Request succeeded!";
            QByteArray responseData = reply->readAll();
            QJsonDocument doc = QJsonDocument::fromJson(responseData);
            if (doc.isNull())
            {
                qDebug() << "Failed to parse JSON!";
            }
            QJsonObject jsonObj = doc.object();
            QJsonArray gamesArray = jsonObj["games"].toArray();
            QList<MainWindow::GameInfo> resultList;
            for (const QJsonValue &gameVal : gamesArray)
            {
                MainWindow::GameInfo temp;
                QJsonObject game = gameVal.toObject();
                temp.gameid = game["gameId"].toInt();
                temp.step = game["step"].toInt();
                temp.firstPlayer= game["firstPlayer"].toString();
                temp.secondPlayer = game["secondPlayer"].toString();
                QByteArray gameDataJson = game["gameData"].toString().toUtf8();
                QJsonDocument doc = QJsonDocument::fromJson(gameDataJson);
                QJsonArray jsonArray = doc.array();
                QStack<board::GameData> gameDataStack;
                for (const QJsonValue &value : jsonArray)
                {
                    QJsonObject gameObject = value.toObject();
                    board::GameData gameData = board::GameData::fromJson(gameObject);
                    gameDataStack.push(gameData);
                }
                temp.gamedata=gameDataStack;//注意这里是反序，等到更新棋局的时候会正过来
                QByteArray boardJson = game["nowBoard"].toString().toUtf8();
                QJsonDocument docboard = QJsonDocument::fromJson(boardJson);
                QJsonArray boardArray = docboard.array();;
                std::vector<std::vector<int>> nowBoard;
                for (const QJsonValue& rowValue : boardArray)
                {
                    if (rowValue.isArray())
                    {
                        QJsonArray rowArray = rowValue.toArray();
                        std::vector<int> row;
                        for (const QJsonValue& cellValue : rowArray)
                        {
                            row.push_back(cellValue.toInt());
                        }
                        nowBoard.push_back(row);
                    }
                }
                temp.nowboard=nowBoard;
                temp.state= game["state"].toInt();
                temp.time = game["time"].toString();
                resultList.append(temp);
            }
            qDebug() << username << " Total games: " << resultList.size();
            RecordWindow *newWindow = new RecordWindow(username,resultList);
            newWindow->show();
            connect(newWindow, &RecordWindow::oncontinueGame, this, &MainWindow::closeWindow);
        }
        reply->deleteLater();
    });
    connect(manager, &QNetworkAccessManager::finished, manager, &QNetworkAccessManager::deleteLater);
}
QString appconfigFile = "config_app.ini";
QSettings appsettings(appconfigFile, QSettings::IniFormat);
QString appip = appsettings.value("app/ip", "127.0.0.1").toString();
QString appport = appsettings.value("app/port", "666").toString();
QString appurl = QString("http://%1:%2/").arg(appip).arg(appport);
QUrl appurlObj(appurl);
QString serverconfigFile = "config_server.ini";
QSettings serversettings(serverconfigFile, QSettings::IniFormat);
QString serverip = serversettings.value("server/ip", "127.0.0.1").toString();
QString serverport = serversettings.value("server/port", "666").toString();
QString serverurl = QString("ws://%1:%2/ws/").arg(serverip).arg(serverport);
QUrl serverurlObj(serverurl);
MainWindow::~MainWindow()
{

}
void MainWindow::closeWindow()
{
    this->close();
}
