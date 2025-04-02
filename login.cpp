#include"login.h"
#include"mainwindow.h"
#include<QWidget>
LoginWindow::LoginWindow(QWidget *parent)
    : QWidget(parent)
{
    setWindowIcon(QIcon(":/images/image/icon.jpg"));
    usernameLabel = new QLabel("Username:");
    passwordLabel = new QLabel("Password:");
    usernameLineEdit = new QLineEdit();
    passwordLineEdit = new QLineEdit();
    passwordLineEdit->setEchoMode(QLineEdit::Password);
    LoginButton = new QPushButton("Login");
    layout = new QVBoxLayout();
    layout->addWidget(usernameLabel);
    layout->addWidget(usernameLineEdit);
    layout->addWidget(passwordLabel);
    layout->addWidget(passwordLineEdit);
    layout->addWidget(LoginButton);
    setLayout(layout);
    connect(LoginButton, &QPushButton::clicked, this, &LoginWindow::onLoginClicked);
}
void LoginWindow::onLoginClicked()
{
    username = usernameLineEdit->text();
    QString password = passwordLineEdit->text();

    if(username.isEmpty()||password.isEmpty())
    {
        QMessageBox::warning(this, "Input Error", "Please fill in both fields.");
        return;
    }

    // 创建 JSON 请求数据
    QJsonObject data;
    data["username"] = username;
    data["password"] = password;
    QJsonDocument doc(data);
    QByteArray requestData = doc.toJson();

    // 创建网络请求
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    QNetworkRequest request(appurl+"gobang/start/login");// 服务器的注册接口
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    // 发送 POST 请求
    manager->post(request, requestData);

    // 连接信号，处理服务器响应
    connect(manager, &QNetworkAccessManager::finished, this, &LoginWindow::onLoginResponse);
}

void LoginWindow::onLoginResponse(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError)
    {
        int res=QMessageBox::information(this, "Login Success","you can start game");
        if(res==QMessageBox::Ok)
        {
            this->close();
        }
    }
    else
    {
        QString errorString = reply->errorString();
        QByteArray responseData = reply->readAll();

        // 解析返回的 JSON 错误响应
        QJsonDocument doc = QJsonDocument::fromJson(responseData);
        if (doc.isObject())
        {
            QJsonObject jsonObj = doc.object();
            QString errorMessage = jsonObj["error"].toString();  // 获取自定义的错误信息
            QMessageBox::warning(this, "Login Failed", errorMessage.isEmpty() ? errorString : errorMessage);
        }
        else
        {
            QMessageBox::warning(this, "Login Failed", errorString);
        }
    }
    reply->deleteLater();
}
void LoginWindow::closeEvent(QCloseEvent *event)
{
    MainWindow *NewWindow= new MainWindow(true,username);
    NewWindow->show();
    event->accept();
}
