#ifndef REGISTER_H
#define REGISTER_H
#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QJsonObject>
#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
class RegisterWindow : public QWidget
{
    Q_OBJECT
private:
    QLabel *usernameLabel;
    QLabel *passwordLabel;
    QPushButton *registerButton;
    QVBoxLayout *layout;
    QLineEdit *usernameLineEdit;
    QLineEdit *passwordLineEdit;
    void closeEvent(QCloseEvent *event) override;
public:
    QString username="";
    RegisterWindow(QWidget *parent = nullptr);
    void onRegisterClicked();
    void onRegistrationResponse(QNetworkReply *reply);
};

#endif // REGISTER_H
