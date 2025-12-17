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
    setFixedSize(500, 350);

    // 设置角色数据
    for (int i = 0; i < ui->roleComboBox->count(); ++i) {
        int roleValue = i;  // 0=学生, 1=教师, 2=管理员
        ui->roleComboBox->setItemData(i, roleValue);
    }

    ui->passwordEdit->setEchoMode(QLineEdit::Password);

    // 连接信号槽
    connect(ui->loginButton, &QPushButton::clicked, this, &LoginDialog::onLoginButtonClicked);
    connect(ui->roleComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &LoginDialog::onRoleChanged);

    // 直接调用，无需计时器
    onRoleChanged(ui->roleComboBox->currentIndex());
}

LoginDialog::~LoginDialog()
{
    delete ui;
}

void LoginDialog::onRoleChanged(int index)
{
    if (index < 0) return;

    int role = ui->roleComboBox->itemData(index).toInt();
    updateUIForRole(role);
}

void LoginDialog::updateUIForRole(int role)
{
    switch (role) {
    case 0: // 学生
        ui->idLabel->setText("学号:");
        ui->idLabel->setVisible(true);
        ui->idEdit->setVisible(true);
        ui->idEdit->setPlaceholderText("请输入学号");
        ui->idEdit->clear();
        break;
    case 1: // 教师
        ui->idLabel->setText("工号:");
        ui->idLabel->setVisible(true);
        ui->idEdit->setVisible(true);
        ui->idEdit->setPlaceholderText("请输入工号");
        ui->idEdit->clear();
        break;
    case 2: // 管理员
        ui->idLabel->setText("用户名:");
        ui->idLabel->setVisible(true);
        ui->idEdit->setVisible(true);
        ui->idEdit->setPlaceholderText("请输入管理员用户名");
        ui->idEdit->clear();
        break;
    }

    // 强制更新布局
    ui->formLayout->update();
    ui->verticalLayout->update();
}

void LoginDialog::onLoginButtonClicked()
{
    QString identifier = ui->idEdit->text().trimmed();
    QString password = ui->passwordEdit->text();
    int role = ui->roleComboBox->currentData().toInt();

    if (identifier.isEmpty() || password.isEmpty()) {
        if (role == 2) {
            QMessageBox::warning(this, "输入错误", "请输入用户名和密码");
        } else {
            QMessageBox::warning(this, "输入错误",
                                 role == 0 ? "请输入学号和密码" : "请输入工号和密码");
        }
        return;
    }

    qDebug() << "尝试登录:";
    qDebug() << "角色:" << role;
    qDebug() << "标识:" << identifier;

    bool loginSuccess = false;
    int userId = -1;
    QString username = "";

    if (role == 2) { // 管理员登录
        // 管理员使用用户名和密码登录
        QSqlQuery query;
        query.prepare("SELECT user_id, username FROM users "
                      "WHERE username = ? AND password = ? AND role = ?");
        query.addBindValue(identifier);
        query.addBindValue(password);
        query.addBindValue(role);

        if (query.exec() && query.next()) {
            userId = query.value(0).toInt();
            username = query.value(1).toString();
            loginSuccess = true;
        }
    } else { // 学生或教师登录
        // 学生或教师使用ID和密码登录
        bool ok;
        int id = identifier.toInt(&ok);
        if (!ok) {
            QMessageBox::warning(this, "输入错误",
                                 role == 0 ? "学号必须是数字" : "工号必须是数字");
            return;
        }

        QSqlQuery query;
        if (role == 0) { // 学生
            query.prepare("SELECT u.user_id, u.username FROM users u "
                          "INNER JOIN students s ON u.student_id = s.student_id "
                          "WHERE u.student_id = ? AND u.password = ? AND u.role = ?");
        } else { // 教师
            query.prepare("SELECT u.user_id, u.username FROM users u "
                          "INNER JOIN teachers t ON u.teacher_id = t.teacher_id "
                          "WHERE u.teacher_id = ? AND u.password = ? AND u.role = ?");
        }
        query.addBindValue(id);
        query.addBindValue(password);
        query.addBindValue(role);

        if (query.exec() && query.next()) {
            userId = query.value(0).toInt();
            username = query.value(1).toString();
            loginSuccess = true;
        }
    }

    if (loginSuccess) {
        UserRole userRole = static_cast<UserRole>(role);
        m_currentUser = User(userId, username, userRole);
        m_loggedIn = true;
        qDebug() << "登录成功! 用户ID:" << userId << "用户名:" << username;
        accept();
    } else {
        qDebug() << "登录失败: 用户名/ID或密码错误";
        if (role == 2) {
            QMessageBox::warning(this, "登录失败", "用户名或密码错误");
        } else {
            QMessageBox::warning(this, "登录失败",
                                 role == 0 ? "学号或密码错误" : "工号或密码错误");
        }
    }
}
