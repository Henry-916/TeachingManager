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
    // 使用静态映射表简化代码
    static const QMap<int, QString> roleLabels = {
        {0, "学号:"},
        {1, "工号:"},
        {2, "账号:"}
    };

    static const QMap<int, QString> placeholders = {
        {0, "请输入学号"},
        {1, "请输入工号"},
        {2, "请输入管理员账号"}
    };

    if (roleLabels.contains(role)) {
        ui->idLabel->setText(roleLabels[role]);
        ui->idEdit->setPlaceholderText(placeholders[role]);
    }

    ui->idEdit->clear();
}

void LoginDialog::onLoginButtonClicked()
{
    QString identifier = ui->idEdit->text().trimmed();
    QString password = ui->passwordEdit->text();
    int role = ui->roleComboBox->currentData().toInt();

    if (identifier.isEmpty() || password.isEmpty()) {
        if (role == 2) {
            QMessageBox::warning(this, "输入错误", "请输入账号和密码");
        } else {
            QMessageBox::warning(this, "输入错误",
                                 role == 0 ? "请输入学号和密码" : "请输入工号和密码");
        }
        return;
    }

    qDebug() << "尝试登录:";
    qDebug() << "角色:" << role;
    qDebug() << "账号:" << identifier;

    bool loginSuccess = false;
    int userId = -1;
    QString account = identifier;

    // 统一验证逻辑：使用账号、密码和角色进行验证
    if (m_db.validateUser(account, password, role, userId)) {
        loginSuccess = true;
    }

    if (loginSuccess) {
        UserRole userRole = static_cast<UserRole>(role);
        m_currentUser = User(userId, account, userRole);  // account作为username
        m_loggedIn = true;
        qDebug() << "登录成功! 用户ID:" << userId << "账号:" << account;
        accept();
    } else {
        qDebug() << "登录失败: 账号或密码错误";
        if (role == 2) {
            QMessageBox::warning(this, "登录失败", "账号或密码错误");
        } else {
            QMessageBox::warning(this, "登录失败",
                                 role == 0 ? "学号或密码错误" : "工号或密码错误");
        }
    }
}
