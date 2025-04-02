#ifndef LOGIN_H
#define LOGIN_H
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
#include <QSettings>
class LoginWindow : public QWidget
{
    Q_OBJECT
private:
    QLabel *usernameLabel;
    QLabel *passwordLabel;
    QPushButton *LoginButton;
    QVBoxLayout *layout;
    QLineEdit *usernameLineEdit;
    QLineEdit *passwordLineEdit;
    void closeEvent(QCloseEvent *event) override;
public:
    QString username;
    LoginWindow(QWidget *parent = nullptr);
    void onLoginClicked();
    void onLoginResponse(QNetworkReply *reply);
};
#endif // LOGIN_H
