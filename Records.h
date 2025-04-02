#ifndef RECORDS_H
#define RECORDS_H
#include<QWidget>
#include<QTableWidget>
#include<QPushButton>
#include<QVBoxLayout>
#include<QHBoxLayout>
#include<QList>
#include"mainwindow.h"
class RecordWindow:public QWidget{
    Q_OBJECT
public:
    void showPreviousPage();
    void showNextPage();
    QString username;
    RecordWindow(QString username,const QList<MainWindow::GameInfo> &gameList,QWidget *parent = nullptr);
    void continueGame(vector<vector<int>> nowBoard,QStack<board::GameData> record,int type,int step,QString firstPlayer);
    QList<MainWindow::GameInfo> games;
signals:
    void oncontinueGame();
private:
    int currentPage;
    int itemsPerPage;
    QTableWidget *tableWidget;
    QPushButton *prevButton;
    QPushButton *nextButton;
    void updateTableData();
};

#endif // RECORDS_H
