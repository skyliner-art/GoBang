#ifndef NEWWINDOW_END_H
#define NEWWINDOW_END_H
#include<QWidget>
#include<QPushButton>
#include<QLabel>
class newWindow_end:public QWidget
{
    Q_OBJECT
private:
    QLabel *resultLabel;
    QLabel *otherLabel;
    QPushButton *Another_Round;//再来一局
    QPushButton *reback;//返回
public:
    int winner;//记录胜方
    int lasttype;//记录上次返回类型，便于重新开始
    QString username;
    newWindow_end(int lasttype,int winner,QString username,QWidget *parent = nullptr);
    void restartWindow();
    void rebackWindow_start();
    ~newWindow_end();
};

#endif // NEWWINDOW_END_H
