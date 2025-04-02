#ifndef RULES_H
#define RULES_H
#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include<QPushButton>
class RulesWindow : public QWidget
{
    Q_OBJECT

public:
    QString username;
    explicit RulesWindow(QString username,QWidget *parent = nullptr);
    void openstartWindow();
    ~RulesWindow();
private:
    QPushButton *back;
};
#endif // RULES_H
