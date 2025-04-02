#include"newWindow_end.h"
#include"newWindow_start.h"
#include"board.h"
newWindow_end::newWindow_end(int lasttype,int winner,QString username,QWidget *parent)
    : QWidget(parent),winner(winner),username(username),lasttype(lasttype)
{
    // 根据 winner 显示获胜信息
    setWindowIcon(QIcon(":/images/image/icon.jpg"));
    resultLabel = new QLabel(this);
    otherLabel = new QLabel(this);
    otherLabel->setText("<i><b>You can't enjoy playing Gomoku<br>only when you win</b></i>");

    if (winner == 1)
    {
        resultLabel->setText("Black Wins!");
    }
    else if (winner == 0)
    {
        resultLabel->setText("White Wins!");
    }
    else if(winner == 4)
    {
        resultLabel->setText("Draw!");
    }
    resultLabel->setGeometry(100, 50, 200, 50);
    otherLabel->setGeometry(100, 100, 400, 100);
    otherLabel->setStyleSheet("font-size: 13px;");
    Another_Round = new QPushButton("Another_Round",this);
    reback = new QPushButton("reback",this);
    Another_Round->setGeometry(100,200,200,50);
    reback->setGeometry(100,250,200,50);

    setFixedSize(400,400);

    connect(Another_Round,&QPushButton::clicked,this,&newWindow_end::restartWindow);
    connect(reback,&QPushButton::clicked,this,&newWindow_end::rebackWindow_start);
}
void newWindow_end::restartWindow()
{
    if(lasttype==1){
        QMessageBox::warning(this,"Warning","No man can call again yesterday.");
        return;
    }
    board *newWindow = new board(true,lasttype,username);
    newWindow->show();
    this->close();
}
void newWindow_end::rebackWindow_start()
{
    if(lasttype==1){
        this->close();
    }else {
        newWindow_start *newWindow = new newWindow_start(username);
        newWindow->show();
        this->close();
    }
}
newWindow_end::~newWindow_end()
{

}
