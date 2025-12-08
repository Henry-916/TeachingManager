#include "logindialog.h"
#include "ui_logindialog.h"
#include <QSqlQuery>
#include <QMessageBox>
#include <QDebug>
#include <QSqlError>

LoginDialog::LoginDialog(Database &db, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LoginDialog)
    , m_db(db)
    , m_loggedIn(false)
{
    ui->setupUi(this);
    setWindowTitle("用户登录 - 教学管理系统");
    setFixedSize(400, 300);

    // 设置角色下拉框
    ui->roleComboBox->addItem("学生", static_cast<int>(UserRole::Student));
    ui->roleComboBox->addItem("教师", static_cast<int>(UserRole::Teacher));
    ui->roleComboBox->addItem("管理员", static_cast<int>(UserRole::Admin));

    // 密码显示模式
    ui->passwordEdit->setEchoMode(QLineEdit::Password);

    // 连接信号槽
    connect(ui->loginButton, &QPushButton::clicked, this, &LoginDialog::onLoginButtonClicked);
    connect(ui->registerButton, &QPushButton::clicked, this, &LoginDialog::onRegisterButtonClicked);
}

LoginDialog::~LoginDialog()
{
    delete ui;
}

void LoginDialog::onLoginButtonClicked()
{
    QString username = ui->usernameEdit->text().trimmed();
    QString password = ui->passwordEdit->text();
    int role = ui->roleComboBox->currentData().toInt();

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请输入用户名和密码");
        return;
    }

    qDebug() << "尝试登录:";
    qDebug() << "用户名:" << username;
    qDebug() << "密码:" << password;
    qDebug() << "角色:" << role;

    // 使用Database类的authenticateUser方法
    if (m_db.authenticateUser(username, password, role)) {
        // 获取用户ID
        QSqlQuery query(m_db.getDatabase());
        query.prepare("SELECT user_id FROM users WHERE username = ?");
        query.addBindValue(username);

        if (query.exec() && query.next()) {
            int userId = query.value(0).toInt();
            UserRole userRole = static_cast<UserRole>(role);
            m_currentUser = User(userId, username, userRole);
            m_loggedIn = true;
            qDebug() << "登录成功!";
            accept();
        } else {
            QMessageBox::warning(this, "登录失败", "获取用户信息失败");
        }
    } else {
        QMessageBox::warning(this, "登录失败", "用户名或密码错误");
    }
}

void LoginDialog::onRegisterButtonClicked()
{
    QString username = ui->usernameEdit->text().trimmed();
    QString password = ui->passwordEdit->text();
    int role = ui->roleComboBox->currentData().toInt();

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请输入用户名和密码");
        return;
    }

    // 检查用户名是否已存在
    QSqlQuery checkQuery(m_db.getDatabase());
    checkQuery.prepare("SELECT COUNT(*) FROM users WHERE username = ?");
    checkQuery.addBindValue(username);

    if (checkQuery.exec() && checkQuery.next() && checkQuery.value(0).toInt() > 0) {
        QMessageBox::warning(this, "注册失败", "用户名已存在");
        return;
    }

    // 使用Database类的addUser方法
    if (m_db.addUser(username, password, role)) {
        QMessageBox::information(this, "注册成功", "用户注册成功，请登录");
    } else {
        QMessageBox::warning(this, "注册失败", "用户注册失败");
    }
}
