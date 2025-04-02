#include <QTableWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QWidget>
#include"Records.h"
using namespace std;
RecordWindow::RecordWindow(QString username,const QList<MainWindow::GameInfo> &gameList, QWidget *parent)
    : QWidget(parent), games(gameList), currentPage(0), itemsPerPage(10),username(username)
{
        setWindowIcon(QIcon(":/images/image/icon.jpg"));
        tableWidget = new QTableWidget(this);
        tableWidget->setColumnCount(6);
        tableWidget->setHorizontalHeaderLabels({"firstPlayer", "secondPlayer", "State","Steps","Continue","Time"});//先手，后手，状态，步数,时间

        prevButton = new QPushButton("Previous", this);
        nextButton = new QPushButton("Next", this);

        connect(prevButton, &QPushButton::clicked, this, &RecordWindow::showPreviousPage);
        connect(nextButton, &QPushButton::clicked, this, &RecordWindow::showNextPage);

        // 创建布局
        QVBoxLayout *mainLayout = new QVBoxLayout(this);
        QHBoxLayout *buttonLayout = new QHBoxLayout();

        // 按钮布局
        buttonLayout->addWidget(prevButton);
        buttonLayout->addWidget(nextButton);

        mainLayout->addWidget(tableWidget);
        mainLayout->addLayout(buttonLayout);

        setLayout(mainLayout);

        // 初始化显示第一页数据
        updateTableData();
    }

void RecordWindow::showPreviousPage()
{
    if (currentPage > 0)
    {
        currentPage--;
        updateTableData();
    }
}

void RecordWindow::showNextPage()
{
    int totalPages = (games.size() + itemsPerPage - 1) / itemsPerPage;
    if (currentPage < totalPages - 1)
    {
        currentPage++;
        updateTableData();
    }
}

void RecordWindow::updateTableData()
{
    // 计算当前页应该显示的起始位置和结束位置
    int startIndex = currentPage * itemsPerPage;
    int endIndex = min((currentPage + 1) * itemsPerPage, static_cast<int>(games.size()));
    tableWidget->setRowCount(endIndex - startIndex);
    for (int i = startIndex; i < endIndex; ++i)
    {
        const MainWindow::GameInfo &game = games[i];
        tableWidget->setItem(i - startIndex, 0, new QTableWidgetItem(game.firstPlayer));
        tableWidget->setItem(i - startIndex, 1, new QTableWidgetItem(game.secondPlayer));
        tableWidget->setItem(i - startIndex, 3, new QTableWidgetItem(QString::number(game.step)));
        tableWidget->setItem(i - startIndex, 5, new QTableWidgetItem(game.time));
        if(game.state==3)
        {
            tableWidget->setItem(i - startIndex, 2, new QTableWidgetItem("unfinished"));
        }
        else if(game.state==2)
        {
            tableWidget->setItem(i - startIndex, 2, new QTableWidgetItem("SecondPlayer Win"));
        }
        else if(game.state==1)
        {
            tableWidget->setItem(i - startIndex, 2, new QTableWidgetItem("firstPlayer Win"));
        }
        else if(game.state==4)
        {
            tableWidget->setItem(i - startIndex, 2, new QTableWidgetItem("Draw"));
        }
        else if (game.state == 3)
        {
            tableWidget->setItem(i - startIndex, 2, new QTableWidgetItem("Unfinished"));
        }
        int type;//继续对局的种类
        if(game.firstPlayer!="ai"&&game.secondPlayer!="ai"&&game.secondPlayer==game.firstPlayer)
        {
            type=0;
        }
        else if(game.firstPlayer!="ai"&&game.secondPlayer!="ai"&&game.secondPlayer!=game.firstPlayer)
        {
            type=1;
        }
        else if((game.firstPlayer!="ai"&&game.secondPlayer=="ai")||(game.firstPlayer=="ai"&&game.secondPlayer!="ai"))
        {
            type=2;
        }
        else if(game.firstPlayer=="ai"&&game.secondPlayer=="ai")
        {
            continue;
        }
        QPushButton *continueButton = new QPushButton("Continue", this);
        tableWidget->setCellWidget(i - startIndex, 4, continueButton);
        connect(continueButton, &QPushButton::clicked, this, [type,this, game]()
        {
            continueGame(game.nowboard,game.gamedata,type,game.step,game.firstPlayer);
        });
    }

        // 设置前一页/后一页按钮的可用性
        prevButton->setEnabled(currentPage > 0);
        nextButton->setEnabled(currentPage < (games.size() + itemsPerPage - 1) / itemsPerPage - 1);
}

void RecordWindow::continueGame(vector<vector<int>> nowBoard,QStack<board::GameData> gamedata,int type,int step,QString firstPlayer)
{
    emit oncontinueGame();
    qDebug() << "Continuing game with ID: " << username;
    board *newWindow = new board(false,type,username);
    newWindow->chessboard=nowBoard;
    while(gamedata.size()!=1)
    {
        newWindow->record.append(gamedata.top());
        gamedata.pop();
    }
    newWindow->record.append(gamedata.top());
    newWindow->countstep=step;
    if(type==2)
    {
        if(firstPlayer=="ai")
        {
            newWindow->isfirst=false;
        }
        else
        {
            newWindow->isfirst=true;
        }
        bool ok;
        int difficulty = QInputDialog::getInt(this, "Select Difficulty", "Enter AI's thinking depth:", 9, 5, 20, 1, &ok);

        if (ok) {
            qDebug() << "AI Thinking Depth set to:" << difficulty;
        } else {
            difficulty = 9;  // 你可以选择一个默认的值
            qDebug() << "AI Thinking Depth set to default:";
        }
        newWindow->sentfirst(difficulty);
        newWindow->show();
        newWindow->update();
        if((newWindow->isfirst&&step%2==1)||(!newWindow->isfirst&&step%2==0))
        {
            newWindow->waitingForMove = true;
            newWindow->countstep++;
            newWindow->callaiScript();
        }
    }
    QTimer::singleShot(0, this, &RecordWindow::close);
}
