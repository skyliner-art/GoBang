#include"Rules.h"
#include"newWindow_start.h"

RulesWindow::RulesWindow(QString username,QWidget *parent) : QWidget(parent),username(username)
{
    setWindowIcon(QIcon(":/images/image/icon.jpg"));
    setWindowTitle("游戏规则");
    QLabel *rulesLabel = new QLabel(this);
    rulesLabel->setText("这是五子棋的规则：\n"
                        "1. 黑子先手，白子后手。\n"
                        "2. 轮流下棋，直到一方在棋盘上连续形成五子连线。\n"
                        "3. 与五子棋基本规则相同，棋盘大小为15*15。\n"
                        "4. ***存在先手（黑子）禁手***\n"
                        "   一、三三禁手：黑方一子落下同时形成两个或两个以上的活三(或嵌四),此步为三三禁手。注意:这里一定要 两个都是活三才能算。\n"
                        "   二、四四禁手：黑方一子落下同时形成两个或两个以上的四,活四、冲四、嵌五之四四,包括在此四之内。此步为四四禁手。注意:只要是两个四即为禁手,无论是哪种四,活四,跳四,冲四都算。\n"
                        "   三、长连禁手：黑方一子落下形成连续六子或六子以上相连。注意:白棋出现长连与连五同等作用,即白棋出现长连也将获胜。\n"
                        "   四、四三三禁手：黑方一步使一个四,两个活三同时形成。\n"
                        "   五、四四三禁手：黑方一步使两个四,一个活三同时形成。\n"
                        "   关于禁手的规定:黑方五连与禁手同时形成,禁手失效,黑方胜。\n"
                        );

    rulesLabel->setWordWrap(true);//自动换行
    QFont font;
    font.setPointSize(15);
    rulesLabel->setFont(font);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(rulesLabel);
    setLayout(layout);

    back = new QPushButton("Back",this);
    back->setFixedSize(200, 50);
    layout->addWidget(back);
    setFixedSize(700, 700);
    connect(back,&QPushButton::clicked,this,&RulesWindow::openstartWindow);
}
void RulesWindow::openstartWindow()
{
    newWindow_start *newWindow = new newWindow_start(username);
    newWindow->show();
    this->close();
}
RulesWindow::~RulesWindow()
{
}
